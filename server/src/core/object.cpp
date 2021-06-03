/**
 * server/src/core/object.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#include "object.hpp"
#include "abstractmethod.hpp"
#include "abstractproperty.hpp"
#include "traintastic.hpp"
#include "../world/worldloader.hpp"
#include "../world/worldsaver.hpp"

Object::Object() :
  m_dying{false}
{
}

Object::~Object()
{
}

void Object::destroy()
{
  assert(!m_dying);
  if(!m_dying)
  {
    m_dying = true;
    destroying();
  }
}

InterfaceItem* Object::getItem(std::string_view name)
{
  return m_interfaceItems.find(name);
}

AbstractMethod* Object::getMethod(std::string_view name)
{
  return dynamic_cast<AbstractMethod*>(getItem(name));
}

AbstractProperty* Object::getProperty(std::string_view name)
{
  return dynamic_cast<AbstractProperty*>(getItem(name));
}

void Object::load(WorldLoader& loader, const nlohmann::json& data)
{
  for(auto& [name, value] : data.items())
    if(AbstractProperty* property = getProperty(name))
      if(property->type() == ValueType::Object)
      {
        if(contains(property->flags(), PropertyFlags::SubObject))
        {
          //loadObject(*property->toObject(), value);
          property->toObject()->load(loader, value);
        }
        else
        {
          if(value.is_string())
            property->load(loader.getObject(value));
          else if(value.is_null())
            property->load(ObjectPtr());
        }
      }
      else
        property->load(value);

  // state values (optional):
  nlohmann::json state = loader.getState(getObjectId());
  for(auto& [name, value] : state.items())
    if(AbstractProperty* property = getProperty(name))
      property->load(value);
}

void Object::save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const
{
  data["class_id"] = getClassId();

  for(auto& item : interfaceItems())
    if(AbstractProperty* property = dynamic_cast<AbstractProperty*>(&item.second))
    {
      if(property->isStoreable())
      {
        if(property->type() == ValueType::Object)
        {
          if(ObjectPtr value = property->toObject())
          {
            if(IdObject* idObject = dynamic_cast<IdObject*>(value.get()))
              data[property->name()] = idObject->id.toJSON();
            else if(SubObject* subObject = dynamic_cast<SubObject*>(value.get()))
            {
              if((property->flags() & PropertyFlags::SubObject) == PropertyFlags::SubObject)
                data[property->name()] = saver.saveObject(value);
              else
                data[property->name()] = subObject->getObjectId();
            }
          }
          else
            data[property->name()] = nullptr;
        }
        else
          data[property->name()] = property->toJSON();
      }
      else if(property->isStateStoreable())
      {
        assert(property->type() != ValueType::Object);
        state[property->name()] = property->toJSON();
      }
    }
}

void Object::loaded()
{
  for(auto& it : m_interfaceItems)
    if(AbstractProperty* property = dynamic_cast<AbstractProperty*>(&it.second))
      if(contains(property->flags(), PropertyFlags::SubObject))
        property->toObject()->loaded();
}

void Object::worldEvent(WorldState state, WorldEvent event)
{
  for(auto& it : m_interfaceItems)
    if(AbstractProperty* property = dynamic_cast<AbstractProperty*>(&it.second))
      if(contains(property->flags(), PropertyFlags::SubObject))
        property->toObject()->worldEvent(state, event);
}

void Object::logDebug(const std::string& message)
{
  Traintastic::instance->console->debug(getObjectId(), message);
}

void Object::logInfo(const std::string& message)
{
  Traintastic::instance->console->info(getObjectId(), message);
}

void Object::logNotice(const std::string& message)
{
  Traintastic::instance->console->notice(getObjectId(), message);
}

void Object::logWarning(const std::string& message)
{
  Traintastic::instance->console->warning(getObjectId(), message);
}

void Object::logError(const std::string& message)
{
  Traintastic::instance->console->error(getObjectId(), message);
}

void Object::logCritical(const std::string& message)
{
  Traintastic::instance->console->critical(getObjectId(), message);
}

void Object::logFatal(const std::string& message)
{
  Traintastic::instance->console->fatal(getObjectId(), message);
}
