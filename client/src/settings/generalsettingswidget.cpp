/**
 * client/src/settings/generalsettingswidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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
#include <QMessageBox>
#include <traintastic/locale/locale.hpp>
#include "generalsettings.hpp"
#include <traintastic/utils/standardpaths.hpp>

static QString getLanguageName(const QString& filename)
{
  try
  {
    Locale locale(getLocalePath() / filename.toStdString());
    return locale.translate(QString("language:").append(QFileInfo(filename).baseName()));
  }
  catch(...)
  {
  }
  return "";
}

GeneralSettingsWidget::GeneralSettingsWidget(QWidget* parent)
  : SettingsBaseWidget("qtapp.settings.general", parent)
{
  GeneralSettings& s = GeneralSettings::instance();

  // language:
  {
    QComboBox* cb = new QComboBox(this);
    QDirIterator it(QString::fromStdString(getLocalePath().string()), {"*.lang"}, QDir::Files | QDir::Readable);
    while(it.hasNext())
    {
      const QString filename = it.next();
      if(QFileInfo(filename).baseName() == "neutral")
        continue;
      const QString label = getLanguageName(filename);
      if(!label.isEmpty())
      {
        cb->addItem(label, QFileInfo(filename).baseName());
        if(filename.toStdString() == Locale::instance->filename)
          cb->setCurrentIndex(cb->count() - 1);
      }
    }
    connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
      [this, cb](int index)
      {
        GeneralSettings::instance().language.setValue(cb->itemData(index).toString());
        QMessageBox::information(this, Locale::tr("qtapp.settings:restart_required"), Locale::tr("qtapp.settings.general:language_changed_restart_required"));
      });
    add(s.language.name(), cb);
  }

  addSetting(s.connectAutomaticallyToDiscoveredServer);

  done();
}
