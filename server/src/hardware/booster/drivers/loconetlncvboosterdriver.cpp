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

#include "loconetlncvboosterdriver.hpp"
#include "../../interface/loconetinterface.hpp"
#include "../../../core/attributes.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../core/objectproperty.tpp"
#include "../../../log/log.hpp"
#include "../../../utils/displayname.hpp"
#include "../../../utils/unit.hpp"

namespace
{
  std::chrono::milliseconds startDelay(uint16_t address, uint16_t period)
  {
    constexpr uint32_t prime = 2654435761u; // Knuth
    return std::chrono::milliseconds((address * prime) % (1000 * period));
  }
}

LocoNetLNCVBoosterDriver::LocoNetLNCVBoosterDriver(Booster& booster, uint16_t moduleId)
  : LocoNetBoosterDriver(booster)
  , address{this, "address", addressDefault, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript}
  , pollInterval{this, "poll_interval", pollIntervalDefault, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript}
  , m_moduleId{moduleId}
  , m_pollTimer{EventLoop::ioContext()}
{
  Attributes::addDisplayName(address, DisplayName::Hardware::address);
  Attributes::addEnabled(address, false);
  Attributes::addMinMax(address, addressMin, addressMax);
  m_interfaceItems.add(address);

  Attributes::addDisplayName(pollInterval, "booster_driver:poll_interval");
  Attributes::addEnabled(pollInterval, false);
  Attributes::addMinMax(pollInterval, pollIntervalMin, pollIntervalMax);
  Attributes::addUnit(pollInterval, Unit::seconds);
  m_interfaceItems.add(pollInterval);

  LocoNetBoosterDriver::updateEnabled();
}

void LocoNetLNCVBoosterDriver::destroying()
{
  LocoNetBoosterDriver::destroying();
  m_pollTimer.cancel();
}

void LocoNetLNCVBoosterDriver::interfaceOnlineChanged(bool value)
{
  LocoNetBoosterDriver::interfaceOnlineChanged(value);

  if(value) // online
  {
    m_softwareVersion.retries = 0;
    m_temperature.retries = 0;
    m_load.retries = 0;

    // start poll timer with an address-based phase offset to spread bus load:
    m_pollTimer.expires_after(startDelay(address, pollInterval));
    m_pollTimer.async_wait(std::bind_front(&LocoNetLNCVBoosterDriver::poll, weak_ptr<LocoNetLNCVBoosterDriver>()));
  }
  else // offline
  {
    m_pollTimer.cancel();
  }
}

void LocoNetLNCVBoosterDriver::updateEnabled(bool editable, bool online)
{
  LocoNetBoosterDriver::updateEnabled(editable, online);
  Attributes::setEnabled(address, editable && !online);
  Attributes::setEnabled(pollInterval, editable);
}

void LocoNetLNCVBoosterDriver::poll(const std::weak_ptr<LocoNetLNCVBoosterDriver>& weak, std::error_code ec)
{
  if(ec)
  {
    return;
  }

  if(auto self = weak.lock())
  {
    if(self->m_softwareVersion.pollEnabled())
    {
      self->interface->readLNCV(self->m_moduleId, self->address, self->m_softwareVersion.lncv, std::bind_front(&LocoNetLNCVBoosterDriver::softwareVersionResponse, weak));
    }
    if(self->m_temperature.pollEnabled())
    {
      self->interface->readLNCV(self->m_moduleId, self->address, self->m_temperature.lncv, std::bind_front(&LocoNetLNCVBoosterDriver::temperatureResponse, weak));
    }
    if(self->m_load.pollEnabled())
    {
      self->interface->readLNCV(self->m_moduleId, self->address, self->m_load.lncv, std::bind_front(&LocoNetLNCVBoosterDriver::loadResponse, weak));
    }

    if(self->m_temperature.pollEnabled() || self->m_load.pollEnabled())
    {
      self->m_pollTimer.expires_at(self->m_pollTimer.expiry() + std::chrono::seconds(self->pollInterval.value()));
      self->m_pollTimer.async_wait(std::bind_front(&LocoNetLNCVBoosterDriver::poll, weak));
    }
  }
}

void LocoNetLNCVBoosterDriver::softwareVersionResponse(const std::weak_ptr<LocoNetLNCVBoosterDriver>& weak, uint16_t value, std::error_code ec)
{
  if(auto self = weak.lock())
  {
    if(!ec)
    {
      Log::log(self->parent(), LogMessage::I2006_BOOSTER_X_SOFTWARE_VERSION_X, self->boosterName(), value);
    }
    else
    {
      Log::log(self->parent(), LogMessage::E2025_READING_BOOSTER_X_SOFTWARE_VERSION_FAILED_X, self->boosterName(), ec.message());
    }
    self->m_softwareVersion.retries = retryLimit; // stop reading, try just once
  }
}

void LocoNetLNCVBoosterDriver::temperatureResponse(const std::weak_ptr<LocoNetLNCVBoosterDriver>& weak, uint16_t value, std::error_code ec)
{
  if(auto self = weak.lock())
  {
    if(!ec)
    {
      self->m_temperature.retries = 0;
      self->reportTemperature(value);
    }
    else
    {
      self->m_temperature.retries++;
      self->reportTemperature(); // invalidate value

      Log::log(
        self->parent(),
        self->m_temperature.pollEnabled()
          ? LogMessage::W2026_READING_BOOSTER_X_TEMPERATURE_FAILED_X  // keep trying -> warning
          : LogMessage::E2026_READING_BOOSTER_X_TEMPERATURE_FAILED_X, // give up -> error
        self->boosterName(),
        ec.message());
    }
  }
}

void LocoNetLNCVBoosterDriver::loadResponse(const std::weak_ptr<LocoNetLNCVBoosterDriver>& weak, uint16_t value, std::error_code ec)
{
  if(auto self = weak.lock())
  {
    if(!ec)
    {
      self->m_load.retries = 0;
      self->reportLoad(value);
    }
    else
    {
      self->m_load.retries++;
      self->reportLoad(); // invalidate value

      Log::log(
        self->parent(),
        self->m_load.pollEnabled()
          ? LogMessage::W2027_READING_BOOSTER_X_LOAD_FAILED_X  // keep trying -> warning
          : LogMessage::E2027_READING_BOOSTER_X_LOAD_FAILED_X, // give up -> error
        self->boosterName(),
        ec.message());
    }
  }
}
