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

#ifndef TRAINTASTIC_SERVER_HARDWARE_BOOSTER_DRIVERS_YD7403BOOSTERDRIVER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_BOOSTER_DRIVERS_YD7403BOOSTERDRIVER_HPP

#include "loconetboosterdriver.hpp"
#include "../../../core/property.hpp"

namespace LocoNet {
  struct Message;
}

class YD7403BoosterDriver final : public LocoNetBoosterDriver
{
  CLASS_ID("booster_driver.yd7403");
  BOOSTER_DRIVER_CREATE(YD7403BoosterDriver);
  BOOSTER_DRIVER_NAME("YaMoRC YD7403");

public:
  Property<uint8_t> address;

  YD7403BoosterDriver(Booster& booster);

  SupportedStatusValues supportedStatusValues() const final;

protected:
  void destroying() final;

  void interfaceOnlineChanged(bool value) final;
  void updateEnabled(bool editable, bool online) final;

private:
  static constexpr uint8_t addressDefault = 0;

  size_t m_onReceiveHandle = 0;
  float m_currentScale = std::numeric_limits<float>::quiet_NaN();

  void enableEvents();
  void disableEvents();

  void receive(const LocoNet::Message& message);
};

#endif

