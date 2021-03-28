/**
 * server/src/core/abstractobjectlist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#include "abstractobjectlist.hpp"
#include "../world/worldloader.hpp"

AbstractObjectList::AbstractObjectList(Object& _parent, const std::string& parentPropertyName) :
  SubObject{_parent, parentPropertyName}
{
}

void AbstractObjectList::load(WorldLoader& loader, const nlohmann::json& data)
{
  SubObject::load(loader, data);

  nlohmann::json objects = data.value("objects", nlohmann::json::array());
  std::vector<ObjectPtr> items;
  items.reserve(objects.size());
  for(auto& [_, id] : objects.items())
    if(ObjectPtr item = loader.getObject(id))
      items.emplace_back(std::move(item));
  setItems(items);
}
