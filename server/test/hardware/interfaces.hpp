/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2022-2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_TEST_HARDWARE_INTERFACES_HPP
#define TRAINTASTIC_SERVER_TEST_HARDWARE_INTERFACES_HPP

#include "../src/hardware/interface/dccex/dccexinterface.hpp"
#include "../src/hardware/interface/ecos/ecosinterface.hpp"
#include "../src/hardware/interface/hsi88.hpp"
#include "../src/hardware/interface/loconetinterface.hpp"
#include "../src/hardware/interface/marklincan/marklincaninterface.hpp"
#include "../src/hardware/interface/traintasticdiyinterface.hpp"
#include "../src/hardware/interface/withrottleinterface.hpp"
#include "../src/hardware/interface/wlanmausinterface.hpp"
#include "../src/hardware/interface/xpressnetinterface.hpp"
#include "../src/hardware/interface/z21interface.hpp"

#define INTERFACES \
  DCCEXInterface, \
  ECoSInterface, \
  HSI88Interface, \
  LocoNetInterface, \
  MarklinCANInterface, \
  TraintasticDIYInterface, \
  WiThrottleInterface, \
  WlanMausInterface, \
  XpressNetInterface, \
  Z21Interface

#define INTERFACES_DECODER \
  DCCEXInterface, \
  ECoSInterface, \
  LocoNetInterface, \
  MarklinCANInterface, \
  XpressNetInterface, \
  Z21Interface

#define INTERFACES_INPUT \
  DCCEXInterface, \
  ECoSInterface, \
  HSI88Interface, \
  LocoNetInterface, \
  TraintasticDIYInterface, \
  XpressNetInterface, \
  Z21Interface

#define INTERFACES_OUTPUT \
  DCCEXInterface, \
  ECoSInterface, \
  LocoNetInterface, \
  TraintasticDIYInterface, \
  XpressNetInterface, \
  Z21Interface

#define INTERFACES_IDENTIFICATION \
  LocoNetInterface

#endif
