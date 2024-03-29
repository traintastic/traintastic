/**
 * client/src/settings/developersettings.hpp
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

#ifndef TRAINTASTIC_CLIENT_SETTINGS_DEVELOPERSETTINGS_HPP
#define TRAINTASTIC_CLIENT_SETTINGS_DEVELOPERSETTINGS_HPP

#include "settingsbase.hpp"
#include <QDir>
#include "setting.hpp"
#include <traintastic/utils/stdfilesystem.hpp>

class DeveloperSettings : public SettingsBase
{
  private:
    DeveloperSettings()
      : SettingsBase("developer")
      , dontLoadFallbackLanguage{*this, "dont_load_fallback_language", false}
      , logMissingStrings{*this, "log_missing_strings", false}
      , logMissingStringsDir{*this, "log_missing_strings_dir", QDir::home().path()}
    {
    }

  public:
    static DeveloperSettings& instance()
    {
      static DeveloperSettings settings;
      return settings;
    }

    Setting<bool> dontLoadFallbackLanguage;
    Setting<bool> logMissingStrings;
    Setting<QString> logMissingStringsDir;
};

#endif
