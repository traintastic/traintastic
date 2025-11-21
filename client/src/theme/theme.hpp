/**
 * client/src/theme/theme.hpp
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

#ifndef TRAINTASTIC_CLIENT_THEME_THEME_HPP
#define TRAINTASTIC_CLIENT_THEME_THEME_HPP

#include <array>
#include <QString>
#include <QIcon>

class Theme
{
  public:
    enum class IconSet
    {
      Light,
      Dark,
    };

  private:
    Theme() = delete;

    static bool s_isDark;
    static IconSet s_iconSet;

    static const std::array<const QString*, 3>& getIconPaths();

  public:
    static bool isDark();
    static void setDark(bool value);

    static void setIconSet(IconSet value);

    static QString getIconFile(const QString& id);

    inline static QIcon getIcon(const QString& id) { return QIcon(getIconFile(id)); }
    static QIcon getIcon(const QString& idOff, const QString& idOn);
    static QIcon getIconForClassId(const QString& classId);

    static void setWindowIcon(QWidget& widget, const QString& classId);
};

#endif
