/**
 * server/src/hardware/input/input.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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

#include "../../core/idobject.hpp"
#include "../../core/objectproperty.hpp"
#include "../../core/objectvectorproperty.hpp"
#include "../../core/event.hpp"
#include "../../enum/tristate.hpp"
#include "inputcontroller.hpp"

#ifdef interface
  #undef interface // interface is defined in combaseapi.h
#endif

class Input : public IdObject
{
  CLASS_ID("input")
  DEFAULT_ID("input")
  CREATE(Input)

  friend class InputController;

  private:
    static constexpr uint32_t addressMinDefault = 0;
    static constexpr uint32_t addressMaxDefault = 1'000'000;

    void interfaceChanged();
    void channelChanged();

  protected:
    void addToWorld() override;
    void loaded() override;
    void destroying() override;
    void worldEvent(WorldState state, WorldEvent event) override;

    void updateValue(TriState _value);

  public:
    static constexpr uint32_t invalidAddress = std::numeric_limits<uint32_t>::max();

    Property<std::string> name;
    ObjectProperty<InputController> interface;
    Property<uint32_t> channel;
    Property<uint32_t> address;
    Property<TriState> value;
    ObjectVectorProperty<Object> consumers;
    Event<bool> onValueChanged;

    Input(World& world, std::string_view _id);

    void simulateChange(SimulateInputAction action);
};

#endif
