/**
 * server/src/hardware/input/loconetinput.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020 Reinder Feenstra
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

#ifndef SERVER_HARDWARE_INPUT_LOCONETINPUT_HPP
#define SERVER_HARDWARE_INPUT_LOCONETINPUT_HPP

#include "input.hpp"
#include "../../core/objectproperty.hpp"
#include "../protocol/loconet.hpp"

class LocoNetInput : public Input
{
  friend class Protocol::LocoNet;

  protected:
    void modeChanged(TraintasticMode mode) final;

    inline void valueChanged(bool _value) { Input::valueChanged(_value); }

  public:
    CLASS_ID("input.loconet")
    CREATE(LocoNetInput)

    ObjectProperty<Protocol::LocoNet> loconet;
    Property<uint16_t> address;

    LocoNetInput(const std::weak_ptr<World> world, const std::string& _id);
};

#endif
