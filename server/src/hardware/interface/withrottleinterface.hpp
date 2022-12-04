/**
 * server/src/hardware/interface/withrottleinterface.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_WITHROTTLEINTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_WITHROTTLEINTERFACE_HPP

#include "interface.hpp"
#include "../throttle/throttlecontroller.hpp"
#include "../protocol/withrottle/kernel.hpp"
#include "../protocol/withrottle/settings.hpp"
#include "../../core/objectproperty.hpp"

/**
 * \brief WiThrottle hardware interface
 */
class WiThrottleInterface
  : public Interface
  , public ThrottleController
{
  CLASS_ID("interface.withrottle")
  DEFAULT_ID("withrottle")
  CREATE(WiThrottleInterface)

  private:
    std::unique_ptr<WiThrottle::Kernel> m_kernel;

  protected:
    void worldEvent(WorldState state, WorldEvent event) final;

    bool setOnline(bool& value, bool simulation) final;

  public:
    Property<uint16_t> port;
    ObjectProperty<WiThrottle::Settings> wiThrottle;

    WiThrottleInterface(World& world, std::string_view _id);
};

#endif
