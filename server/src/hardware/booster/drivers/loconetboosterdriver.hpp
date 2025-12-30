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

#ifndef TRAINTASTIC_SERVER_HARDWARE_BOOSTER_DRIVERS_LOCONETBOOSTERDRIVER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_BOOSTER_DRIVERS_LOCONETBOOSTERDRIVER_HPP

#include "boosterdriver.hpp"
#include "../../../core/objectproperty.hpp"

class LocoNetInterface;

class LocoNetBoosterDriver : public BoosterDriver
{
public:
  ObjectProperty<LocoNetInterface> interface;

  void worldEvent(WorldState state, WorldEvent event) override;

protected:
  LocoNetBoosterDriver(Booster& booster);

  void destroying() override;
  void loaded() override;

  virtual void interfaceOnlineChanged(bool value);

  void updateEnabled();
  virtual void updateEnabled(bool editable, bool online);

private:
  boost::signals2::connection m_interfacePropertyChanged;

  void interfacePropertyChanged(BaseProperty& property);
};

#endif
