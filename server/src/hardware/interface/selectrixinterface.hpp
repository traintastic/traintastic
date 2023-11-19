/**
 * server/src/hardware/interface/selectrixinterface.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_SELECTRIXINTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_SELECTRIXINTERFACE_HPP

#include "interface.hpp"
#include "../decoder/decodercontroller.hpp"
#include "../input/inputcontroller.hpp"
#include "../output/outputcontroller.hpp"
#include "../../core/serialdeviceproperty.hpp"
#include "../../core/objectproperty.hpp"
#include "../../enum/serialflowcontrol.hpp"
#include <tcb/span.hpp>

namespace Selectrix {
  class Kernel;
  class Settings;
}

/**
 * \brief Selectrix hardware interface
 */
class SelectrixInterface final
  : public Interface
{
  CLASS_ID("interface.selectrix")
  DEFAULT_ID("selectrix")
  CREATE_DEF(SelectrixInterface)

  private:
    std::unique_ptr<Selectrix::Kernel> m_kernel;
    boost::signals2::connection m_selectrixPropertyChanged;

    void addToWorld() final;
    void loaded() final;
    void destroying() final;
    void worldEvent(WorldState state, WorldEvent event) final;

  protected:
    bool setOnline(bool& value, bool simulation) final;

  public:
    SerialDeviceProperty device;
    Property<uint32_t> baudrate;
    Property<SerialFlowControl> flowControl;
    ObjectProperty<Selectrix::Settings> selectrix;

    SelectrixInterface(World& world, std::string_view _id);

};

#endif
