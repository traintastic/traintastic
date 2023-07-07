/**
 * server/src/hardware/interface/wlanmausinterface.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_WLANMAUSINTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_WLANMAUSINTERFACE_HPP

#include "interface.hpp"
#include "../../core/objectproperty.hpp"

namespace Z21 {
class ServerKernel;
class ServerSettings;
}

/**
 * @brief WLANmaus/Z21 app hardware interface
 */
class WlanMausInterface : public Interface
{
  CLASS_ID("interface.wlanmaus")
  DEFAULT_ID("wlanmaus")
  CREATE_DEF(WlanMausInterface)

  private:
    std::unique_ptr<Z21::ServerKernel> m_kernel;
    boost::signals2::connection m_z21PropertyChanged;

  protected:
    void worldEvent(WorldState state, WorldEvent event) final;
    void idChanged(const std::string& newId) final;
    bool setOnline(bool& value, bool simulation) final;

  public:
    ObjectProperty<Z21::ServerSettings> z21;

    WlanMausInterface(World& world, std::string_view _id);
};

#endif

