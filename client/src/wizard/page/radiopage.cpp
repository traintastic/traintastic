/**
 * client/src/wizard/page/radiopage.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "radiopage.hpp"
#include <QLayout>
#include <QButtonGroup>
#include <QRadioButton>

RadioPage::RadioPage(QWidget* parent)
  : TextPage(parent)
  , m_group{new QButtonGroup(this)}
{
}

int RadioPage::currentIndex() const
{
  return m_group->id(m_group->checkedButton());
}

void RadioPage::addItem(const QString& label, bool disabled)
{
  auto* button = new QRadioButton(label);
  button->setDisabled(disabled);
  m_group->addButton(button, m_group->buttons().size());
  layout()->addWidget(button);
}

void RadioPage::clear()
{
  while(layout()->count() > 1) // remove all but first (=text label)
  {
    auto* item = layout()->itemAt(layout()->count() - 1);
    if(item->widget())
    {
      delete item->widget();
    }
    layout()->removeItem(item);
  }
}
