/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#include "interfacesimulatorsettings.hpp"
#include "../core/attributes.hpp"
#include "../utils/displayname.hpp"

InterfaceSimulatorSettings::InterfaceSimulatorSettings(Object& parentObject, std::string_view parentPropertyName)
  : SubObject(parentObject, parentPropertyName)
  , useSimulator{this, "use_simulator", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , hostname{this, "hostname", "127.0.0.1", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , port{this, "port", 5741, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  Attributes::addEnabled(useSimulator, false);
  m_interfaceItems.add(useSimulator);

  Attributes::addDisplayName(hostname, DisplayName::IP::hostname);
  Attributes::addEnabled(hostname, false);
  m_interfaceItems.add(hostname);

  Attributes::addDisplayName(port, DisplayName::IP::port);
  Attributes::addEnabled(port, false);
  m_interfaceItems.add(port);
}

void InterfaceSimulatorSettings::updateEnabled(const bool worldEdit, const bool interfaceOnline)
{
  Attributes::setEnabled({useSimulator, hostname, port}, worldEdit && !interfaceOnline);
}
