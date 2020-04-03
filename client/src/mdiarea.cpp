/**
 * client/src/mdiarea.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#include "mdiarea.hpp"
#include <QPushButton>
#include <QAction>

MdiArea::MdiArea(QWidget* parent) :
  QMdiArea(parent)
{
}

void MdiArea::addBackgroundAction(QAction* action)
{
  if(m_backgroudnActionButtons.contains(action))
    return;

  QPushButton* button = new QPushButton(action->icon(), action->text(), this);
  connect(button, &QPushButton::clicked, action, &QAction::trigger);
  //button->setIconSize({72, 72});
  button->setMinimumSize(100, 100);
  button->move(30 + 120 * children().count(), 20);
  button->show();
  m_backgroudnActionButtons.insert(action, button);
}

void MdiArea::removeBackgroundAction(QAction* action)
{
  QPushButton* button = m_backgroudnActionButtons.value(action, nullptr);
  if(button)
    delete button;
}

