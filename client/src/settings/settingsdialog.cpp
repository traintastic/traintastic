/**
 * client/src/settings/settingsdialog.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

#include "settingsdialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <traintastic/locale/locale.hpp>
#include "generalsettingswidget.hpp"
#include "statusbarsettingswidget.hpp"
#include "boardsettingswidget.hpp"
#include "developersettingswidget.hpp"

SettingsDialog::SettingsDialog(QWidget* parent) :
  QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
  m_menu{new QVBoxLayout()},
  m_stack{new QStackedWidget(this)}
{
  setWindowTitle(Locale::tr("qtapp.mainmenu:settings"));

  add(new GeneralSettingsWidget(this));
  add(new StatusBarSettingsWidget(this));
  add(new BoardSettingsWidget(this));
  add(new DeveloperSettingsWidget(this));

  QHBoxLayout* l = new QHBoxLayout();
  m_menu->addStretch();
  l->addLayout(m_menu);
  l->addWidget(m_stack);
  setLayout(l);
}

void SettingsDialog::add(SettingsBaseWidget* widget)
{
  //! @todo replace button by more stylish selector
  auto* pb = new QPushButton(Locale::tr(widget->title()));
  connect(pb, &QPushButton::clicked, this,
    [this, index=m_stack->count()]()
    {
      m_stack->setCurrentWidget(m_stack->widget(index));
    });
  m_menu->addWidget(pb);

  m_stack->addWidget(widget);
}
