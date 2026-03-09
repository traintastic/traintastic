/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2020-2025 Reinder Feenstra
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

#include "propertyvaluelabel.hpp"
#include "../network/property.hpp"

PropertyValueLabel::PropertyValueLabel(Property& property, QWidget* parent) :
  QLabel(parent),
  m_property{property}
{
  setEnabled(m_property.getAttributeBool(AttributeName::Enabled, true));
  setVisible(m_property.getAttributeBool(AttributeName::Visible, true));
  updateText();
  connect(&m_property, &Property::valueChanged, this, &PropertyValueLabel::updateText);
  connect(&m_property, &Property::attributeChanged, this,
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

        case AttributeName::Unit:
          updateText();
          break;

        default:
          break;
      }
    });
}

void PropertyValueLabel::updateText()
{
  switch(m_property.type())
  {
    case ValueType::Integer:
    {
      const auto v = m_property.toInt64();
      auto s = QString("%1").arg(v);
      if(const auto unit = m_property.getAttributeString(AttributeName::Unit, {}); !unit.isEmpty())
      {
        s.append(" ").append(unit);
      }
      setText(s);
      break;
    }
    case ValueType::Float:
    {
      const auto v = m_property.toDouble();
      auto s = std::isfinite(v)
        ? QString("%1").arg(v)
        : QStringLiteral("-");
      if(const auto unit = m_property.getAttributeString(AttributeName::Unit, {}); !unit.isEmpty())
      {
        s.append(" ").append(unit);
      }
      setText(s);
      break;
    }
    default:
      setText(m_property.toString());
      break;
  }
}
