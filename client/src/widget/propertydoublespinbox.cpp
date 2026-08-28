/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2021-2026 Reinder Feenstra
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

#include "propertydoublespinbox.hpp"
#include <QToolTip>
#include "../network/abstractproperty.hpp"

PropertyDoubleSpinBox::PropertyDoubleSpinBox(AbstractProperty& property, QWidget* parent)
  : QDoubleSpinBox(parent)
  , m_property{property}
{
  Q_ASSERT(m_property.type() == ValueType::Float);
  setEnabled(m_property.getAttributeBool(AttributeName::Enabled, true));
  setVisible(m_property.getAttributeBool(AttributeName::Visible, true));
  updateRange();
  if(auto unit = m_property.getAttributeString(AttributeName::Unit, ""); !unit.isEmpty())
  {
    setSuffix(unit.prepend(" "));
  }
  setDecimals(m_property.getAttributeInt(AttributeName::Decimals, decimals()));
  setSingleStep(m_property.getAttributeDouble(AttributeName::Step, singleStep()));
  setValue(m_property.toDouble());
  connect(&m_property, &AbstractProperty::valueChangedDouble, this,
    [this](double value)
    {
      if(!hasFocus())
      {
        setValue(value);
      }
    });
  connect(&m_property, &AbstractProperty::attributeChanged, this,
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

        case AttributeName::Decimals:
          setDecimals(value.toInt());
          break;

        case AttributeName::Min:
        case AttributeName::Max:
          updateRange();
          break;

        case AttributeName::Step:
          setSingleStep(value.toDouble());
          break;

        case AttributeName::Unit:
          if(auto unit = value.toString(); !unit.isEmpty())
          {
            setSuffix(unit.prepend(" "));
          }
          else
          {
            setSuffix("");
          }
          break;

        default:
          break;
      }
    });
  connect(this, QOverload<double>::of(&PropertyDoubleSpinBox::valueChanged), &m_property, QOverload<double>::of(&AbstractProperty::setValueDouble));
}

void PropertyDoubleSpinBox::focusOutEvent(QFocusEvent* event)
{
  QDoubleSpinBox::focusOutEvent(event);
  setValue(m_property.toDouble());
}

void PropertyDoubleSpinBox::updateRange()
{
  setRange(
    m_property.getAttributeDouble(AttributeName::Min, std::numeric_limits<double>::lowest()),
    m_property.getAttributeDouble(AttributeName::Max, std::numeric_limits<double>::max()));
}
