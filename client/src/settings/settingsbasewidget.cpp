/**
 * client/src/settings/settingsbasewidget.cpp
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

#include "settingsbasewidget.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QToolButton>
#include <QFileDialog>
#include <traintastic/locale/locale.hpp>

SettingsBaseWidget::SettingsBaseWidget(QString trPrefix, QWidget* parent)
  : QScrollArea(parent)
  , m_trPrefix{std::move(trPrefix)}
{
  setMinimumSize(400, 400);
  setWidgetResizable(true);
  setWidget(new QWidget(this));
  widget()->setLayout(new QVBoxLayout());
  widget()->show();
}

void SettingsBaseWidget::add(const QString& settingName, QWidget* widget)
{
  QHBoxLayout* l = new QHBoxLayout();
  l->addWidget(new QLabel(Locale::tr(m_trPrefix + ":" + settingName), this->widget()));
  l->addStretch();
  l->addWidget(widget);
  static_cast<QVBoxLayout*>(this->widget()->layout())->addLayout(l);
}

void SettingsBaseWidget::add(const QString& settingName, QLayout* layout)
{
  QHBoxLayout* l = new QHBoxLayout();
  l->addWidget(new QLabel(Locale::tr(m_trPrefix + ":" + settingName), this->widget()));
  l->addStretch();
  l->addLayout(layout);
  static_cast<QVBoxLayout*>(this->widget()->layout())->addLayout(l);
}

void SettingsBaseWidget::addSettingOnOff(Setting<bool>& setting)
{
  QCheckBox* cb = new QCheckBox(widget());
  cb->setChecked(setting.value());
  connect(cb, &QCheckBox::toggled, [&setting](bool value){ setting.setValue(value); });
  add(setting.name(), cb);
}

void SettingsBaseWidget::addSettingDir(Setting<QString>& setting)
{
  QLineEdit* edt = new QLineEdit(setting.value(), this);
  connect(edt, &QLineEdit::textChanged, [&setting](const QString& value){ setting.setValue(value); });

  QToolButton* btn = new QToolButton(this);
  connect(btn, &QToolButton::clicked,
    [this, edt]()
    {
      const QString dir = QFileDialog::getExistingDirectory(this, Locale::tr("settings:select_folder"), edt->text());
      if(!dir.isEmpty())
        edt->setText(dir);
    });

  QHBoxLayout* l = new QHBoxLayout();
  l->addWidget(edt);
  l->addWidget(btn);
  add(setting.name(), l);
}

void SettingsBaseWidget::done()
{
  static_cast<QVBoxLayout*>(widget()->layout())->addStretch();
}
