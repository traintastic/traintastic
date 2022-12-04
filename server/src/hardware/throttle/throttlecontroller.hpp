/**
 * server/src/hardware/throttle/throttlecontroller.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_THROTTLE_THROTTLECONTROLLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_THROTTLE_THROTTLECONTROLLER_HPP

#include "../../core/objectproperty.hpp"

#ifdef interface
  #undef interface // interface is defined in combaseapi.h
#endif

class IdObject;
class Throttle;
class ThrottleList;
enum class ThrottleListColumn;

class ThrottleController
{
  private:
    IdObject& interface();

  protected:
    ThrottleController(IdObject& interface, ThrottleListColumn columns);

  public:
    ObjectProperty<ThrottleList> throttles;

    virtual ~ThrottleController();

    /**
     *
     * \return \c true if added, \c false otherwise.
     */
    [[nodiscard]] bool addThrottle(Throttle& throttle);

    /**
     *
     * \return \c true if removed, \c false otherwise.
     */
    [[nodiscard]] bool removeThrottle(Throttle& throttle);
};

#endif
