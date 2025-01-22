/**
 * server/src/board/tile/rail/signal/signal3aspectrailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2024 Reinder Feenstra
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

#include "signal3aspectrailtile.hpp"
#include "../blockrailtile.hpp"
#include "../../../map/abstractsignalpath.hpp"
#include "../../../map/blockpath.hpp"
#include "../../../../core/attributes.hpp"
#include "../../../../core/method.tpp"
#include "../../../../core/objectproperty.tpp"
#include "../../../../hardware/output/outputcontroller.hpp"

static constexpr std::array<SignalAspect, 4> aspectValues = {
    SignalAspect::Unknown,
    SignalAspect::Stop,
    SignalAspect::ProceedReducedSpeed,
    SignalAspect::Proceed
};

namespace
{
  class SignalPath : public AbstractSignalPath
  {
    private:
      static bool hasSignalReservedPathToBlock(const SignalRailTile& signalTile, const BlockRailTile& blockTile)
      {
        const auto path = signalTile.reservedPath();
        return path && path->toBlock().get() == &blockTile;
      }

      static bool isPathReserved(const BlockRailTile& from, BlockSide fromSide, const BlockRailTile& to)
      {
        if(const auto path = from.getReservedPath(fromSide))
        {
          return (&to == path->toBlock().get());
        }
        return false;
      }

    protected:
      SignalAspect determineAspect() const final
      {
        const auto* blockItem = nextBlock();
        if(!blockItem)
        {
          return SignalAspect::Stop;
        }
        const auto blockState = blockItem->blockState();

        if((!requireReservation() && blockState == BlockState::Free) ||
            (blockState == BlockState::Reserved && hasSignalReservedPathToBlock(signal(), *blockItem->block())))
        {
          auto [blockItem2, signalItem2] = nextBlockOrSignal(blockItem->next().get());
          if(blockItem2)
          {
            const auto blockState2 = blockItem2->blockState();

            if(blockState2 == BlockState::Free ||
                (blockState2 == BlockState::Reserved && isPathReserved(*blockItem->block(), ~blockItem->enterSide(), *blockItem2->block())))
            {
              return SignalAspect::Proceed;
            }
          }
          else if(signalItem2)
          {
            auto signalTile = signalItem2->signal();
            if(signalTile && signalTile->aspect != SignalAspect::Unknown && signalTile->aspect != SignalAspect::Stop)
            {
              switch(signalTile->aspect.value())
              {
                case SignalAspect::ProceedReducedSpeed:
                case SignalAspect::Proceed:
                  return SignalAspect::Proceed;

                case SignalAspect::Stop:
                case SignalAspect::Unknown:
                  break;
              }
            }
          }
          return SignalAspect::ProceedReducedSpeed;
        }
        return SignalAspect::Stop;
      }

    public:
      SignalPath(Signal3AspectRailTile& signal)
        : AbstractSignalPath(signal, 2)
      {
      }
  };
}

Signal3AspectRailTile::Signal3AspectRailTile(World& world, std::string_view _id) :
  SignalRailTile(world, _id, TileId::RailSignal3Aspect)
{
  // Skip Unknown aspect
  tcb::span<const SignalAspect, 3> setAspectValues = tcb::make_span(aspectValues).subspan<1>();

  outputMap.setValueInternal(std::make_shared<SignalOutputMap>(*this, outputMap.name(), std::initializer_list<SignalAspect>{SignalAspect::Stop, SignalAspect::ProceedReducedSpeed, SignalAspect::Proceed}, getDefaultActionValue));

  Attributes::addValues(aspect, aspectValues);
  m_interfaceItems.add(aspect);

  Attributes::addValues(setAspect, setAspectValues);
  m_interfaceItems.add(setAspect);

  connectOutputMap();
}

void Signal3AspectRailTile::boardModified()
{
  m_signalPath = std::make_unique<SignalPath>(*this);
  SignalRailTile::boardModified();
}
