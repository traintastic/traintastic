#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_FEEDBACKMAPITEMBASE_TPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_FEEDBACKMAPITEMBASE_TPP

#include "feedbackmapitembase.hpp"
#include "feedbackmapinputcondition.hpp"
#include "../../../core/attributes.hpp"
#include "../../../core/method.tpp"

template<class Key>
FeedbackMapItemBase<Key>::FeedbackMapItemBase(Object& map, Key _key)
  : FeedbackMapItem(map)
  , key{this, "key", _key, PropertyFlags::ReadOnly | PropertyFlags::Store}
  , m_keyValues{{_key}}
{
  Attributes::addValues(key, m_keyValues);
  m_interfaceItems.add(key);
}

template<class Key>
std::string FeedbackMapItemBase<Key>::getObjectId() const
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

#endif
