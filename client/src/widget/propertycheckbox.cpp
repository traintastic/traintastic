/**
 * client/src/widget/propertycheckbox.cpp
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

#include "propertycheckbox.hpp"
#include "../network/property.hpp"
#include "../utils/internalupdateholder.hpp"

PropertyCheckBox::PropertyCheckBox(Property& property, QWidget* parent) :
  QCheckBox(parent),
  m_property{property},
  m_internalUpdate{false}
{
  Q_ASSERT(m_property.type() == ValueType::Boolean);
  setEnabled(m_property.getAttributeBool(AttributeName::Enabled, true));
  setVisible(m_property.getAttributeBool(AttributeName::Visible, true));
  setChecked(m_property.toBool());
  connect(&m_property, &Property::valueChangedBool,
    [this](bool value)
    {
      InternalUpdateHolder hold(m_internalUpdate);
      setChecked(value);
    });
  connect(&m_property, &Property::attributeChanged,
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
  connect(this, &PropertyCheckBox::toggled,
    [this](bool value)
    {
      if(!m_internalUpdate)
        m_property.setValueBool(value);
    });
}
