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

#include "trackdriver.hpp"
#include "trackdrivercontroller.hpp"
#include "../../core/attributes.hpp"
#include "../../core/objectproperty.tpp"
#include "../../enum/tristate.hpp"

TrackDriver::TrackDriver(std::shared_ptr<TrackDriverController> controller, uint32_t address_)
  : interface{this, "interface", std::move(controller), PropertyFlags::Constant | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , address{this, "address", address_, PropertyFlags::Constant | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , shortCircuit{this, "short_circuit", TriState::Undefined, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
{
  m_interfaceItems.add(interface);

  m_interfaceItems.add(address);

  Attributes::addValues(shortCircuit, TriStateValues);
  m_interfaceItems.add(shortCircuit);
}

void TrackDriver::trainAdded(Object& /*source*/, bool invertPolarity, const Train& train, BlockTrainDirection direction)
{
  assert(interface);
  if(++m_useCount == 1)
  {
    assert(m_useTrain == 0);
    m_useTrain = reinterpret_cast<uintptr_t>(&train);
    interface->trackDriverTrainAdded(address, invertPolarity, train, direction);
  }
}

void TrackDriver::trainFlipped(Object& /*source*/, const Train& train, BlockTrainDirection direction)
{
  assert(interface);
  interface->trackDriverTrainFlipped(address, train, direction);
}

void TrackDriver::trainRemoved(Object& /*source*/, const Train& train)
{
  assert(interface);
  assert(m_useTrain == reinterpret_cast<uintptr_t>(&train));
  assert(m_useCount > 0);
  if(--m_useCount == 0)
  {
    m_useTrain = 0;
    interface->trackDriverTrainRemoved(address, train);
  }
}
