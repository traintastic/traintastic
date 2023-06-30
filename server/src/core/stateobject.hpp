/**
 * server/src/core/stateobject.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CORE_STATEOBJECT_HPP
#define TRAINTASTIC_SERVER_CORE_STATEOBJECT_HPP

#include "object.hpp"

class World;

//! \brief Object with storable state information
//! Properties can't have the \ref PropertyFlags::Store flag set.
//! As this object only contains state information the world should load without restoring any state data,
//! including these state object's.
class StateObject : public Object
{
private:
  std::string m_id;

protected:
  void save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const override;

public:
  static void addToWorld(World& world, StateObject& object);

  StateObject(std::string id);

  std::string getObjectId() const final
  {
    return m_id;
  }
};

#endif
