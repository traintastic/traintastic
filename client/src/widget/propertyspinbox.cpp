/**
 * client/src/widget/propertyspinbox.cpp
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

#include "propertyspinbox.hpp"
#include "../network/abstractproperty.hpp"

PropertySpinBox::PropertySpinBox(AbstractProperty& property, QWidget* parent) :
  QSpinBox(parent),
  m_property{property}
{
  Q_ASSERT(m_property.type() == ValueType::Integer);
  setEnabled(m_property.getAttributeBool(AttributeName::Enabled, true));
  setVisible(m_property.getAttributeBool(AttributeName::Visible, true));
  setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
  setValue(m_property.toInt());
  connect(&m_property, &AbstractProperty::valueChangedInt, this, &PropertySpinBox::setValue);
  connect(&m_property, &AbstractProperty::attributeChanged,
    [this](AttributeName name, const QVariant& value)
    {
      switch(name)
      {
        case AttributeName::Enabled:
          setEnabled(value.toBool());
          break;

        case AttributeName::Visible:
          setVisible(value.toBool());
          break;

        default:
          break;
      }
    });
  connect(this, QOverload<int>::of(&PropertySpinBox::valueChanged), &m_property, &AbstractProperty::setValueInt);
}
