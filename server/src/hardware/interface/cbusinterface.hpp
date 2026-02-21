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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_CBUSINTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_CBUSINTERFACE_HPP

#include "interface.hpp"
#include "../../core/serialdeviceproperty.hpp"
#include <traintastic/enum/cbusinterfacetype.hpp>

class CBUSSettings;

namespace CBUS {
class Kernel;
class Simulator;
}

/**
 * @brief CBUS hardware interface
 */
class CBUSInterface final
  : public Interface
{
  CLASS_ID("interface.cbus")
  DEFAULT_ID("cbus")
  CREATE_DEF(CBUSInterface)

public:
  Property<CBUSInterfaceType> type;
  SerialDeviceProperty device;
  Property<std::string> hostname;
  Property<uint16_t> port;
  ObjectProperty<CBUSSettings> cbus;

  CBUSInterface(World& world, std::string_view _id);
  ~CBUSInterface() final;

  //! \brief Send CBUS/VLCB message
  //! \param[in] message CBUS/VLCB message bytes, 1..8 bytes.
  //! \return \c true if send, \c false otherwise.
  bool send(std::vector<uint8_t> message);

  //! \brief Send DCC packet
  //! \param[in] dccPacket DCC packet byte, exluding checksum. Length is limited to 6.
  //! \param[in] repeat DCC packet repeat count 0..7
  //! \return \c true if send, \c false otherwise.
  bool sendDCC(std::vector<uint8_t> dccPacket, uint8_t repeat);

protected:
  void addToWorld() final;
  void loaded() final;
  void destroying() final;
  void worldEvent(WorldState state, WorldEvent event) final;

  bool setOnline(bool& value, bool simulation) final;

private:
  std::unique_ptr<CBUS::Kernel> m_kernel;
  std::unique_ptr<CBUS::Simulator> m_simulator;
  boost::signals2::connection m_cbusPropertyChanged;

  void updateVisible();
};

#endif
