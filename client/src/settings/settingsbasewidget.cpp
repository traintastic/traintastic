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
#include <traintastic/locale/locale.hpp>

SettingsBaseWidget::SettingsBaseWidget(QWidget* parent) :
  QScrollArea(parent)
{
  setMinimumSize(400, 400);
  setWidgetResizable(true);
  setWidget(new QWidget(this));
  widget()->setLayout(new QVBoxLayout());
  widget()->show();
}

void SettingsBaseWidget::add(const QString& displayName, QWidget* widget)
{
  QHBoxLayout* l = new QHBoxLayout();
  l->addWidget(new QLabel(Locale::tr(displayName), this->widget()));
  l->addStretch();
  l->addWidget(widget);
  static_cast<QVBoxLayout*>(this->widget()->layout())->addLayout(l);
}

void SettingsBaseWidget::addOnOffSetting(const QString& displayName, std::function<bool()> getter, std::function<void(bool)> setter)
{
  QCheckBox* cb = new QCheckBox(widget());
  cb->setChecked(getter());
  connect(cb, &QCheckBox::toggled, setter);
  add(displayName, cb);
}

void SettingsBaseWidget::done()
{
  static_cast<QVBoxLayout*>(widget()->layout())->addStretch();
}
