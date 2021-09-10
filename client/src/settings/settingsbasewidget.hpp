/**
 * client/src/settings/settingsbasewidget.hpp
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

#ifndef TRAINTASTIC_CLIENT_SETTINGS_SETTINGSBASEWIDGET_HPP
#define TRAINTASTIC_CLIENT_SETTINGS_SETTINGSBASEWIDGET_HPP

#include <QScrollArea>
#include "setting.hpp"

class SettingsBaseWidget : public QScrollArea
{
  private:
    const QString m_trPrefix;

  protected:
    SettingsBaseWidget(QString trPrefix, QWidget* parent = nullptr);

    void add(const QString& settingName, QWidget* widget);
    void add(const QString& settingName, QLayout* layout);

    void addSettingOnOff(Setting<bool>& setting);
    void addSettingDir(Setting<QString>& setting);

    template<class T>
    inline void addSetting(Setting<T>& setting)
    {
      if constexpr(std::is_same_v<T, bool>)
        addSettingOnOff(setting);
      else
        static_assert(sizeof(T) != sizeof(T));
    }

    void done();

  public:
    virtual QString title() const = 0;
};

#endif
