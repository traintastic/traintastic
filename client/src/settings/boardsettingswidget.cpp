/**
 * client/src/settings/boardsettingswidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2025 Reinder Feenstra
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

#include "boardsettingswidget.hpp"
#include "boardsettings.hpp"

BoardSettingsWidget::BoardSettingsWidget(QWidget* parent)
  : SettingsBaseWidget("qtapp.settings.board", parent)
{
  BoardSettings& s = BoardSettings::instance();

  addSetting(s.colorScheme);
  addSetting(s.gridOperateMode);
  addSetting(s.gridEditMode);
  addSetting(s.turnoutDrawState);
  addSetting(s.showBlockSensorStates);

  done();
}
