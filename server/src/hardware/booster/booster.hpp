/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_BOOSTER_BOOSTER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_BOOSTER_BOOSTER_HPP

#include "../../core/idobject.hpp"
#include "../../core/property.hpp"
#include "../../core/objectproperty.hpp"

class BoosterDriver;

class Booster : public IdObject
{
  CLASS_ID("booster")
  DEFAULT_ID("booster")
  CREATE_DEF(Booster)

public:
  static constexpr auto noValue = std::numeric_limits<float>::quiet_NaN();

  Property<std::string> name;
  Property<std::string> type;
  ObjectProperty<BoosterDriver> driver;
  Property<float> load_; // load is already a function
  Property<float> temperature;
  Property<float> current;
  Property<float> voltage;
  Property<float> inputVoltage;

  Booster(World& world, std::string_view _id);

protected:
  void addToWorld() override;
  void destroying() override;
  void load(WorldLoader& loader, const nlohmann::json& data) override;
  void worldEvent(WorldState state, WorldEvent event) override;

private:
  void setDriver(std::shared_ptr<BoosterDriver> drv);
};

#endif
