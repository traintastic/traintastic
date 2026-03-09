/**
 * client/src/theme/theme.cpp
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

#include "theme.hpp"
#include <cassert>
#include <QFile>
#include <QPalette>
#include <QWidget>

const QString iconPathDefault = QStringLiteral(":/");
const QString iconPathDark = QStringLiteral(":/dark/");
const QString iconPathLight = QStringLiteral(":/light/");
const QString iconExtension = QStringLiteral(".svg");
const std::array<const QString*, 3> iconPathsDark = {&iconPathDark, &iconPathDefault, &iconPathLight};
const std::array<const QString*, 3> iconPathsLight = {&iconPathLight, &iconPathDefault, &iconPathDark};

bool Theme::s_isDark = false;
Theme::IconSet Theme::s_iconSet = Theme::IconSet::Light;

bool Theme::isDark()
{
  return s_isDark;
}

void Theme::setDark(bool value)
{
  s_isDark = value;
  setIconSet(value ? Theme::IconSet::Dark : Theme::IconSet::Light);
}

void Theme::setIconSet(IconSet value)
{
  s_iconSet = value;
}

QIcon Theme::getIcon(const QString& idOff, const QString& idOn)
{
  QIcon icon;
  icon.addFile(getIconFile(idOff), QSize(), QIcon::Normal, QIcon::Off);
  icon.addFile(getIconFile(idOn), QSize(), QIcon::Normal, QIcon::On);
  return icon;
}

QIcon Theme::getIconForClassId(const QString& classId)
{
  if(classId == "lua.script" || classId == "lua.script_list")
    return getIcon("lua");
  else if(classId == "board" || classId == "list.board")
    return getIcon("board");
  else if(classId == "clock")
    return getIcon("clock");
  else if(classId == "train" || classId == "list.train")
    return getIcon("train");
  else if(classId == "zone" || classId == "list.zone")
    return getIcon("zone");
  else
    return QIcon();
}

void Theme::setWindowIcon(QWidget& widget, const QString& classId)
{
  if(auto icon = Theme::getIconForClassId(classId); !icon.isNull())
  {
    widget.setWindowIcon(icon);
  }
  else
  {
    widget.setWindowIcon(QIcon(":/appicon.svg"));
  }
}

const std::array<const QString*, 3>& Theme::getIconPaths()
{
  switch(s_iconSet)
  {
    case IconSet::Dark:
      return iconPathsDark;

    case IconSet::Light:
      return iconPathsLight;
  }
  assert(false);
  return iconPathsLight;
}

QString Theme::getIconFile(const QString& id)
{
  for(auto path : getIconPaths())
  {
    QString file = *path + id + iconExtension;
    if(QFile::exists(file))
      return file;
  }
  return QString();
}
