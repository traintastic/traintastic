/**
 * client/src/settings/settingsbase.hpp
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

#ifndef TRAINTASTIC_CLIENT_SETTINGS_SETTINGSBASE_HPP
#define TRAINTASTIC_CLIENT_SETTINGS_SETTINGSBASE_HPP

#include <QObject>
#include <QSettings>
#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
  #include <QCoreApplication>
#endif

class SettingsBase : public QObject
{
  Q_OBJECT

  template<class T>
  friend class Setting;

  private:
    QSettings m_systemSettings;
    QSettings m_settings;

    template<class T>
    inline T get(const QString& key, const T& defaultValue) const
    {
      if constexpr(std::is_enum_v<T>)
        return static_cast<T>(m_settings.value(key, static_cast<uint>(defaultValue)).toUInt());
      else
        return qvariant_cast<T>(m_settings.value(key, defaultValue));
    }

    template<class T>
    inline void set(const QString& key, const T& value)
    {
      if constexpr(std::is_enum_v<T>)
      {
        if(m_settings.value(key) != static_cast<uint>(value))
        {
          m_settings.setValue(key, static_cast<uint>(value));
          emit changed();
        }
      }
      else
      {
        if(m_settings.value(key) != value)
        {
          m_settings.setValue(key, value);
          emit changed();
        }
      }
    }

  protected:
    SettingsBase(const QString& group)
      : QObject()
      , m_systemSettings(
        QSettings::SystemScope      
#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
  #ifdef Q_OS_DARWIN
        , QCoreApplication::organizationDomain().isEmpty()
            ? QCoreApplication::organizationName()
            : QCoreApplication::organizationDomain()
  #else
        , QCoreApplication::organizationName().isEmpty()
            ? QCoreApplication::organizationDomain()
            : QCoreApplication::organizationName()
  #endif
        , QCoreApplication::applicationName()
#endif
        )
    {
      m_systemSettings.beginGroup(group);
      m_settings.beginGroup(group);
    }

    SettingsBase(const SettingsBase&) = delete;

  signals:
    void changed();
};

#endif
