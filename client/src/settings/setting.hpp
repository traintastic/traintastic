/**
 * client/src/settings/setting.hpp
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

#ifndef TRAINTASTIC_CLIENT_SETTINGS_SETTING_HPP
#define TRAINTASTIC_CLIENT_SETTINGS_SETTING_HPP

#include "settingsbase.hpp"

template<class T>
class Setting
{
  private:
    SettingsBase& m_settings;
    const QString m_name;
    const T m_defaultValue;

  public:
    Setting(SettingsBase& settings, QString name, T defaultValue)
      : m_settings{settings}
      , m_name{std::move(name)}
      , m_defaultValue{std::move(defaultValue)}
    {
    }

    Setting(const Setting<T>&) = delete;

    const QString& name() const
    {
      return m_name;
    }

    T defaultValue() const
    {
      return m_defaultValue;
    }

    T value() const
    {
      return m_settings.get(m_name, m_defaultValue);
    }

    void setValue(const T& value)
    {
      m_settings.set(m_name, value);
    }

    operator T() const
    {
      return value();
    }

    Setting<T>& operator =(const T& value)
    {
      setValue(value);
      return *this;
    }
};

#endif
