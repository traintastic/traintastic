/**
 * server/test/lua/enums.hpp
 *
 * This file is part of the traintastic test suite.
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

#ifndef TRAINTASTIC_SERVER_TEST_LUA_ENUMS_HPP
#define TRAINTASTIC_SERVER_TEST_LUA_ENUMS_HPP

#include "../../src/enum/decoderprotocol.hpp"
#include "../../src/enum/direction.hpp"
#include "../../src/enum/directioncontrolstate.hpp"
#include "../../src/enum/worldevent.hpp"
#include "../../src/enum/worldscale.hpp"

#define TEST_ENUMS \
  DecoderProtocol, \
  Direction, \
  DirectionControlState, \
  WorldEvent, \
  WorldScale

#endif

