/**
 * client/src/settings/generalsettingswidget.cpp
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

#include "generalsettingswidget.hpp"
#include <QComboBox>
#include <QDir>
#include <QDirIterator>
#include <traintastic/locale/locale.hpp>
#include "generalsettings.hpp"
#include "../utils/getlocalepath.hpp"

static QString getLanguageName(const QString& filename)
{
  QFile file(filename);
  if(file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    const QString header = QStringLiteral("## Traintastic language file: ");
    QString line = QString::fromUtf8(file.readLine());
    if(line.startsWith(header))
      return line.remove(0, header.length()).trimmed();
  }
  return "";
}

GeneralSettingsWidget::GeneralSettingsWidget(QWidget* parent) :
  SettingsBaseWidget(parent)
{
  GeneralSettings& s = GeneralSettings::instance();

  // language:
  {
    QComboBox* cb = new QComboBox(this);
    QDirIterator it(getLocalePath(), {"*.txt"}, QDir::Files | QDir::Readable);
    while(it.hasNext())
    {
      const QString filename = it.next();
      const QString label = getLanguageName(filename);
      if(!label.isEmpty())
      {
        cb->addItem(label, QFileInfo(filename).baseName());
        if(filename.toStdString() == Locale::instance->filename)
          cb->setCurrentIndex(cb->count() - 1);
      }
    }
    connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
      [cb](int index)
      {
        GeneralSettings::instance().setLanguage(cb->itemData(index).toString());
      });
    add("qtapp.settings.general:language", cb);
  }


  done();
}
