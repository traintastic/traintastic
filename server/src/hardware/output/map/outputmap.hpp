/**
 * server/src/hardware/output/map/outputmap.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023-2024 Reinder Feenstra
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
#include <unordered_map>
#include <optional>
#include <boost/signals2/signal.hpp>
#include "../../../core/method.hpp"
#include "../../../core/objectproperty.hpp"
#include "../../../core/vectorproperty.hpp"
#include "../../../core/objectvectorproperty.hpp"
#include "../output.hpp"
#include "outputactionvalue.hpp"

class OutputController;
class OutputMapItem;
class OutputMapOutputAction;

class OutputMap : public SubObject
{
  public:
    using Items = std::vector<std::shared_ptr<OutputMapItem>>;
    using OutputConnectionPair = std::pair<std::shared_ptr<Output>, boost::signals2::connection>;
    using Outputs = std::vector<OutputConnectionPair>;

  private:
    static constexpr size_t addressesSizeMin = 1;
    static constexpr size_t addressesSizeMax = 8;

    boost::signals2::connection m_interfaceDestroying;
    boost::signals2::connection m_outputECoSObjectsChanged;

    void addOutput(OutputChannel ch, uint32_t id);
    void addOutput(OutputChannel ch, uint32_t id, OutputController& outputController);
    OutputConnectionPair getOutput(OutputChannel ch, uint32_t id, OutputController& outputController);
    void releaseOutput(OutputConnectionPair& outputConnPair);

  protected:
    Outputs m_outputs;

    void load(WorldLoader& loader, const nlohmann::json& data) override;
    void loaded() override;
    void worldEvent(WorldState state, WorldEvent event) override;

    void interfaceChanged();
    void channelChanged();
    void addressesSizeChanged();
    void updateOutputActions();
    void updateOutputActions(OutputType outputType);
    void updateEnabled();

    uint32_t getUnusedAddress() const;
    std::shared_ptr<OutputMapOutputAction> createOutputAction(OutputType outputType, size_t index, std::optional<OutputActionValue> actionValue);

    virtual std::optional<OutputActionValue> getDefaultOutputActionValue(const OutputMapItem& item, OutputType outputType, size_t outputIndex) = 0;

    int getMatchingActionOnCurrentState();

    virtual void updateStateFromOutput();

  public:
    ObjectProperty<Object> parentObject; // UI needs access to parent object
    ObjectProperty<OutputController> interface;
    Property<OutputChannel> channel;
    VectorProperty<uint32_t> addresses;
    Property<uint16_t> ecosObject;
    ObjectVectorProperty<OutputMapItem> items;
    Method<void()> addAddress;
    Method<void()> removeAddress;

    OutputMap(Object& _parent, std::string_view parentPropertyName);
    ~OutputMap() override;

    const std::shared_ptr<Output>& output(size_t index) const
    {
      assert(index < m_outputs.size());
      return m_outputs[index].first;
    }
};

#endif
