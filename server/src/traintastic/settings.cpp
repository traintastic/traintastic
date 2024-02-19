/**
 * server/src/traintastic/settings.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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
#include <iomanip>
#include "../core/attributes.hpp"
#include "traintastic.hpp"
#include "../network/server.hpp"
#include "../log/log.hpp"
#include "../os/localtime.hpp"
#include "../utils/category.hpp"

using nlohmann::json;

Settings::PreStart Settings::getPreStartSettings(const std::filesystem::path& path)
{
  std::ifstream file(path / filename);
  if(file.is_open())
  {
    json settings = json::parse(file);
    PreStart preStart;
    preStart.memoryLoggerSize = settings.value(Name::memoryLoggerSize, Default::memoryLoggerSize);
    preStart.enableFileLogger = settings.value(Name::enableFileLogger, Default::enableFileLogger);
    return preStart;
  }
  return PreStart();
}

Settings::Settings(const std::filesystem::path& path)
  : m_filename{path / filename}
#ifndef NO_LOCALHOST_ONLY_SETTING
  , localhostOnly{this, "localhost_only", true, PropertyFlags::ReadWrite, [this](const bool& /*value*/){ saveToFile(); }}
#endif
  , port{this, "port", Server::defaultPort, PropertyFlags::ReadWrite, [this](const uint16_t& /*value*/){ saveToFile(); }}
  , discoverable{this, "discoverable", true, PropertyFlags::ReadWrite, [this](const bool& /*value*/){ saveToFile(); }}
  , lastWorld{this, "last_world", "", PropertyFlags::ReadWrite | PropertyFlags::Internal, [this](const std::string& /*value*/){ saveToFile(); }}
  , loadLastWorldOnStartup{this, "load_last_world_on_startup", true, PropertyFlags::ReadWrite, [this](const bool& /*value*/){ saveToFile(); }}
  , autoSaveWorldOnExit{this, "auto_save_world_on_exit", false, PropertyFlags::ReadWrite, [this](const bool& /*value*/){ saveToFile(); }}
  , saveWorldUncompressed{this, "save_world_uncompressed", false, PropertyFlags::ReadWrite, [this](const bool& /*value*/){ saveToFile(); }}
  , allowClientServerRestart{this, "allow_client_server_restart", false, PropertyFlags::ReadWrite | PropertyFlags::Internal, [this](const bool& /*value*/){ saveToFile(); }}
  , allowClientServerShutdown{this, "allow_client_server_shutdown", false, PropertyFlags::ReadWrite | PropertyFlags::Internal, [this](const bool& /*value*/){ saveToFile(); }}
  , memoryLoggerSize{this, Name::memoryLoggerSize, Default::memoryLoggerSize, PropertyFlags::ReadWrite, [this](const uint32_t& /*value*/){ saveToFile(); }}
  , enableFileLogger{this, Name::enableFileLogger, Default::enableFileLogger, PropertyFlags::ReadWrite, [this](const bool& /*value*/){ saveToFile(); }}
{
  m_interfaceItems.add(lastWorld);
  m_interfaceItems.add(loadLastWorldOnStartup);
  m_interfaceItems.add(autoSaveWorldOnExit);

#ifndef NO_LOCALHOST_ONLY_SETTING
  Attributes::addCategory(localhostOnly, Category::network);
  m_interfaceItems.add(localhostOnly);
#endif
  Attributes::addCategory(port, Category::network);
  m_interfaceItems.add(port);
  Attributes::addCategory(discoverable, Category::network);
  m_interfaceItems.add(discoverable);
  Attributes::addCategory(allowClientServerRestart, Category::network);
  m_interfaceItems.add(allowClientServerRestart);
  Attributes::addCategory(allowClientServerShutdown, Category::network);
  m_interfaceItems.add(allowClientServerShutdown);

  Attributes::addCategory(memoryLoggerSize, Category::log);
  Attributes::addMinMax(memoryLoggerSize, 0U, memoryLoggerSizeMax);
  m_interfaceItems.add(memoryLoggerSize);
  Attributes::addCategory(enableFileLogger, Category::log);
  m_interfaceItems.add(enableFileLogger);

  Attributes::addCategory(saveWorldUncompressed, Category::developer);
  m_interfaceItems.add(saveWorldUncompressed);

  loadFromFile();
}

void Settings::loadFromFile()
{
  std::ifstream file(m_filename);
  if(file.is_open())
  {
    json settings = json::parse(file);
    for(auto& [name, value] : settings.items())
    {
      AbstractProperty* property = getProperty(name);
      if(property)
        property->loadJSON(value);
      else
        Log::log(*this, LogMessage::W1002_SETTING_X_DOESNT_EXIST, name);
    }
    Log::log(*this, LogMessage::N1008_LOADED_SETTINGS);
  }
  else
  {
    Log::log(*this, LogMessage::I1002_SETTING_FILE_NOT_FOUND_USING_DEFAULTS);
    saveToFile();
  }
}

void Settings::saveToFile()
{
  // backup settings:
  if(std::filesystem::is_regular_file(m_filename))
  {
    const std::filesystem::path backupDir = Traintastic::instance->dataBackupDir();
    auto dateTimeStr =
      []()
      {
        const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::stringstream ss;
        tm tm;
        ss << std::put_time(localTime(&now, &tm), "_%Y%m%d_%H%M%S");
        return ss.str();
      };

    if(!std::filesystem::is_directory(backupDir))
    {
      std::error_code ec;
      std::filesystem::create_directories(backupDir, ec);
      if(ec)
        Log::log(*this, LogMessage::C1008_CREATING_BACKUP_DIRECTORY_FAILED_X, ec);
    }

    {
      std::error_code ec;
      std::filesystem::rename(m_filename, backupDir / (m_filename.stem() += dateTimeStr()) += m_filename.extension(), ec);
      if(ec)
        Log::log(*this, LogMessage::C1009_CREATING_SETTING_BACKUP_FAILED_X, ec);
    }
  }

  json settings = json::object();
  for(const auto& it : m_interfaceItems)
    if(AbstractProperty* property = dynamic_cast<AbstractProperty*>(&it.second))
      settings[std::string{property->name()}] = property->toJSON();

  std::ofstream file(m_filename);
  if(file.is_open())
  {
    file << settings.dump(2);
    Log::log(*this, LogMessage::N1009_SAVED_SETTINGS);
  }
  else
    Log::log(*this, LogMessage::C1003_CANT_WRITE_TO_SETTINGS_FILE_X, m_filename);
}
