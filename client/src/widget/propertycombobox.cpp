/**
 * client/src/widget/propertycombobox.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2021 Reinder Feenstra
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

#include "propertycombobox.hpp"
#include <cassert>
#include <traintastic/locale/locale.hpp>
#include "../network/property.hpp"
#include "../utils/internalupdateholder.hpp"
#include "../utils/enum.hpp"

PropertyComboBox::PropertyComboBox(Property& property, QWidget* parent) :
  QComboBox(parent),
  m_property{property},
  m_internalUpdate{0}
{
  Q_ASSERT(m_property.type() == ValueType::Enum || m_property.type() == ValueType::Integer);
  setEnabled(m_property.getAttributeBool(AttributeName::Enabled, true));
  setVisible(m_property.getAttributeBool(AttributeName::Visible, true));

  connect(&m_property, &Property::valueChangedInt64, this,
    [this](qint64 value)
    {
      InternalUpdateHolder hold(m_internalUpdate);
      if(int index = findData(value); index != -1)
        setCurrentIndex(index);
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

        case AttributeName::AliasKeys:
        case AttributeName::AliasValues:
        case AttributeName::Values:
          updateValues();
          break;

        default:
          break;
      }
    });
  connect(this, static_cast<void(PropertyComboBox::*)(int)>(&PropertyComboBox::currentIndexChanged),
    [this](int)
    {
      if(m_internalUpdate == 0)
        if(QVariant v = currentData(); v.canConvert<int>())
          m_property.setValueInt(v.toInt());
    });

  updateValues();
}

void PropertyComboBox::updateValues()
{
  QVariant values = m_property.getAttribute(AttributeName::Values, QVariant());
  if(Q_LIKELY(values.isValid()))
  {
    InternalUpdateHolder hold(m_internalUpdate);//QSignalBlocker block(this);
    clear();
    if(Q_LIKELY(values.userType() == QMetaType::QVariantList))
    {
      switch(m_property.type())
      {
        case ValueType::Integer:
        {
          const QVariantList aliasKeys = m_property.getAttribute(AttributeName::AliasKeys, QVariant()).toList();
          const QVariantList aliasValues = m_property.getAttribute(AttributeName::AliasValues, QVariant()).toList();

          for(QVariant& v : values.toList())
          {
            const qint64 value = v.toLongLong();
            if(int index = aliasKeys.indexOf(value); index != -1)
              addItem(Locale::instance->parse(aliasValues[index].toString()), value);
            else
              addItem(QString::number(value), value);
            if(m_property.toInt64() == value)
              setCurrentIndex(count() - 1);
          }
          break;
        }
        case ValueType::Enum:
          for(QVariant& v : values.toList())
          {
            const qint64 value = v.toLongLong();
            addItem(translateEnum(m_property.enumName(), value), value);
            if(m_property.toInt64() == value)
              setCurrentIndex(count() - 1);
          }
          break;

        case ValueType::Invalid:
        case ValueType::Boolean:
        case ValueType::Float:
        case ValueType::String:
        case ValueType::Object:
        case ValueType::Set:
          assert(false);
          break;
      }
    }
  }
}
