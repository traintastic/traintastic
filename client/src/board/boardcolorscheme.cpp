/**
 * client/src/board/boardcolorscheme.cpp
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

#include "boardcolorscheme.hpp"

const BoardColorScheme BoardColorScheme::dark = {
  /*.background =*/ {0x10, 0x10, 0x10},
  /*.track =*/ {0xC0, 0xC0, 0xC0},
  /*.trackDisabled =*/ {0x40, 0x40, 0x40},
  /*.blockFree =*/ {0x66, 0xC6, 0x66},
  /*.blockOccupied =*/ {0xC6, 0x66, 0x66},
  /*.blockUnknown =*/ {0x66, 0x66, 0x66},
  /*.sensorFree =*/ {0x66, 0xC6, 0x66},
  /*.sensorOccupied =*/ {0xC6, 0x66, 0x66},
  /*.sensorIdle =*/ {0x40, 0x40, 0x40},
  /*.sensorTriggered =*/ {0x00, 0xBF, 0xFF},
  /*.sensorUnknown =*/ {0x10, 0x10, 0x10},
  /*.turnoutState =*/ {Qt::blue},
};

const BoardColorScheme BoardColorScheme::light = {
  /*.background =*/ {0xF5, 0xF5, 0xF5},
  /*.track =*/ {0x00, 0x00, 0x00},
  /*.trackDisabled =*/ {0xA0, 0xA0, 0xA0},
  /*.blockFree =*/ {0x44, 0xC6, 0x44},
  /*.blockOccupied =*/ {0xC6, 0x44, 0x44},
  /*.blockUnknown =*/ {0x88, 0x88, 0x88},
  /*.sensorFree =*/ {0x44, 0xC6, 0x44},
  /*.sensorOccupied =*/ {0xC6, 0x44, 0x44},
  /*.sensorIdle =*/ {0x40, 0x40, 0x40},
  /*.sensorTriggered =*/ {0x00, 0xBF, 0xFF},
  /*.sensorUnknown =*/ {0x10, 0x10, 0x10},
  /*.turnoutState =*/ {Qt::cyan},
};