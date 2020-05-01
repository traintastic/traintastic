#include "commandstationproperty.hpp"
#include "../hardware/commandstation/commandstation.hpp"

using T = Hardware::CommandStation::CommandStation;

CommandStationProperty::CommandStationProperty(Object* object, const std::string& name, const std::shared_ptr<T>& value, PropertyFlags flags) :
  AbstractObjectProperty(object, name, flags),
  m_value{value}
{
}

CommandStationProperty::CommandStationProperty(Object* object, const std::string& name, nullptr_t, PropertyFlags flags) :
  CommandStationProperty(object, name, std::shared_ptr<T>(), flags)
{
}

CommandStationProperty::CommandStationProperty(Object* object, const std::string& name, const std::shared_ptr<T>& value, PropertyFlags flags, OnSet onSet) :
  CommandStationProperty(object, name, value, flags)
{
  m_onSet = onSet;
}

CommandStationProperty::CommandStationProperty(Object* object, const std::string& name, nullptr_t, PropertyFlags flags, OnSet onSet) :
  CommandStationProperty(object, name, std::shared_ptr<T>(), flags, onSet)
{
}

const std::shared_ptr<T>& CommandStationProperty::value() const
{
  return m_value;
}

void CommandStationProperty::setValue(const std::shared_ptr<T>& value)
{
  assert(isWriteable());
  if(m_value == value)
    return;
  else if(!isWriteable())
    throw not_writable_error();
  else if(!m_onSet || m_onSet(value))
  {
    m_value = value;
    changed();
  }
  else
    throw invalid_value_error();
 }

void CommandStationProperty::setValueInternal(const std::shared_ptr<T>& value)
{
  if(m_value != value)
  {
    m_value = value;
    changed();
  }
}

const T* CommandStationProperty::operator ->() const
{
  return m_value.get();
}

T* CommandStationProperty::operator ->()
{
  return m_value.get();
}

const T& CommandStationProperty::operator *() const
{
  return *m_value;
}

T& CommandStationProperty::operator *()
{
  return *m_value;
}

CommandStationProperty::operator bool()
{
  return m_value.operator bool();
}

CommandStationProperty& CommandStationProperty::operator =(const std::shared_ptr<T>& value)
{
  setValue(value);
  return *this;
}

ObjectPtr CommandStationProperty::toObject() const 
{
  return std::dynamic_pointer_cast<Object>(m_value);
}

void CommandStationProperty::fromObject(const ObjectPtr& value) 
{
  if(value)
  {
    if(std::shared_ptr<T> v = std::dynamic_pointer_cast<T>(value))
      setValue(v);
    else
      throw conversion_error();
  }
  else
    setValue(nullptr);
}
