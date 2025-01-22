/**
 * client/src/settings/generalsettings.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023-2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_SETTINGS_GENERALSETTINGS_HPP
#define TRAINTASTIC_CLIENT_SETTINGS_GENERALSETTINGS_HPP

#include "settingsbase.hpp"
#include "setting.hpp"

class GeneralSettings  : public SettingsBase
{
  private:
    GeneralSettings()
      : SettingsBase("general_") // "general" is special in Qt!!
      , showIntroductionWizard{*this, "show_introduction_wizard", true}
      , language{*this, "language", "en-us"}
      , connectAutomaticallyToDiscoveredServer{*this, "connect_automatically_to_discovered_server", true}
    {
    }

  public:
    static GeneralSettings& instance()
    {
      static GeneralSettings settings;
      return settings;
    }

    Setting<bool> showIntroductionWizard;
    Setting<QString> language;
    Setting<bool> connectAutomaticallyToDiscoveredServer;
};

#endif
