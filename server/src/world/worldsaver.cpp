/**
 * server/src/world/worldsaver.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#include "worldsaver.hpp"
#include <fstream>
#include <boost/uuid/uuid_io.hpp>
#include "world.hpp"

using nlohmann::json;

WorldSaver::WorldSaver(const World& world)
{
  json objects = json::array();
  for(auto& it : world.m_objects)
    if(ObjectPtr object = it.second.lock())
      objects.push_back(saveObject(object));

  json data;
  data["uuid"] = to_string(world.m_uuid);
  data[world.name.name()] = world.name.toJSON();
  data[world.scale.name()] = world.scale.toJSON();
  data["objects"] = objects;

  std::filesystem::path dir = std::filesystem::path(world.m_filename).remove_filename();
  std::string s = dir.string();
  if(!std::filesystem::is_directory(dir))
    std::filesystem::create_directories(dir);

  std::ofstream file(world.m_filename);
  if(file.is_open())
  {
    file << data.dump(2);
    //Traintastic::instance->console->notice(classId, "Saved world " + name.value());
  }
  else
    throw std::runtime_error("file not open");
    //Traintastic::instance->console->critical(classId, "Can't write to world file");
}

json WorldSaver::saveObject(const ObjectPtr& object)
{
  json objectData;

  objectData["class_id"] = object->getClassId();

  if(AbstractObjectList* list = dynamic_cast<AbstractObjectList*>(object.get()))
  {
    json objects = json::array();
    for(auto& item: list->getItems())
      if(IdObject* idObject = dynamic_cast<IdObject*>(item.get()))
        objects.push_back(idObject->id);
      else
        assert(false);
    objectData["objects"] = objects;
  }
  else if(DecoderFunction* function = dynamic_cast<DecoderFunction*>(object.get()))
    objectData["decoder"] = function->decoder().id.toJSON();

  for(auto& item : object->interfaceItems())
    if(AbstractProperty* property = dynamic_cast<AbstractProperty*>(&item.second))
    {
      if(!property->isStoreable())
        continue;

      if(property->type() == ValueType::Object)
      {
        if(ObjectPtr value = property->toObject())
        {
          if(IdObject* idObject = dynamic_cast<IdObject*>(value.get()))
            objectData[property->name()] = idObject->id.toJSON();
          else if(SubObject* subObject = dynamic_cast<SubObject*>(value.get()))
          {
            if((property->flags() & PropertyFlags::SubObject) == PropertyFlags::SubObject)
              objectData[property->name()] = saveObject(value);
            else
              objectData[property->name()] = subObject->id();
          }
        }
        else
          objectData[property->name()] = nullptr;
      }
      else
        objectData[property->name()] = property->toJSON();
    }

  return objectData;
}
