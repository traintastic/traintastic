/**
 * server/src/hardware/protocol/traintasticcs/settings.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "settings.hpp"
#include "messages.hpp"
#include "../../../core/attributes.hpp"
#include "../../../utils/displayname.hpp"

namespace TraintasticCS {

Settings::Settings(Object& _parent, std::string_view parentPropertyName)
  : SubObject(_parent, parentPropertyName)
  , xpressnetEnabled{this, "xpressnet_enabled", true, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , debugLogRXTX{this, "debug_log_rx_tx", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , debugLogPing{this, "debug_log_ping", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  //Attributes::addGroup(xpressnetEnabled, Group::xpressnet);
  m_interfaceItems.add(xpressnetEnabled);

  Attributes::addDisplayName(debugLogRXTX, DisplayName::Hardware::debugLogRXTX);
  //Attributes::addGroup(debugLogRXTX, Group::debug);
  m_interfaceItems.add(debugLogRXTX);

  //Attributes::addGroup(debugLogPing, Group::debug);
  m_interfaceItems.add(debugLogPing);
}

Config Settings::config() const
{
  Config config;

  config.xpressnet.enabled = xpressnetEnabled;

  config.debugLogRXTX = debugLogRXTX;
  config.debugLogPing = debugLogPing;

  return config;
}

}
