#ifndef WORLDSAVER_HPP
#define WOLRDSAVER_HPP

//#include <memory>
//#include <string>
//#include <unordered_map>
#include <nlohmann/json.hpp>
#include "objectptr.hpp"
//#include "stdfilesystem.hpp"

//class Object;
class World;

class WorldSaver
{
  private:
    nlohmann::json saveObject(const ObjectPtr& object);

  public:
    WorldSaver(const World& world);
};

#endif
