#ifndef WORLDLOADER_HPP
#define WOLRDLOADER_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "stdfilesystem.hpp"

class Object;
class World;

class WorldLoader
{
  private:
    struct ObjectData
    {
      nlohmann::json json;
      std::shared_ptr<Object> object;
      bool loaded;
    };

    std::shared_ptr<World> m_world;
    std::unordered_map<std::string, ObjectData> m_objects;

    void createObject(ObjectData& objectData);
    void loadObject(ObjectData& objectData);

  public:
    WorldLoader(const std::filesystem::path& filename);

    std::shared_ptr<World> world() { return m_world; }
};


#endif
