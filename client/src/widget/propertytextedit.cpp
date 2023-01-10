/**
 * client/src/widget/propertytextedit.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2023 Reinder Feenstra
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

#include "propertytextedit.hpp"
#include "../network/property.hpp"

PropertyTextEdit::PropertyTextEdit(Property& property, QWidget* parent) :
  QPlainTextEdit(parent),
  m_property{property}
{
  Q_ASSERT(m_property.type() == ValueType::String);
  setPlainText(m_property.toString());
  connect(&m_property, &Property::valueChangedString, this,
    [this](const QString& value)
    {
      if(toPlainText() != value)
        setPlainText(value);
    });
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

        default:
          break;
      }
    });
  connect(this, &PropertyTextEdit::textChanged, [this](){ m_property.setValueString(toPlainText()); });
}
