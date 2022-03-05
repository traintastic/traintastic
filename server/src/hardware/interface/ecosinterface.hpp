/**
 * server/src/hardware/interface/ecosinterface.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_ECOSINTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_ECOSINTERFACE_HPP

#include "interface.hpp"
#include "../protocol/ecos/kernel.hpp"
#include "../protocol/ecos/settings.hpp"
#include "../decoder/decodercontroller.hpp"
#include "../decoder/list/decoderlist.hpp"
#include "../input/inputcontroller.hpp"
#include "../input/list/inputlist.hpp"
#include "../output/outputcontroller.hpp"
#include "../output/list/outputlist.hpp"
#include "../../core/objectproperty.hpp"

/**
 * @brief ECoS hardware interface
 */
class ECoSInterface final
  : public Interface
  , public DecoderController
  , public InputController
  , public OutputController
{
  CLASS_ID("interface.ecos")
  DEFAULT_ID("ecos")
  CREATE(ECoSInterface)

  private:
    std::unique_ptr<ECoS::Kernel> m_kernel;
    boost::signals2::connection m_ecosPropertyChanged;

    void addToWorld() final;
    void destroying() final;
    void worldEvent(WorldState state, WorldEvent event) final;

    void idChanged(const std::string& newId) final;

    void typeChanged();

  protected:
    bool setOnline(bool& value, bool simulation) final;

  public:
    Property<std::string> hostname;
    ObjectProperty<ECoS::Settings> ecos;
    ObjectProperty<DecoderList> decoders;
    ObjectProperty<InputList> inputs;
    ObjectProperty<OutputList> outputs;
    Property<std::string> testCommand;
    Method<void()> testCommandSend;

    ECoSInterface(World& world, std::string_view _id);

    // DecoderController:
    [[nodiscard]] bool addDecoder(Decoder& decoder) final;
    [[nodiscard]] bool removeDecoder(Decoder& decoder) final;
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) final;

    // InputController:
    std::pair<uint32_t, uint32_t> inputAddressMinMax(uint32_t /*channel*/) const final { return {1, 1}; }
    [[nodiscard]] bool addInput(Input& input) final;
    [[nodiscard]] bool removeInput(Input& input) final;

    // OutputController:
    std::pair<uint32_t, uint32_t> outputAddressMinMax(uint32_t /*channel*/) const final { return {1, 1}; }
    [[nodiscard]] bool addOutput(Output& output) final;
    [[nodiscard]] bool removeOutput(Output& output) final;
    [[nodiscard]] bool setOutputValue(uint32_t channel, uint32_t address, bool value) final;
};

#endif
