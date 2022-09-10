#ifndef ASD
#define ASD

#include "link.hpp"
#include <map>
#include <boost/signals2/connection.hpp>
#include <traintastic/enum/blockstate.hpp>

class BlockRailTile;
class TurnoutRailTile;
enum class TurnoutPosition : uint8_t;
class DirectionControlRailTile;
enum class DirectionControlState : uint8_t;

class SignalPath
{
  private:
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
        std::unique_ptr<const Item> m_next;

      public:
        BlockItem(std::weak_ptr<BlockRailTile> block, std::unique_ptr<const Item> next_)
          : m_block{std::move(block)}
          , m_next{std::move(next_)}
        {
        }

        const std::unique_ptr<const Item>& next() const final
        {
          return m_next;
        }

        BlockState blockState() const;
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

    std::unique_ptr<const Item> m_root;
    std::vector<boost::signals2::connection> m_connections;
    std::function<void(const std::vector<BlockState>&)> m_onEvaluated;

    SignalPath(const SignalPath&) = delete;
    SignalPath& operator =(const SignalPath&) = delete;

    void evaluate();
    std::unique_ptr<const Item> findBlocks(const Node& node, const Link& link, size_t blocksAhead);

    inline std::unique_ptr<const Item> findBlocks(const Node& node, const std::shared_ptr<const Link>& link, size_t blocksAhead)
    {
      if(link)
        return findBlocks(node, *link, blocksAhead);
      return {};
    }

  public:
    SignalPath(const Node& signalNode, size_t blocksAhead, std::function<void(const std::vector<BlockState>&)> onEvaluated);
    ~SignalPath();
};

#endif
