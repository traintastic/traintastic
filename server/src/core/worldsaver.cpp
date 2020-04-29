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
  data[world.name.name()] = world.name.value();
  data[world.scale.name()] = world.scale.value();
  data["objects"] = objects;

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

  if(Hardware::DecoderFunction* function = dynamic_cast<Hardware::DecoderFunction*>(object.get()))
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
