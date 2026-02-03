/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025-2026 Reinder Feenstra
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

#include "yd7403boosterdriver.hpp"
#include <cassert>
#include "../../interface/loconetinterface.hpp"
#include "../../protocol/loconet/message/yamorc/smartbooster.hpp"
#include "../../../core/attributes.hpp"
#include "../../../utils/displayname.hpp"

namespace {

constexpr float voltageScale = 0.1f; //!< 100mV

}

YD7403BoosterDriver::YD7403BoosterDriver(Booster& booster)
  : LocoNetBoosterDriver(booster)
  , address{this, "address", addressDefault, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript,
    [this](uint8_t /*value*/)
    {
      if(interface && interface->online)
      {
        disableEvents();
        enableEvents();
      }
    }}
{
  Attributes::addDisplayName(address, DisplayName::Hardware::address);
  Attributes::addEnabled(address, false);
  m_interfaceItems.add(address);
}

void YD7403BoosterDriver::destroying()
{
  disableEvents();
  LocoNetBoosterDriver::destroying();
}

BoosterDriver::SupportedStatusValues YD7403BoosterDriver::supportedStatusValues() const
{
  using enum SupportedStatusValues;
  return (Load | Temperature | Current | Voltage | InputVoltage);
}

void YD7403BoosterDriver::interfaceOnlineChanged(bool value)
{
  LocoNetBoosterDriver::interfaceOnlineChanged(value);

  if(value)
  {
    m_currentScale = std::numeric_limits<float>::quiet_NaN(); // invalidate
    enableEvents();
  }
  else
  {
    disableEvents();
  }
}

void YD7403BoosterDriver::updateEnabled(bool editable, bool online)
{
  LocoNetBoosterDriver::updateEnabled(editable, online);
  Attributes::setEnabled(address, editable);
}

void YD7403BoosterDriver::enableEvents()
{
  using namespace LocoNet::YaMoRC;

  assert(interface);
  assert(interface->online);

  // register callback for receiving SmartBooster messages with address:
  m_onReceiveHandle = interface->registerOnReceive(
    {
      {0x7F, LocoNet::OPC_MULTI_SENSE},
      {0x6F, 0x60 | (address.value() >> 7)},
      {0x7F, address.value() & 0x7F}
    }, std::bind_front(&YD7403BoosterDriver::receive, this));

  interface->send(SmartBoosterSetUnsolicitedEvents(address, true));
  interface->send(SmartBoosterRequestAll(address));
}

void YD7403BoosterDriver::disableEvents()
{
  if(interface)
  {
    // unregister callback for received messages:
    interface->unregisterOnReceive(m_onReceiveHandle);
    m_onReceiveHandle = 0;

    if(interface->online)
    {
      interface->send(LocoNet::YaMoRC::SmartBoosterSetUnsolicitedEvents(address, false));
    }
  }
}

void YD7403BoosterDriver::receive(const LocoNet::Message& message)
{
  using namespace LocoNet::YaMoRC;

  assert(SmartBooster::match(message)); // enforced by filter

  const auto& smartBooster = static_cast<const SmartBooster&>(message);
  switch(smartBooster.payload())
  {
    using enum SmartBooster::Payload;

    case Temperature:
      reportTemperature(smartBooster.value());
      break;

    case Load:
      reportLoad(smartBooster.value());
      break;

    case CurrentUnits:
      if(std::isfinite(m_currentScale))
      {
        reportCurrent(smartBooster.value() * m_currentScale);
      }
      break;

    case OutputVoltage:
      reportVoltage(smartBooster.value() * voltageScale);
      break;

    case InputVoltage:
      reportInputVoltage(smartBooster.value() * voltageScale);
      break;

    //case MaxCurrentUnits:
    //case Source:

    case CurrentScale:
      m_currentScale = smartBooster.value() / 1000.0f; // mA -> A
      reportCurrent(); // invalidate current, scale changed
      break;

    default:
      break;
  }
}
