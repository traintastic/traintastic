/**
 * server/src/hardware/interface/traintasticdiyinterface.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_TRAINTASTICDIYINTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_TRAINTASTICDIYINTERFACE_HPP

#include "interface.hpp"
#include "../input/inputcontroller.hpp"
#include "../output/outputcontroller.hpp"
#include "../../core/serialdeviceproperty.hpp"
#include "../../core/objectproperty.hpp"
#include "../../enum/traintasticdiyinterfacetype.hpp"
#include "../../enum/serialflowcontrol.hpp"

namespace TraintasticDIY {
class Kernel;
class Settings;
}

/**
 * \brief Traintastic DIY hardware interface
 */
class TraintasticDIYInterface final
  : public Interface
  , public InputController
  , public OutputController
{
  CLASS_ID("interface.traintastic_diy")
  DEFAULT_ID("traintastic_diy")
  CREATE_DEF(TraintasticDIYInterface)

  private:
    std::unique_ptr<TraintasticDIY::Kernel> m_kernel;
    boost::signals2::connection m_traintasticDIYPropertyChanged;

    void addToWorld() final;
    void loaded() final;
    void destroying() final;

    void updateVisible();

  protected:
    bool setOnline(bool& value, bool simulation) final;

  public:
    Property<TraintasticDIYInterfaceType> type;
    SerialDeviceProperty device;
    Property<uint32_t> baudrate;
    Property<SerialFlowControl> flowControl;
    Property<std::string> hostname;
    Property<uint16_t> port;
    ObjectProperty<TraintasticDIY::Settings> traintasticDIY;

    TraintasticDIYInterface(World& world, std::string_view _id);

    // InputController:
    std::pair<uint32_t, uint32_t> inputAddressMinMax(uint32_t /*channel*/) const final;
    void inputSimulateChange(uint32_t channel, uint32_t address, SimulateInputAction action) final;

    // OutputController:
    std::pair<uint32_t, uint32_t> outputAddressMinMax(uint32_t /*channel*/) const final;
    [[nodiscard]] bool setOutputValue(uint32_t channel, uint32_t address, bool value) final;
};

#endif
