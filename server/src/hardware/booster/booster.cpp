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

#include "booster.hpp"
#include "drivers/boosterdriver.hpp"
#include "drivers/boosterdrivers.hpp"
#include "list/boosterlist.hpp"
#include "list/boosterlisttablemodel.hpp"
#include "../../core/attributes.hpp"
#include "../../core/objectproperty.tpp"
#include "../../utils/category.hpp"
#include "../../utils/contains.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/unit.hpp"
#include "../../world/world.hpp"

CREATE_IMPL(Booster)

Booster::Booster(World& world, std::string_view _id)
  : IdObject(world, _id)
  , name{this, "name", std::string(_id), PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly}
  , type{this, "type", "", PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly, nullptr,
      [this](std::string& value) -> bool
      {
        if(!contains<std::string_view>(BoosterDrivers::types(), value))
        {
          return false;
        }
        if(auto drv = BoosterDrivers::create(value, *this))
        {
          setDriver(std::move(drv));
          return true;
        }
        return false;
      }}
  , driver{this, "driver", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
  , load_{this, "load", noValue, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , temperature{this, "temperature", noValue, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , current{this, "current", noValue, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , voltage{this, "voltage", noValue, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , inputVoltage{this, "inputVoltage", noValue, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
{
  const bool editable = contains(m_world.state, WorldState::Edit);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);

  Attributes::addCustom(type, false);
  Attributes::addEnabled(type, editable);
  Attributes::addValues(type, BoosterDrivers::types());
  Attributes::addAliases(type, BoosterDrivers::types(), BoosterDrivers::names());
  m_interfaceItems.add(type);

  Attributes::addCategory(load_, Category::status);
  Attributes::addUnit(load_, Unit::percent);
  Attributes::addVisible(load_, false);
  m_interfaceItems.add(load_);

  Attributes::addCategory(temperature, Category::status);
  Attributes::addUnit(temperature, Unit::degreeCelcius);
  Attributes::addVisible(temperature, false);
  m_interfaceItems.add(temperature);

  Attributes::addCategory(current, Category::status);
  Attributes::addUnit(current, Unit::ampere);
  Attributes::addVisible(current, false);
  m_interfaceItems.add(current);

  Attributes::addCategory(voltage, Category::status);
  Attributes::addUnit(voltage, Unit::volt);
  Attributes::addVisible(voltage, false);
  m_interfaceItems.add(voltage);

  Attributes::addCategory(inputVoltage, Category::status);
  Attributes::addUnit(inputVoltage, Unit::volt);
  Attributes::addVisible(inputVoltage, false);
  m_interfaceItems.add(inputVoltage);

  Attributes::addCategory(driver, Category::driver);
  Attributes::addDisplayName(driver, {});
  m_interfaceItems.add(driver);

  setDriver(BoosterDrivers::create(BoosterDrivers::types().front(), *this));
}

void Booster::addToWorld()
{
  IdObject::addToWorld();
  m_world.boosters->addObject(shared_ptr<Booster>());
}

void Booster::destroying()
{
  driver->destroy();
  driver.setValueInternal(nullptr);
  m_world.boosters->removeObject(shared_ptr<Booster>());
  IdObject::destroying();
}

void Booster::load(WorldLoader& loader, const nlohmann::json& data)
{
  if(auto drv = BoosterDrivers::create(data.value<std::string_view>("type", {}), *this)) [[likely]]
  {
    setDriver(std::move(drv));
    IdObject::load(loader, data);
  }
  else // unknown driver (this can only happen with corrupt/incorrect files)
  {
    auto dataCopy = data;
    dataCopy.erase("driver"); // clear driver data, so defaults are used
    IdObject::load(loader, dataCopy);
  }
}

void Booster::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);
  switch(event)
  {
    case WorldEvent::EditEnabled:
    case WorldEvent::EditDisabled:
      Attributes::setEnabled({name, type}, contains(state, WorldState::Edit));
      break;

    default:
      break;
  };
  driver->worldEvent(state, event);
}

void Booster::setDriver(std::shared_ptr<BoosterDriver> drv)
{
  assert(drv);

  if(driver)
  {
    driver->destroy();
  }

  // Reset all status values:
  load_.setValueInternal(noValue);
  temperature.setValueInternal(noValue);
  current.setValueInternal(noValue);
  voltage.setValueInternal(noValue);
  inputVoltage.setValueInternal(noValue);

  // Show/hide supported values:
  {
    using enum BoosterDriver::SupportedStatusValues;
    const auto mask = drv->supportedStatusValues();
    Attributes::setVisible(load_, contains(mask, Load));
    Attributes::setVisible(temperature, contains(mask, Temperature));
    Attributes::setVisible(current, contains(mask, Current));
    Attributes::setVisible(voltage, contains(mask, Voltage));
    Attributes::setVisible(inputVoltage, contains(mask, InputVoltage));
  }

  Attributes::setDisplayName(driver, drv->getName());
  driver.setValueInternal(std::move(drv));
}
