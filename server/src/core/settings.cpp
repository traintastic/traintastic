/**
 * server/src/core/settings.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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
#include <fstream>
//#include "traintastic.hpp"

using nlohmann::json;

Settings::Settings(const std::filesystem::path& filename) :
  Object{},
  m_filename{filename},
  localhostOnly{this, "localhost_only", true, PropertyFlags::ReadWrite, [this](const bool&){ save(); }},
  port{this, "port", defaultPort, PropertyFlags::ReadWrite, [this](const uint16_t&){ save(); }},
  discoverable{this, "discoverable", true, PropertyFlags::ReadWrite, [this](const bool&){ save(); }},
  defaultWorld{this, "default_world", "", PropertyFlags::ReadWrite, [this](const std::string&){ save(); }},
  autoSaveWorldOnExit{this, "auto_save_world_on_exit", false, PropertyFlags::ReadWrite, [this](const bool&){ save(); }},
  allowClientServerRestart{this, "allow_client_server_restart", false, PropertyFlags::ReadWrite, [this](const bool&){ save(); }},
  allowClientServerShutdown{this, "allow_client_server_shutdown", false, PropertyFlags::ReadWrite, [this](const bool&){ save(); }}
{
  m_interfaceItems.add(localhostOnly);
  m_interfaceItems.add(port);
  m_interfaceItems.add(discoverable);
  m_interfaceItems.add(defaultWorld);
  m_interfaceItems.add(autoSaveWorldOnExit);
  m_interfaceItems.add(allowClientServerRestart);
  m_interfaceItems.add(allowClientServerShutdown);

  load();
}

void Settings::load()
{
  std::ifstream file(m_filename);
  if(file.is_open())
  {
    logDebug("Settings file: " + m_filename.string());
    json settings = json::parse(file);
    for(auto& [name, value] : settings.items())
    {
      AbstractProperty* property = getProperty(name);
      if(property)
        property->load(value);
      else
        logWarning("Setting `" + name + "` doesn't exist");
    }
    logInfo("Loaded settings");
  }
  else
    logInfo("Settings file not found, using defaults");
}

void Settings::save()
{
  json settings = json::object();
  for(const auto& it : m_interfaceItems)
    if(AbstractProperty* property = dynamic_cast<AbstractProperty*>(&it.second))
      settings[property->name()] = property->toJSON();

  std::ofstream file(m_filename);
  if(file.is_open())
  {
    file << settings.dump(2);
    logNotice("Saved settings");
  }
  else
    logCritical("Can't write to settings file");
}
