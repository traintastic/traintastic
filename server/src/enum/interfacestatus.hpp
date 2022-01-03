/**
 * server/src/enum/interfacestatus.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_ENUM_INTERFACESTATUS_HPP
#define TRAINTASTIC_SERVER_ENUM_INTERFACESTATUS_HPP

#include <traintastic/enum/interfacestatus.hpp>

inline constexpr std::array<InterfaceStatus, 4> interfaceStatusValues{{
  InterfaceStatus::Offline,
  InterfaceStatus::Initializing,
  InterfaceStatus::Online,
  InterfaceStatus::Error,
}};

#endif
