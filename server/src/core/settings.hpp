/**
 * server/src/core/settings.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CORE_SETTINGS_HPP
#define TRAINTASTIC_SERVER_CORE_SETTINGS_HPP

#include "object.hpp"
#include <traintastic/utils/stdfilesystem.hpp>
#include "property.hpp"

class Settings : public Object
{
  private:
    static constexpr std::string_view filename = "settings.json";

    struct Name
    {
      static constexpr const char* memoryLoggerSize = "memory_logger_size";
      static constexpr const char* enableFileLogger = "enable_file_logger";
    };

    struct Default
    {
      static constexpr uint32_t memoryLoggerSize = 1000;
      static constexpr bool enableFileLogger = false;
    };

    const std::filesystem::path m_filename;

    void load();
    void save();

  public:
    CLASS_ID("settings")

    struct PreStart
    {
      uint32_t memoryLoggerSize = Default::memoryLoggerSize;
      bool enableFileLogger = Default::enableFileLogger;
    };

    static constexpr std::string_view id = classId;
    static constexpr uint16_t defaultPort = 5740; //!< unoffical, not (yet) assigned by IANA

    static PreStart getPreStartSettings(const std::filesystem::path& path);

    Property<bool> localhostOnly;
    Property<uint16_t> port;
    Property<bool> discoverable;
    Property<std::string> defaultWorld;
    Property<bool> autoSaveWorldOnExit;
    Property<bool> allowClientServerRestart;
    Property<bool> allowClientServerShutdown;
    Property<uint32_t> memoryLoggerSize;
    Property<bool> enableFileLogger;

    Settings(const std::filesystem::path& path);

    std::string getObjectId() const final { return std::string(id); }
};

#endif
