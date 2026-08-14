/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_RCLINK_RCLINKINTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_RCLINK_RCLINKINTERFACE_HPP

#include "../interface.hpp"
#include "../../../core/serialdeviceproperty.hpp"
#include "../../../core/objectproperty.hpp"

class RCLinkSettings;

namespace RCLink {
class Kernel;
}

/**
 * @brief RC-Link hardware interface
 */
class RCLinkInterface final
  : public Interface
{
  CLASS_ID("interface.rclink")
  DEFAULT_ID("rclink")
  CREATE_DEF(RCLinkInterface)

public:
  SerialDeviceProperty device;
  ObjectProperty<RCLinkSettings> rcLink;

  RCLinkInterface(World& world, std::string_view _id);
  ~RCLinkInterface() final;

protected:
  void addToWorld() final;
  void loaded() final;
  void destroying() final;
  void worldEvent(WorldState state, WorldEvent event) final;

  bool setOnline(bool& value, bool simulation) final;
  void onlineChanged(bool value) final;

private:
  std::unique_ptr<RCLink::Kernel> m_kernel;
  boost::signals2::connection m_rcLinkPropertyChanged;

  void updateEnabled();
};

#endif
