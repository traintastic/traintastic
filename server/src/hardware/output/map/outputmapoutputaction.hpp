/**
 * server/src/hardware/output/map/outputmapoutputaction.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAPOUTPUTACTION_HPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAPOUTPUTACTION_HPP

#include "../../../core/object.hpp"
#include "../../../enum/outputaction.hpp"
#include "../output.hpp"

class OutputMapOutputAction : public Object
{
  CLASS_ID("output_map_output_action")

  friend class OutputMapItem;

  protected:
    Object& m_parent;
    std::shared_ptr<Output> m_output;

    void save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const override;
    void worldEvent(WorldState state, WorldEvent event) override;

  public:
    Property<OutputAction> action;

    OutputMapOutputAction(Object& _parent, std::shared_ptr<Output> _output);

    std::string getObjectId() const final;

    const std::shared_ptr<Output>& output() const { return m_output; }

    void execute();
};

#endif
