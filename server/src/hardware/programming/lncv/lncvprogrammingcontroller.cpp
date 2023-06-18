/**
 * server/src/hardware/programming/lncv/lncvprogrammingcontroller.cpp
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

#include "lncvprogrammingcontroller.hpp"
#include "lncvprogrammer.hpp"
#include "../../../core/controllerlist.hpp"
#include "../../../core/idobject.hpp"
#include "../../../core/objectproperty.tpp"
#include "../../../world/world.hpp"

bool LNCVProgrammingController::attachLNCVProgrammer(LNCVProgrammer& programmer)
{
  if(m_programmer)
    return false;

  m_programmer = &programmer;
  return true;
}

void LNCVProgrammingController::detachLNCVProgrammer(LNCVProgrammer& programmer)
{
  if(m_programmer == &programmer)
  {
    [[maybe_unused]] const bool r = stopLNCVProgramming();
    m_programmer = nullptr;
  }
}

void LNCVProgrammingController::addToWorld()
{
  if(auto* object = dynamic_cast<IdObject*>(this))
    object->world().lncvProgrammingControllers->add(std::dynamic_pointer_cast<LNCVProgrammingController>(object->shared_from_this()));
  else
    assert(false);
}

void LNCVProgrammingController::destroying()
{
  if(auto* object = dynamic_cast<IdObject*>(this))
    object->world().lncvProgrammingControllers->remove(std::dynamic_pointer_cast<LNCVProgrammingController>(object->shared_from_this()));
  else
    assert(false);

  if(m_programmer)
    m_programmer->destroy();
}
