/**
 * server/src/core/world.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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

#ifndef SERVER_CORE_WORLD_HPP
#define SERVER_CORE_WORLD_HPP

#include "object.hpp"
#include "property.hpp"
#include "stdfilesystem.hpp"
#include <boost/uuid/uuid.hpp>

class CommandStationList;
class DecoderList;

class World : public Object
{
  protected:
    static const std::string id;

    std::filesystem::path m_filename;
    boost::uuids::uuid m_uuid;

    void load();
    void save();

  public:
    CLASS_ID("world");

    Property<std::string> name;
    std::shared_ptr<CommandStationList> commandStationList; // TODO: make property
    std::shared_ptr<DecoderList> decoderList; // TODO: make property

    World();
    World(const std::filesystem::path& filename);

    const boost::uuids::uuid& uuid() const { return m_uuid; }
};

#endif
