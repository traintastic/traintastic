/**
 * client/src/widget/propertydirectioncontrol.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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

#include "propertydirectioncontrol.hpp"
#include "../network/property.hpp"
#include <QHBoxLayout>
#include <QToolButton>
#include <enum/direction.hpp>

PropertyDirectionControl::PropertyDirectionControl(Property& property, QWidget* parent) :
  QWidget(parent),
  m_property{property},
  m_reverse{new QToolButton()},
  m_forward{new QToolButton()}
{
  Q_ASSERT(property.enumName() == EnumName<Direction>::value);

  QHBoxLayout* l = new QHBoxLayout();

  m_reverse->setArrowType(Qt::LeftArrow);
  //m_reverse->setCheckable(true);

  m_forward->setArrowType(Qt::RightArrow);
  //m_forward->setCheckable(true);

  l->addWidget(m_reverse);
  l->addWidget(m_forward);

  setLayout(l);

  setValue(m_property.toInt64());
  connect(&m_property, &Property::valueChangedInt64, this, &PropertyDirectionControl::setValue);
  connect(m_reverse, &QToolButton::clicked, this, &PropertyDirectionControl::buttonClicked);
  connect(m_forward, &QToolButton::clicked, this, &PropertyDirectionControl::buttonClicked);
}

void PropertyDirectionControl::setValue(int64_t value)
{
  const Direction direction = static_cast<Direction>(value);
  m_reverse->setDown(direction == Direction::Reverse);
  m_forward->setDown(direction == Direction::Forward);
}

void PropertyDirectionControl::buttonClicked(bool)
{
  if(sender() == m_reverse)
    m_property.setValueInt64(static_cast<int64_t>(Direction::Reverse));
  else if(sender() == m_forward)
    m_property.setValueInt64(static_cast<int64_t>(Direction::Forward));
  else
    Q_ASSERT(false);
}
