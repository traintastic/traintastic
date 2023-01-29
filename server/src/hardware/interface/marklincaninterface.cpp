/**
 * server/src/hardware/interface/marklincaninterface.cpp
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

#include "marklincaninterface.hpp"
#include "../decoder/list/decoderlist.hpp" // ????
#include "../decoder/list/decoderlisttablemodel.hpp"
#include "../protocol/marklincan/iohandler/simulationiohandler.hpp"
#include "../protocol/marklincan/iohandler/tcpiohandler.hpp"
#include "../protocol/marklincan/iohandler/udpiohandler.hpp"
#include "../../core/attributes.hpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/displayname.hpp"

constexpr auto decoderListColumns = DecoderListColumn::Id | DecoderListColumn::Name | DecoderListColumn::Address;

MarklinCANInterface::MarklinCANInterface(World& world, std::string_view _id)
  : Interface(world, _id)
  , DecoderController(*this, decoderListColumns)
  , type{this, "type", MarklinCANInterfaceType::TCP, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , hostname{this, "hostname", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  name = "M\u00E4rklin CAN";

  Attributes::addDisplayName(type, DisplayName::Interface::type);
  Attributes::addEnabled(type, !online);
  Attributes::addValues(type, marklinCANInterfaceTypeValues);
  m_interfaceItems.insertBefore(type, notes);

  Attributes::addDisplayName(hostname, DisplayName::IP::hostname);
  Attributes::addEnabled(hostname, !online);
  m_interfaceItems.insertBefore(hostname, notes);

  m_interfaceItems.insertBefore(decoders, notes);
}

void MarklinCANInterface::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(m_kernel)
    m_kernel->decoderChanged(decoder, changes, functionNumber);
}

bool MarklinCANInterface::setOnline(bool& value, bool simulation)
{
  if(!m_kernel && value)
  {
    try
    {
      const MarklinCAN::Config config{true};

      if(simulation)
      {
        m_kernel = MarklinCAN::Kernel::create<MarklinCAN::SimulationIOHandler>(config);
      }
      else
      {
        switch(type.value())
        {
          case MarklinCANInterfaceType::TCP:
            m_kernel = MarklinCAN::Kernel::create<MarklinCAN::TCPIOHandler>(config, hostname.value());
            break;

          case MarklinCANInterfaceType::UDP:
            m_kernel = MarklinCAN::Kernel::create<MarklinCAN::UDPIOHandler>(config, hostname.value());
            break;
        }
      }
      assert(m_kernel);

      status.setValueInternal(InterfaceStatus::Initializing);

      m_kernel->setLogId(id.value());

      m_kernel->setOnStarted(
        [this]()
        {
          status.setValueInternal(InterfaceStatus::Online);
        });

      m_kernel->setDecoderController(this);

      m_kernel->start();

      Attributes::setEnabled({type, hostname}, false);
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
    Attributes::setEnabled({type, hostname}, true);

    m_kernel->stop();
    m_kernel.reset();

    status.setValueInternal(InterfaceStatus::Offline);
  }
  return true;
}

void MarklinCANInterface::addToWorld()
{
  Interface::addToWorld();
  DecoderController::addToWorld();
}

void MarklinCANInterface::destroying()
{
  DecoderController::destroying();
  Interface::destroying();
}

void MarklinCANInterface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);

  if(m_kernel)
  {
    switch(event)
    {
      case WorldEvent::PowerOff:
        m_kernel->systemStop();
        break;

      case WorldEvent::PowerOn:
        m_kernel->systemGo();
        m_kernel->systemHalt();
        break;

      case WorldEvent::Stop:
        m_kernel->systemHalt();
        break;

      case WorldEvent::Run:
        m_kernel->systemGo();
        break;

      default:
        break;
    }
  }
}

void MarklinCANInterface::idChanged(const std::string& newId)
{
  if(m_kernel)
    m_kernel->setLogId(newId);
}
