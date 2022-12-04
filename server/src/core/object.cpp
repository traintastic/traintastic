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
#include "idobject.hpp"
#include "subobject.hpp"
#include "abstractmethod.hpp"
#include "abstractproperty.hpp"
#include "abstractobjectproperty.hpp"
#include "abstractvectorproperty.hpp"
#include "../world/worldloader.hpp"
#include "../world/worldsaver.hpp"

Object::Object() :
  m_dying{false}
{
}

void Object::destroy()
{
  assert(!m_dying);
  if(!m_dying)
  {
    m_dying = true;
    destroying();
    onDestroying(*this);
  }
}

const InterfaceItem* Object::getItem(std::string_view name) const
{
  return m_interfaceItems.find(name);
}

InterfaceItem* Object::getItem(std::string_view name)
{
  return m_interfaceItems.find(name);
}

const AbstractMethod* Object::getMethod(std::string_view name) const
{
  return dynamic_cast<const AbstractMethod*>(getItem(name));
}

AbstractMethod* Object::getMethod(std::string_view name)
{
  return dynamic_cast<AbstractMethod*>(getItem(name));
}

const AbstractProperty* Object::getProperty(std::string_view name) const
{
  return dynamic_cast<const AbstractProperty*>(getItem(name));
}

AbstractProperty* Object::getProperty(std::string_view name)
{
  return dynamic_cast<AbstractProperty*>(getItem(name));
}

const AbstractObjectProperty* Object::getObjectProperty(std::string_view name) const
{
  return dynamic_cast<const AbstractObjectProperty*>(getItem(name));
}

AbstractObjectProperty* Object::getObjectProperty(std::string_view name)
{
  return dynamic_cast<AbstractObjectProperty*>(getItem(name));
}

const AbstractVectorProperty* Object::getVectorProperty(std::string_view name) const
{
  return dynamic_cast<const AbstractVectorProperty*>(getItem(name));
}

AbstractVectorProperty* Object::getVectorProperty(std::string_view name)
{
  return dynamic_cast<AbstractVectorProperty*>(getItem(name));
}

void Object::load(WorldLoader& loader, const nlohmann::json& data)
{
  for(auto& [name, value] : data.items())
  {
    if(AbstractProperty* property = getProperty(name))
    {
      if(property->type() == ValueType::Object)
      {
        if(contains(property->flags(), PropertyFlags::SubObject))
        {
          property->toObject()->load(loader, value);
        }
        else
        {
          if(value.is_string())
            property->loadObject(loader.getObject(value.get<std::string_view>()));
          else if(value.is_null())
            property->loadObject(ObjectPtr());
        }
      }
      else
        property->loadJSON(value);
    }
    else if(AbstractVectorProperty* vectorProperty = getVectorProperty(name))
    {
      if(vectorProperty->type() == ValueType::Object)
      {
        if(contains(vectorProperty->flags(), PropertyFlags::SubObject))
        {
          const size_t size = std::min(value.size(), vectorProperty->size());
          for(size_t i = 0; i < size; i++)
            vectorProperty->getObject(i)->load(loader, value[i]);
        }
        else
        {
          assert(false);
          //if(value.is_string())
          //  property->load(loader.getObject(value));
          //else if(value.is_null())
          //  property->load(ObjectPtr());
        }
      }
      else
        vectorProperty->loadJSON(value);
    }
  }

  // state values (optional):
  nlohmann::json state = loader.getState(getObjectId());
  for(auto& [name, value] : state.items())
  {
    if(AbstractProperty* property = getProperty(name))
      property->loadJSON(value);
    else if(AbstractVectorProperty* vectorProperty = getVectorProperty(name))
      vectorProperty->loadJSON(value);
  }
}

void Object::save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const
{
  data["class_id"] = getClassId();

  for(const auto& item : interfaceItems())
    if(BaseProperty* baseProperty = dynamic_cast<BaseProperty*>(&item.second))
    {
      if(baseProperty->isStoreable())
      {
        const std::string name{baseProperty->name()};

        if(baseProperty->type() == ValueType::Object)
        {
          if(AbstractProperty* property = dynamic_cast<AbstractProperty*>(baseProperty))
          {
            if(ObjectPtr value = property->toObject())
            {
              if(IdObject* idObject = dynamic_cast<IdObject*>(value.get()))
                data[name] = idObject->id.toJSON();
              else if(SubObject* subObject = dynamic_cast<SubObject*>(value.get()))
              {
                if((property->flags() & PropertyFlags::SubObject) == PropertyFlags::SubObject)
                  data[name] = saver.saveObject(value);
                else
                  data[name] = subObject->getObjectId();
              }
            }
            else
              data[name] = nullptr;
          }
          else if(AbstractVectorProperty* vectorProperty = dynamic_cast<AbstractVectorProperty*>(baseProperty))
          {
            nlohmann::json values(nlohmann::json::value_t::array);

            for(size_t i = 0; i < vectorProperty->size(); i++)
            {
              if(ObjectPtr value = vectorProperty->getObject(i))
              {
                if((vectorProperty->flags() & PropertyFlags::SubObject) == PropertyFlags::SubObject)
                  values.emplace_back(saver.saveObject(value));
                else
                  values.emplace_back(value->getObjectId());
              }
              else
                values.emplace_back(nullptr);
            }

            data[name] = values;
          }
        }
        else
          data[name] = baseProperty->toJSON();
      }
      else if(baseProperty->isStateStoreable())
      {
        assert(baseProperty->type() != ValueType::Object);
        state[std::string{baseProperty->name()}] = baseProperty->toJSON();
      }
    }
}

void Object::loaded()
{
  for(const auto& it : m_interfaceItems)
  {
    if(AbstractProperty* property = dynamic_cast<AbstractProperty*>(&it.second);
        property && contains(property->flags(), PropertyFlags::SubObject))
    {
      property->toObject()->loaded();
    }
    else if(AbstractVectorProperty* vectorProperty = dynamic_cast<AbstractVectorProperty*>(&it.second);
        vectorProperty && contains(vectorProperty->flags(), PropertyFlags::SubObject))
    {
      const size_t size = vectorProperty->size();
      for(size_t i = 0; i < size; i++)
        vectorProperty->getObject(i)->loaded();
    }
  }
}

void Object::worldEvent(WorldState state, WorldEvent event)
{
  for(const auto& it : m_interfaceItems)
  {
    if(AbstractProperty* property = dynamic_cast<AbstractProperty*>(&it.second);
        property && contains(property->flags(), PropertyFlags::SubObject))
    {
      property->toObject()->worldEvent(state, event);
    }
    else if(AbstractVectorProperty* vectorProperty = dynamic_cast<AbstractVectorProperty*>(&it.second);
        vectorProperty && contains(vectorProperty->flags(), PropertyFlags::SubObject))
    {
      const size_t size = vectorProperty->size();
      for(size_t i = 0; i < size; i++)
        vectorProperty->getObject(i)->worldEvent(state, event);
    }
  }
}
