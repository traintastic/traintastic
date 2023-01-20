/**
 * server/src/hardware/interface/marklincsinterface.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_MARKLINCSINTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_MARKLINCSINTERFACE_HPP

#include "interface.hpp"

/**
 * @brief Märklin CS2/CS3 hardware interface
 */
class MarklinCSInterface final
  : public Interface
{
  CLASS_ID("interface.marklin_cs")
  CREATE(MarklinCSInterface)
  DEFAULT_ID("marklin_cs")

  private:
    void addToWorld() final;
    void destroying() final;
    void worldEvent(WorldState state, WorldEvent event) final;

  protected:
    bool setOnline(bool& value, bool simulation) final;

  public:
    Property<std::string> hostname;

    MarklinCSInterface(World& world, std::string_view _id);
};

#endif
