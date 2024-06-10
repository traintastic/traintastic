#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAPITEMBASE_TPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAPITEMBASE_TPP

#include "outputmapitembase.hpp"

#include "../../../core/attributes.hpp"
#include "../../../core/method.tpp"
#include "outputmapoutputaction.hpp"

template<class Key>
OutputMapItemBase<Key>::OutputMapItemBase(Object& map, Key _key)
  : OutputMapItem(map)
  , m_keyValues{{_key}}
  , key{this, "key", _key, PropertyFlags::ReadOnly | PropertyFlags::Store}
  , use{this, "use", true, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , getOutputAction{*this, "get_output_action",
      [this](uint32_t index)
      {
        return index < outputActions.size() ? outputActions[index] : std::shared_ptr<OutputMapOutputAction>();
      }}
{
  Attributes::addValues(key, m_keyValues);
  m_interfaceItems.add(key);
  Attributes::addEnabled(use, false);
  m_interfaceItems.add(use);
  m_interfaceItems.add(getOutputAction);
}

template<class Key>
std::string OutputMapItemBase<Key>::getObjectId() const
{
  std::string id{m_map.getObjectId()};
  id.append(".");
  if constexpr(std::is_enum_v<Key>)
  {
    id.append(EnumValues<Key>::value.find(key)->second);
  }
  else if constexpr(std::is_same_v<Key, bool>)
  {
    id.append(key ? "true" : "false");
  }
  else
  {
    static_assert(sizeof(Key) != sizeof(Key));
  }
  return id;
}

#endif // TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAPITEMBASE_TPP
