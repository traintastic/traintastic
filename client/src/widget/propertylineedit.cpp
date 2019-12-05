/**
 * client/src/widget/propertylineedit.cpp
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

#include "propertylineedit.hpp"
#include "../network/property.hpp"

PropertyLineEdit::PropertyLineEdit(Property& property, QWidget* parent) :
  QLineEdit(parent),
  m_property{property}
{
  Q_ASSERT(m_property.type() == PropertyType::String);
  setText(m_property.toString());
  connect(&m_property, &Property::valueChangedString, this, &PropertyLineEdit::setText);
  connect(this, &PropertyLineEdit::textEdited, &m_property, &Property::setValueString);
}
