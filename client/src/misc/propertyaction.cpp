/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "propertyaction.hpp"
#include "../network/abstractproperty.hpp"

PropertyAction::PropertyAction(const QIcon &icon, AbstractProperty& property, QObject* parent)
  : QAction(icon, QString(), parent)
  , m_property{property}
{
  assert(property.type() == ValueType::Boolean);

  setText(m_property.displayName());
  setEnabled(m_property.getAttributeBool(AttributeName::Enabled, true));
  setVisible(m_property.getAttributeBool(AttributeName::Visible, true));
  setCheckable(true);
  setChecked(m_property.toBool());

  connect(&m_property, &AbstractProperty::valueChangedBool, this,
    [this](bool value)
    {
      setChecked(value);
    });

  connect(&m_property, &AbstractProperty::attributeChanged, this,
    [this](AttributeName name, const QVariant& value)
    {
      if(name == AttributeName::Enabled)
      {
        setEnabled(value.toBool());
      }
      else if(name == AttributeName::Visible)
      {
        setVisible(value.toBool());
      }
      else if(name == AttributeName::DisplayName)
      {
        setText(m_property.displayName());
      }
    });

  connect(this, &QAction::toggled, this,
    [this](bool value)
    {
      m_property.setValueBool(value);
    });
}

PropertyAction::PropertyAction(AbstractProperty& property, QObject* parent)
  : PropertyAction(QIcon(), property, parent)
{
}
