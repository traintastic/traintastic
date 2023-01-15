/**
 * server/test/hardware/interfaces.hpp
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

#ifndef TRAINTASTIC_SERVER_TEST_HARDWARE_INTERFACES_HPP
#define TRAINTASTIC_SERVER_TEST_HARDWARE_INTERFACES_HPP

#include "../src/hardware/interface/dccplusplusinterface.hpp"
#include "../src/hardware/interface/ecosinterface.hpp"
#include "../src/hardware/interface/hsi88.hpp"
#include "../src/hardware/interface/loconetinterface.hpp"
#include "../src/hardware/interface/traintasticdiyinterface.hpp"
#include "../src/hardware/interface/withrottleinterface.hpp"
#include "../src/hardware/interface/wlanmausinterface.hpp"
#include "../src/hardware/interface/xpressnetinterface.hpp"
#include "../src/hardware/interface/z21interface.hpp"

#define INTERFACES \
  DCCPlusPlusInterface, \
  ECoSInterface, \
  HSI88Interface, \
  LocoNetInterface, \
  TraintasticDIYInterface, \
  WiThrottleInterface, \
  WlanMausInterface, \
  XpressNetInterface, \
  Z21Interface

#define INTERFACES_DECODER \
  DCCPlusPlusInterface, \
  ECoSInterface, \
  LocoNetInterface, \
  XpressNetInterface, \
  Z21Interface

#define INTERFACES_INPUT \
  DCCPlusPlusInterface, \
  ECoSInterface, \
  HSI88Interface, \
  LocoNetInterface, \
  TraintasticDIYInterface, \
  XpressNetInterface, \
  Z21Interface

#define INTERFACES_OUTPUT \
  DCCPlusPlusInterface, \
  ECoSInterface, \
  LocoNetInterface, \
  TraintasticDIYInterface, \
  XpressNetInterface, \
  Z21Interface

#define INTERFACES_IDENTIFICATION \
  LocoNetInterface

#endif
