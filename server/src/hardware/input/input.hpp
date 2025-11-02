/**
 * server/src/hardware/input/input.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INPUT_INPUT_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INPUT_INPUT_HPP

#include "../../core/nonpersistentobject.hpp"
#include <set>
#include <traintastic/enum/inputchannel.hpp>
#include "../../core/property.hpp"
#include "../../core/objectproperty.hpp"
#include "../../core/event.hpp"
#include "../../enum/tristate.hpp"
#include "../../enum/simulateinputaction.hpp"

#ifdef interface
  #undef interface // interface is defined in combaseapi.h
#endif

class InputController;

class Input : public NonPersistentObject
{
  friend class InputController;

  CLASS_ID("input")

  private:
    std::set<std::shared_ptr<Object>> m_usedBy; //!< Objects that use the input.

  protected:
    void updateValue(TriState _value);

  public:
    static constexpr uint32_t addressMinDefault = std::numeric_limits<uint32_t>::min();
    static constexpr uint32_t addressMaxDefault = std::numeric_limits<uint32_t>::max();

    ObjectProperty<InputController> interface;
    Property<InputChannel> channel;
    Property<uint32_t> address;
    Property<TriState> value;
    Event<bool, const std::shared_ptr<Input>&> onValueChanged;

    Input(std::shared_ptr<InputController> inputController, InputChannel channel_, uint32_t address_);

    void simulateChange(SimulateInputAction action);
};

#endif
