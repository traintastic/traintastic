/**
 * client/src/settings/boardsettings.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_SETTINGS_BOARDSETTINGS_HPP
#define TRAINTASTIC_CLIENT_SETTINGS_BOARDSETTINGS_HPP

#include <array>
#include "settingsbase.hpp"
#include "setting.hpp"
#include <traintastic/enum/enum.hpp>

class BoardSettings : public SettingsBase
{
  public:
    enum class ColorScheme
    {
      Dark = 0,
      Light = 1,
    };

  private:
    BoardSettings()
      : SettingsBase("board")
      , showIntroductionWizard{*this, "show_introduction_wizard", true}
      , colorScheme{*this, "color_scheme", ColorScheme::Dark}
      , turnoutDrawState{*this, "turnout_draw_state", true}
      , showBlockSensorStates{*this, "show_block_sensor_states", true}
    {
    }

  public:
    static BoardSettings& instance()
    {
      static BoardSettings settings;
      return settings;
    }

    Setting<bool> showIntroductionWizard;
    Setting<ColorScheme> colorScheme;
    Setting<bool> turnoutDrawState;
    Setting<bool> showBlockSensorStates;
};

TRAINTASTIC_ENUM(BoardSettings::ColorScheme, "board_settings.color_scheme", 2,
{
  {BoardSettings::ColorScheme::Dark, "dark"},
  {BoardSettings::ColorScheme::Light, "light"},
});

template<>
struct SettingEnum<BoardSettings::ColorScheme>
{
  static constexpr std::array<BoardSettings::ColorScheme, 2> values = {BoardSettings::ColorScheme::Dark, BoardSettings::ColorScheme::Light};
};

#endif
