/**
 * server/src/board/map/signalpath.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023 Reinder Feenstra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef TRAINTASTIC_SERVER_BOARD_MAP_ABSTRACTSIGNALPATH_HPP
#define TRAINTASTIC_SERVER_BOARD_MAP_ABSTRACTSIGNALPATH_HPP

#include "path.hpp"
#include "link.hpp"
#include <map>
#include <boost/signals2/connection.hpp>
#include <traintastic/enum/blockstate.hpp>

class BlockRailTile;
enum class BlockSide : uint8_t;
class TurnoutRailTile;
enum class TurnoutPosition : uint8_t;
class DirectionControlRailTile;
enum class DirectionControlState : uint8_t;
class SignalRailTile;
enum class SignalAspect : uint8_t;

class AbstractSignalPath : public Path
{
  private:
    SignalRailTile& m_signal;

    AbstractSignalPath(const AbstractSignalPath&) = delete;
    AbstractSignalPath& operator =(const AbstractSignalPath&) = delete;

    void setAspect(SignalAspect value) const;

  protected:
    class Item
    {
      private:
        Item(const Item&) = delete;
        Item& operator =(const Item&) = delete;

      protected:
        inline static const std::unique_ptr<const Item> noItem;

        Item() = default;

      public:
        virtual ~Item() = default;

        virtual const std::unique_ptr<const Item>& next() const = 0;
    };

    class BlockItem : public Item
    {
      private:
        std::weak_ptr<BlockRailTile> m_block;
        BlockSide m_enterSide;
        std::unique_ptr<const Item> m_next;

      public:
        BlockItem(std::weak_ptr<BlockRailTile> block, BlockSide enterSide_, std::unique_ptr<const Item> next_)
          : m_block{std::move(block)}
          , m_enterSide{enterSide_}
          , m_next{std::move(next_)}
        {
        }

        const std::unique_ptr<const Item>& next() const final
        {
          return m_next;
        }

        std::shared_ptr<BlockRailTile> block() const noexcept;
        BlockState blockState() const;
        BlockSide enterSide() const
        {
          return m_enterSide;
        }
    };

    class SignalItem : public Item
    {
      private:
        std::weak_ptr<SignalRailTile> m_signal;
        std::unique_ptr<const Item> m_next;

      public:
        SignalItem(std::weak_ptr<SignalRailTile> signal, std::unique_ptr<const Item> next_)
          : m_signal{std::move(signal)}
          , m_next{std::move(next_)}
        {
        }

        const std::unique_ptr<const Item>& next() const final
        {
          return m_next;
        }

        std::shared_ptr<SignalRailTile> signal() const
        {
          return m_signal.lock();
        }
    };

    class TurnoutItem : public Item
    {
      private:
        std::weak_ptr<TurnoutRailTile> m_turnout;
        std::map<TurnoutPosition, std::unique_ptr<const Item>> m_next;

      public:
        TurnoutItem(std::weak_ptr<TurnoutRailTile> turnout, std::map<TurnoutPosition, std::unique_ptr<const Item>> next_)
          : m_turnout{std::move(turnout)}
          , m_next{std::move(next_)}
        {
        }

        const std::unique_ptr<const Item>& next() const final;
    };

    class DirectionControlItem : public Item
    {
      private:
        std::weak_ptr<DirectionControlRailTile> m_directionControl;
        std::unique_ptr<const Item> m_next;
        const DirectionControlState m_oneWayState;

      public:
        DirectionControlItem(std::weak_ptr<DirectionControlRailTile> directionControl, DirectionControlState oneWayState, std::unique_ptr<const Item> next_)
          : m_directionControl{std::move(directionControl)}
          , m_next{std::move(next_)}
          , m_oneWayState{oneWayState}
        {
        }

        const std::unique_ptr<const Item>& next() const final;
    };

  private:
    std::unique_ptr<const Item> m_root;
    bool m_requireReservation = false;
    std::vector<boost::signals2::connection> m_connections;

    std::unique_ptr<const Item> findBlocks(const Node& node, const Link& link, size_t blocksAhead);

    inline std::unique_ptr<const Item> findBlocks(const Node& node, const std::shared_ptr<const Link>& link, size_t blocksAhead)
    {
      if(link)
        return findBlocks(node, *link, blocksAhead);
      return {};
    }

  protected:
    inline const SignalRailTile& signal() const
    {
      return m_signal;
    }

    bool requireReservation() const;

    virtual SignalAspect determineAspect() const = 0;

    const Item* root() const
    {
      return m_root.get();
    }

    const BlockItem* nextBlock(const Item* item) const;
    inline const BlockItem* nextBlock() const
    {
      return nextBlock(root());
    }

    std::tuple<const BlockItem*, const SignalItem*> nextBlockOrSignal(const Item* item) const;
    inline std::tuple<const BlockItem*, const SignalItem*> nextBlockOrSignal() const
    {
      return nextBlockOrSignal(root());
    }

    void getBlockStates(tcb::span<BlockState> blockStates) const;
    std::shared_ptr<BlockRailTile> getBlock(size_t index) const;

  public:
    AbstractSignalPath(SignalRailTile& signal);
    AbstractSignalPath(SignalRailTile& signal, size_t blocksAhead);
    virtual ~AbstractSignalPath();

    void evaluate();
};

#endif
