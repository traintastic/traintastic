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
#include <nlohmann/json.hpp>
#include "traintastic.hpp"

using nlohmann::json;

const std::string Settings::id{Settings::classId};

Settings::Settings(const std::filesystem::path& filename) :
  Object{},
  m_filename{filename},
  localhostOnly{this, "localhost_only", true, PropertyFlags::ReadWrite, [this](const bool&){ save(); }},
  port{this, "port", defaultPort, PropertyFlags::ReadWrite, [this](const uint16_t&){ save(); }},
  discoverable{this, "discoverable", true, PropertyFlags::ReadWrite, [this](const bool&){ save(); }},
  defaultWorld{this, "default_world", "", PropertyFlags::ReadWrite, [this](const std::string&){ save(); }},
  autoSaveWorldOnExit{this, "auto_save_world_on_exit", false, PropertyFlags::ReadWrite, [this](const bool&){ save(); }}
{
  m_interfaceItems.add(localhostOnly);
  m_interfaceItems.add(port);
  m_interfaceItems.add(discoverable);
  m_interfaceItems.add(defaultWorld);
  m_interfaceItems.add(autoSaveWorldOnExit);

  load();
}

void Settings::load()
{
  std::ifstream file(m_filename);
  if(file.is_open())
  {
    Traintastic::instance->console->debug(id, "Settings file: " + m_filename.string());
    json settings = json::parse(file);
    for(auto& [name, value] : settings.items())
    {
      AbstractProperty* property = getProperty(name);
      if(property)
        property->fromJSON(value);
      else
        Traintastic::instance->console->warning(id, "Setting `" + name + "` doesn't exist");
    }
    Traintastic::instance->console->info(id, "Loaded settings");
  }
  else
    Traintastic::instance->console->info(id, "Settings file not found, using defaults");
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
    Traintastic::instance->console->notice(id, "Saved settings");
  }
  else
    Traintastic::instance->console->critical(id, "Can't write to settings file");
}
