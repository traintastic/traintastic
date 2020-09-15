/**
 * server/src/core/settings.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_SETTINGS_HPP
#define TRAINTASTIC_SERVER_CORE_SETTINGS_HPP

#include "object.hpp"
#include "stdfilesystem.hpp"
#include "property.hpp"

class Settings : public Object
{
  protected:
    static const std::string id;

    const std::filesystem::path m_filename;

    void load();
    void save();

    void logDebug(const std::string& message);
    void logInfo(const std::string& message);
    void logWarning(const std::string& message);

  public:
    CLASS_ID("settings")

    static constexpr uint16_t defaultPort = 5740; //!< unoffical, not (yet) assigned by IANA

    Property<bool> localhostOnly;
    Property<uint16_t> port;
    Property<bool> discoverable;
    Property<std::string> defaultWorld;
    Property<bool> autoSaveWorldOnExit;
    Property<bool> allowClientServerRestart;
    Property<bool> allowClientServerShutdown;

    Settings(const std::filesystem::path& filename);
};

#endif
