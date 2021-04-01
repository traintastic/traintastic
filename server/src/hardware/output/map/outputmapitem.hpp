/**
 * server/src/hardware/output/map/outputmapitem.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAPITEM_HPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAPITEM_HPP

#include "../../../core/object.hpp"
#include <vector>

class OutputMapOutputAction;

class OutputMapItem : public Object
{
  public:
    using OutputActions = std::vector<std::shared_ptr<OutputMapOutputAction>>;

  protected:
    Object& m_map;
    OutputActions m_outputActions;

    void load(WorldLoader& loader, const nlohmann::json& data) override;
    void save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const override;
    void worldEvent(WorldState state, WorldEvent event) override;

  public:
    OutputMapItem(Object& map);

    const OutputActions& outputActions() const;

    void execute();
};

#endif
