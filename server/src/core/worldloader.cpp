#include "worldloader.hpp"
#include <fstream>
#include <boost/uuid/string_generator.hpp>
#include "world.hpp"

#include "../hardware/commandstation/li10x.hpp"
#include "../hardware/commandstation/usbxpressnetinterface.hpp"
#include "../hardware/commandstation/z21.hpp"
#include "../hardware/controller/z21app.hpp"
#include "../hardware/decoder/decoder.hpp"
#include "../hardware/decoder/decoderfunction.hpp"
#include "../lua/script.hpp"

using nlohmann::json;

WorldLoader::WorldLoader(const std::filesystem::path& filename) :
  m_world{World::create()}
{
  std::ifstream file(filename);
  if(!file.is_open())
    throw std::runtime_error("can't open " + filename.string());

  json data = json::parse(file);;

  m_world->m_filename = filename;
  m_world->m_uuid = boost::uuids::string_generator()(std::string(data["uuid"]));
  m_world->name = data[m_world->name.name()];

  // create a list of all objects
  for(json object : data["objects"])
  {
    if(auto it = object.find("id"); it != object.end())
      m_objects.insert({it.value().get<std::string>(), {object, nullptr, false}});
    else
      throw std::runtime_error("id missing");
  }

  // then create all objects
  for(auto& it : m_objects)
    if(!it.second.object)
      createObject(it.second);

  // and finally load their data
  for(auto& it : m_objects)
    if(!it.second.loaded)
      loadObject(it.second);
}

void WorldLoader::createObject(ObjectData& objectData)
{
  assert(!objectData.object);

  std::string_view classId = objectData.json["class_id"];
  std::string_view id = objectData.json["id"];

  // TODO some kind of class list !!
  if(classId == Hardware::CommandStation::LI10x::classId)
    objectData.object = Hardware::CommandStation::LI10x::create(m_world, id);
  else if(classId == Hardware::CommandStation::USBXpressNetInterface::classId)
    objectData.object = Hardware::CommandStation::USBXpressNetInterface::create(m_world, id);
  else if(classId == Hardware::CommandStation::Z21::classId)
    objectData.object = Hardware::CommandStation::Z21::create(m_world, id);
  else if(classId == Hardware::Controller::Z21App::classId)
    objectData.object = Hardware::Controller::Z21App::create(m_world, id);
  else if(classId == Hardware::Decoder::classId)
    objectData.object = Hardware::Decoder::create(m_world, id);
  else if(classId == Lua::Script::classId)
    objectData.object = Lua::Script::create(m_world, id);
  else if(classId == Hardware::DecoderFunction::classId)
  {
    //objectData.object = Hardware::Decoder::create(m_world, id);
  }

  if(!objectData.object)
    {};//m_objects.insert(id, object);
}

void WorldLoader::loadObject(ObjectData& objectData)
{
  /*assert*/if(!objectData.object)return;
  assert(!objectData.loaded);

  Object& object = *objectData.object;
  for(auto& [name, value] : objectData.json.items())
    if(AbstractProperty* property = object.getProperty(name))
      if(property->type() == ValueType::Object)
      {
        //hier voor objecten wat anders doen
      }
      else
        property->fromJSON(value);

  objectData.loaded = true;
  //objectData.object->loaded();
}

/*
std::shared_ptr<Object> WorldLoader::getObject(std::string_view id)
{

}
*/
