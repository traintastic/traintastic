/**
 * server/src/hardware/output/map/outputmap.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAP_HPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAP_HPP

#include "../../../core/subobject.hpp"
#include <vector>
#include <boost/signals2/signal.hpp>
#include "../../../core/method.hpp"
#include "../output.hpp"

class OutputMapItem;

class OutputMap : public SubObject
{
  public:
    using Items = std::vector<std::shared_ptr<OutputMapItem>>;
    using Outputs = std::vector<std::shared_ptr<Output>>;

  private:
    std::map<std::shared_ptr<Output>, boost::signals2::connection> m_destroyingConnections;

  protected:
    Outputs m_outputs;

    void destroying() override;
    void load(WorldLoader& loader, const nlohmann::json& data) override;
    void save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const override;
    void worldEvent(WorldState state, WorldEvent event) override;

    virtual void outputAdded(const std::shared_ptr<Output>& output) = 0;
    virtual void outputRemoved(const std::shared_ptr<Output>& output) = 0;

  public:
    boost::signals2::signal<void (OutputMap&)> outputsChanged;

    Method<void(std::shared_ptr<Output>)> addOutput;
    Method<void(const std::shared_ptr<Output>&)> removeOutput;

    OutputMap(Object& _parent, std::string_view parentPropertyName);

    virtual Items items() const = 0;
    const Outputs& outputs() const { return m_outputs; }
};

#endif
