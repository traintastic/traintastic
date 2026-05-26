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

#ifndef TRAINTASTIC_SERVER_HARDWARE_BOOSTER_DRIVERS_CBUSBOOSTERDRIVER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_BOOSTER_DRIVERS_CBUSBOOSTERDRIVER_HPP

#include "boosterdriver.hpp"
#include "../../../core/objectproperty.hpp"
#include "../../../core/property.hpp"

namespace CBUS {
  struct Message;
}

class CBUSInterface;

class CBUSBoosterDriver : public BoosterDriver
{
  CLASS_ID("booster_driver.cbus");
  BOOSTER_DRIVER_CREATE(CBUSBoosterDriver);
  BOOSTER_DRIVER_NAME("CBUS/VLCB");

public:
  ObjectProperty<CBUSInterface> interface;
  Property<uint16_t> node;
  Property<uint16_t> currentEvent;
  Property<uint16_t> voltageEvent;

  CBUSBoosterDriver(Booster& booster);

  void worldEvent(WorldState state, WorldEvent event) override;

  SupportedStatusValues supportedStatusValues() const final
  {
    using enum SupportedStatusValues;
    return (Current | Voltage);
  }

protected:
  void destroying() override;
  void loaded() override;

  void interfaceOnlineChanged(bool online);

  void updateEnabled();

private:
  boost::signals2::connection m_interfacePropertyChanged;
  size_t m_onReceiveHandle = 0;

  void interfacePropertyChanged(BaseProperty& property);

  void enableEvents();
  void disableEvents();

  void receive(uint8_t canId, const CBUS::Message& message);
};

#endif
