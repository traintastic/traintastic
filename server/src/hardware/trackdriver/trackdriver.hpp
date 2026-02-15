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

#ifndef TRAINTASTIC_SERVER_HARDWARE_TRACKDRIVER_TRACKDRIVER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_TRACKDRIVER_TRACKDRIVER_HPP

#include "../../core/nonpersistentobject.hpp"
#include <set>
#include "../../core/property.hpp"
#include "../../core/objectproperty.hpp"

#ifdef interface
  #undef interface // interface is defined in combaseapi.h
#endif

class TrackDriverController;

class TrackDriver : public NonPersistentObject
{
  friend class TrackDriverController;

  CLASS_ID("track_driver")

public:
  ObjectProperty<TrackDriverController> interface;
  Property<uint32_t> address;

  TrackDriver(std::shared_ptr<TrackDriverController> controller, uint32_t address_);

private:
  std::set<std::shared_ptr<Object>> m_usedBy; //!< Objects that use the track driver.
};

#endif
