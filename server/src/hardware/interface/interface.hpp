/**
 * server/src/hardware/interface/interface.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_INTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_INTERFACE_HPP

#include "../../core/idobject.hpp"
#include "../../core/objectproperty.hpp"
#include "../../status/interfacestatus.hpp"

/**
 * @brief Base class for a hardware interface
 */
class Interface : public IdObject
{
  protected:
    Interface(World& world, std::string_view _id);

    void addToWorld() override;
    void destroying() override;
    void worldEvent(WorldState state, WorldEvent event) override;

    virtual bool setOnline(bool& value, bool simulation) = 0;
    void setState(InterfaceState value);

  public:
    Property<std::string> name;
    Property<bool> online;
    ObjectProperty<InterfaceStatus> status;
    Property<std::string> notes;
};

#endif
