/**
 * server/src/hardware/controller/controller.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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


#ifndef TRAINTASTIC_SERVER_HARDWARE_CONTROLLER_CONTROLLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_CONTROLLER_CONTROLLER_HPP

#include "../../core/idobject.hpp"
#include "../../core/property.hpp"
#include "../../core/objectproperty.hpp"
#include "../../core/commandstationproperty.hpp"

class Decoder;
enum class DecoderChangeFlags;

class Controller : public IdObject
{
  friend class CommandStation;

  protected:
    void addToWorld() final;
    void worldEvent(WorldState state, WorldEvent event) override;

    virtual bool setActive(bool& value) = 0;

    virtual void emergencyStopChanged(bool value) = 0;
    virtual void powerOnChanged(bool value) = 0;
    virtual void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) = 0;

  public:
    Property<std::string> name;
    CommandStationProperty commandStation;
    Property<bool> active;
    Property<std::string> notes;

    Controller(const std::weak_ptr<World>& _world, std::string_view _id);
};

#endif
