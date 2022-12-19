/**
 * server/src/hardware/interface/traintasticdiyinterface.cpp
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

#include "traintasticdiyinterface.hpp"
#include "../protocol/traintasticdiy/messages.hpp"
#include "../protocol/traintasticdiy/iohandler/serialiohandler.hpp"
#include "../protocol/traintasticdiy/iohandler/simulationiohandler.hpp"
#include "../protocol/traintasticdiy/iohandler/tcpiohandler.hpp"
#include "../../core/attributes.hpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../world/world.hpp"

constexpr auto inputListColumns = InputListColumn::Id | InputListColumn::Name | InputListColumn::Address;
constexpr auto outputListColumns = OutputListColumn::Id | OutputListColumn::Name | OutputListColumn::Address;

TraintasticDIYInterface::TraintasticDIYInterface(World& world, std::string_view _id)
  : Interface(world, _id)
  , InputController(static_cast<IdObject&>(*this))
  , OutputController(static_cast<IdObject&>(*this))
  , type{this, "type", TraintasticDIYInterfaceType::Serial, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](TraintasticDIYInterfaceType /*value*/)
      {
        updateVisible();
      }}
  , device{this, "device", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , baudrate{this, "baudrate", 19200, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , flowControl{this, "flow_control", SerialFlowControl::None, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , hostname{this, "hostname", "192.168.1.203", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , port{this, "port", 5550, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , traintasticDIY{this, "traintastic_diy", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "Traintastic DIY";
  traintasticDIY.setValueInternal(std::make_shared<TraintasticDIY::Settings>(*this, traintasticDIY.name()));

  Attributes::addDisplayName(type, DisplayName::Interface::type);
  Attributes::addEnabled(type, !online);
  Attributes::addValues(type, traintasticDIYInterfaceTypeValues);
  m_interfaceItems.insertBefore(type, notes);

  Attributes::addEnabled(device, !online);
  Attributes::addVisible(device, false);
  m_interfaceItems.insertBefore(device, notes);

  Attributes::addDisplayName(baudrate, DisplayName::Serial::baudrate);
  Attributes::addEnabled(baudrate, !online);
  Attributes::addVisible(baudrate, false);
  m_interfaceItems.insertBefore(baudrate, notes);

  Attributes::addDisplayName(flowControl, DisplayName::Serial::flowControl);
  Attributes::addEnabled(flowControl, !online);
  Attributes::addValues(flowControl, SerialFlowControlValues);
  Attributes::addVisible(flowControl, false);
  m_interfaceItems.insertBefore(flowControl, notes);

  Attributes::addDisplayName(hostname, DisplayName::IP::hostname);
  Attributes::addEnabled(hostname, !online);
  Attributes::addVisible(hostname, false);
  m_interfaceItems.insertBefore(hostname, notes);

  Attributes::addDisplayName(port, DisplayName::IP::port);
  Attributes::addEnabled(port, !online);
  Attributes::addVisible(port, false);
  m_interfaceItems.insertBefore(port, notes);

  m_interfaceItems.insertBefore(traintasticDIY, notes);

  m_interfaceItems.insertBefore(inputs, notes);

  m_interfaceItems.insertBefore(outputs, notes);

  updateVisible();
}

void TraintasticDIYInterface::inputSimulateChange(uint32_t channel, uint32_t address)
{
  if(m_kernel && inRange(address, outputAddressMinMax(channel)))
    m_kernel->simulateInputChange(address);
}

bool TraintasticDIYInterface::setOutputValue(uint32_t channel, uint32_t address, bool value)
{
  assert(isOutputChannel(channel));
  return
    m_kernel &&
    inRange(address, outputAddressMinMax(channel)) &&
    m_kernel->setOutput(static_cast<uint16_t>(address), value);
}

bool TraintasticDIYInterface::setOnline(bool& value, bool simulation)
{
  if(!m_kernel && value)
  {
    try
    {
      if(simulation)
      {
        m_kernel = TraintasticDIY::Kernel::create<TraintasticDIY::SimulationIOHandler>(m_world, traintasticDIY->config());
      }
      else
      {
        switch(type)
        {
          case TraintasticDIYInterfaceType::Serial:
            m_kernel = TraintasticDIY::Kernel::create<TraintasticDIY::SerialIOHandler>(m_world, traintasticDIY->config(), device.value(), baudrate.value(), flowControl.value());
            break;

          case TraintasticDIYInterfaceType::NetworkTCP:
            m_kernel = TraintasticDIY::Kernel::create<TraintasticDIY::TCPIOHandler>(m_world, traintasticDIY->config(), hostname.value(), port.value());
            break;
        }
      }

      if(!m_kernel)
      {
        assert(false);
        return false;
      }

      status.setValueInternal(InterfaceStatus::Initializing);

      m_kernel->setLogId(id.value());
      m_kernel->setOnStarted(
        [this]()
        {
          status.setValueInternal(InterfaceStatus::Online);
        });

      m_kernel->setInputController(this);
      m_kernel->setOutputController(this);
      m_kernel->start();

      m_traintasticDIYPropertyChanged = traintasticDIY->propertyChanged.connect(
        [this](BaseProperty& /*property*/)
        {
          m_kernel->setConfig(traintasticDIY->config());
        });

      Attributes::setEnabled({type, device, baudrate, flowControl, hostname, port}, false);
    }
    catch(const LogMessageException& e)
    {
      status.setValueInternal(InterfaceStatus::Offline);
      Log::log(*this, e.message(), e.args());
      return false;
    }
  }
  else if(m_kernel && !value)
  {
    Attributes::setEnabled({type, device, baudrate, flowControl, hostname, port}, true);

    m_traintasticDIYPropertyChanged.disconnect();

    m_kernel->stop();
    m_kernel.reset();

    status.setValueInternal(InterfaceStatus::Offline);
  }
  return true;
}

void TraintasticDIYInterface::addToWorld()
{
  Interface::addToWorld();
  InputController::addToWorld(inputListColumns);
  OutputController::addToWorld(outputListColumns);
}

void TraintasticDIYInterface::loaded()
{
  Interface::loaded();

  updateVisible();
}

void TraintasticDIYInterface::destroying()
{
  OutputController::destroying();
  InputController::destroying();
  Interface::destroying();
}

void TraintasticDIYInterface::idChanged(const std::string& newId)
{
  if(m_kernel)
    m_kernel->setLogId(newId);
}

void TraintasticDIYInterface::updateVisible()
{
  const bool isSerial = (type == TraintasticDIYInterfaceType::Serial);
  Attributes::setVisible(device, isSerial);
  Attributes::setVisible(baudrate, isSerial);
  Attributes::setVisible(flowControl, isSerial);

  const bool isNetwork = (type == TraintasticDIYInterfaceType::NetworkTCP);
  Attributes::setVisible(hostname, isNetwork);
  Attributes::setVisible(port, isNetwork);
}
