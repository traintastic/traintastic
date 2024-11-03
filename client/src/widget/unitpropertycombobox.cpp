/**
 * client/src/widget/unitpropertycombobox.cpp
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

#include "unitpropertycombobox.hpp"
#include <QHBoxLayout>
#include <QComboBox>
#include <traintastic/locale/locale.hpp>
#include "../network/unitproperty.hpp"
#include "../utils/internalupdateholder.hpp"
#include "../utils/enum.hpp"

UnitPropertyComboBox::UnitPropertyComboBox(UnitProperty& property, QWidget* parent)
  : QWidget(parent)
  , m_property{property}
  , m_valueComboBox{new QComboBox(this)}
  , m_unitComboBox{new QComboBox(this)}
{
  setEnabled(m_property.getAttributeBool(AttributeName::Enabled, true));
  setVisible(m_property.getAttributeBool(AttributeName::Visible, true));

  connect(&m_property, &UnitProperty::attributeChanged, this,
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

  connect(&m_property, &Property::valueChangedDouble, this,
    [this](double value)
    {
      InternalUpdateHolder hold(m_internalUpdate);
      if(int index = m_valueComboBox->findData(value); index != -1) // predefined value
      {
        m_valueComboBox->setCurrentIndex(index);
      }
      else // custom value
      {
        m_valueComboBox->setCurrentText(QString::number(value));
      }
    });

  m_valueComboBox->setEditable(true); // FIXME
  m_valueComboBox->setInsertPolicy(QComboBox::NoInsert);

  if(m_valueComboBox->isEditable())
  {
    connect(m_valueComboBox, &QComboBox::currentTextChanged,
      [this](const QString& value)
      {
        if(m_internalUpdate == 0)
        {
          const QVariant v = m_valueComboBox->currentData();
          if(v.isValid()) // predefined value
          {
            m_property.setValueDouble(v.value<double>());
          }
          else // custom value
          {
            m_property.setValueString(value);
          }
        }
      });
  }
  else
  {
    connect(m_valueComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
      [this](int)
      {
        if(m_internalUpdate == 0)
        {
          const QVariant v = m_valueComboBox->currentData();
          m_property.setValueDouble(v.value<double>());
        }
      });
  }

  QHBoxLayout* l = new QHBoxLayout();
  l->setContentsMargins(0, 0, 0, 0);
  l->addWidget(m_valueComboBox, 1);

  for(qint64 value : enumValues(m_property.unitName()))
  {
    m_unitComboBox->addItem(translateEnum(m_property.unitName(), value), value);
    if(m_property.unitValue() == value)
      m_unitComboBox->setCurrentIndex(m_unitComboBox->count() - 1);
  }
  l->addWidget(m_unitComboBox);
  connect(m_unitComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
    [this](int)
    {
      if(QVariant v = m_unitComboBox->currentData(); v.canConvert<qint64>())
        if(qint64 value = v.toLongLong(); value != m_property.unitValue())
          m_property.setUnitValue(value);
    });

  setLayout(l);
  updateValues();
}

void UnitPropertyComboBox::updateValues()
{
  QVariant values = m_property.getAttribute(AttributeName::Values, QVariant());
  if(Q_LIKELY(values.isValid()))
  {
    InternalUpdateHolder hold(m_internalUpdate);

    m_valueComboBox->clear();

    if(Q_LIKELY(values.userType() == QMetaType::QVariantList))
    {
      bool currentIndexSet = false;

      switch(m_property.type())
      {
        case ValueType::Float:
        {
          const QVariantList aliasKeys = m_property.getAttribute(AttributeName::AliasKeys, QVariant()).toList();
          const QVariantList aliasValues = m_property.getAttribute(AttributeName::AliasValues, QVariant()).toList();

          for(QVariant& v : values.toList())
          {
            const auto value = v.toDouble();
            if(int index = aliasKeys.indexOf(v); index != -1)
            {
              m_valueComboBox->addItem(Locale::instance->parse(aliasValues[index].toString()), value);
            }
            else
            {
              m_valueComboBox->addItem(QString::number(value), value);
            }

            if(qFuzzyCompare(m_property.toDouble(), value) || m_property.toDouble() == value)
            {
              m_valueComboBox->setCurrentIndex(m_valueComboBox->count() - 1);
              currentIndexSet = true;
            }
          }
          break;
        }
        case ValueType::Invalid:
        case ValueType::Boolean:
        case ValueType::Integer:
        case ValueType::Enum:
        case ValueType::String:
        case ValueType::Object:
        case ValueType::Set:
          assert(false);
          break;
      }

      if(m_valueComboBox->isEditable() && !currentIndexSet)
      {
        m_valueComboBox->setCurrentText(m_property.toString());
      }
    }
  }
}
