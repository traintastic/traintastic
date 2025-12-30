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

#ifndef TRAINTASTIC_SERVER_HARDWARE_BOOSTER_DRIVERS_LOCONETLNCVBOOSTERDRIVER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_BOOSTER_DRIVERS_LOCONETLNCVBOOSTERDRIVER_HPP

#include "loconetboosterdriver.hpp"
#include <boost/asio/steady_timer.hpp>
#include "../../../core/property.hpp"

class LocoNetLNCVBoosterDriver : public LocoNetBoosterDriver
{
public:
  Property<uint16_t> address;
  Property<uint16_t> pollInterval;

protected:
  struct PollValue
  {
    uint16_t lncv;
    bool enabled = true;
    uint8_t retries = 0;

    inline bool pollEnabled() const
    {
      return enabled && (retries < retryLimit);
    }
  };

  static constexpr uint8_t retryLimit = 3;

  const uint16_t m_moduleId;
  PollValue m_softwareVersion{1};
  PollValue m_temperature{6};
  PollValue m_load{7};

  LocoNetLNCVBoosterDriver(Booster& booster, uint16_t moduleId);

  void destroying() override;

  void interfaceOnlineChanged(bool value) final;
  void updateEnabled(bool editable, bool online) final;

private:
  static constexpr uint16_t addressMin = 0;
  static constexpr uint16_t addressDefault = 1;
  static constexpr uint16_t addressMax = 65534;
  static constexpr uint16_t pollIntervalMin = 1;
  static constexpr uint16_t pollIntervalDefault = 5;
  static constexpr uint16_t pollIntervalMax = 30;

  boost::asio::steady_timer m_pollTimer;

  static void poll(const std::weak_ptr<LocoNetLNCVBoosterDriver>& weakSelf, std::error_code ec);

  static void softwareVersionResponse(const std::weak_ptr<LocoNetLNCVBoosterDriver>& weak, uint16_t value, std::error_code ec);
  static void temperatureResponse(const std::weak_ptr<LocoNetLNCVBoosterDriver>& weak, uint16_t value, std::error_code ec);
  static void loadResponse(const std::weak_ptr<LocoNetLNCVBoosterDriver>& weak, uint16_t value, std::error_code ec);
};

#endif

