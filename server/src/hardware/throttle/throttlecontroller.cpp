/**
 * server/src/hardware/throttle/throttlecontroller.cpp
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

#include "throttlecontroller.hpp"
#include "throttle.hpp"
#include "list/throttlelist.hpp"
#include "list/throttlelisttablemodel.hpp"
#include "../../core/attributes.hpp"
#include "../../core/objectproperty.tpp"
#include "../../utils/displayname.hpp"

ThrottleController::ThrottleController(IdObject& interface, ThrottleListColumn columns)
  : throttles{&interface, "throttles", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
{
  throttles.setValueInternal(std::make_shared<ThrottleList>(interface, throttles.name(), columns));

  Attributes::addDisplayName(throttles, DisplayName::Hardware::throttles);
}

ThrottleController::~ThrottleController() = default;

bool ThrottleController::addThrottle(Throttle& throttle)
{
  throttles->addObject(throttle.shared_ptr<Throttle>());
  return true;
}

bool ThrottleController::removeThrottle(Throttle& throttle)
{
  throttles->removeObject(throttle.shared_ptr<Throttle>());
  return true;
}

IdObject& ThrottleController::interface()
{
  auto* object = dynamic_cast<IdObject*>(this);
  assert(object);
  return *object;
}
