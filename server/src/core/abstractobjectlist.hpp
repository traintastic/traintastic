/**
 * server/src/core/abstractobjectlist.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_ABSTRACTOBJECTLIST_HPP
#define TRAINTASTIC_SERVER_CORE_ABSTRACTOBJECTLIST_HPP

#include "subobject.hpp"
#include "table.hpp"
#include "property.hpp"

class WorldSaver;

class AbstractObjectList : public SubObject, public Table
{
  friend class WorldSaver;

  protected:
    void load(WorldLoader& loader, const nlohmann::json& data) override;
    void save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const override;

    virtual std::vector<ObjectPtr> getItems() const = 0;
    virtual void setItems(const std::vector<ObjectPtr>& items) = 0;

  public:
    Property<uint32_t> length;

    AbstractObjectList(Object& _parent, const std::string& parentPropertyName);

    virtual ObjectPtr getObject(uint32_t index) = 0;
};

#endif
