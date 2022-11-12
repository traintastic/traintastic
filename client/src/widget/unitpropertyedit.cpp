/**
 * client/src/widget/unitpropertyedit.cpp
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

#include "unitpropertyedit.hpp"
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include "lineeditwithfocusoutsignal.hpp"
#include "../network/unitproperty.hpp"
#include "../utils/enum.hpp"

UnitPropertyEdit::UnitPropertyEdit(UnitProperty& property, QWidget *parent) :
  QWidget(parent),
  m_property{property},
  m_valueLineEdit{new LineEditWithFocusOutSignal(m_property.toString(), this)},
  m_unitComboBox{new QComboBox(this)}
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

        default:
          break;
      }
    });
  connect(&m_property, &UnitProperty::valueChanged, this,
    [this]()
    {
      if(!m_valueLineEdit->hasFocus())
        m_valueLineEdit->setText(m_property.toString());

      const qint64 unit = m_property.unitValue();
      for(int i = 0; i < m_unitComboBox->count(); i++)
        if(unit == m_unitComboBox->itemData(i).toLongLong())
        {
          m_unitComboBox->setCurrentIndex(i);
          break;
        }
    });

  QHBoxLayout* l = new QHBoxLayout();
  l->setContentsMargins(0, 0, 0, 0);

  m_valueLineEdit->setReadOnly(!m_property.isWritable());
  l->addWidget(m_valueLineEdit, 1);
  connect(m_valueLineEdit, &QLineEdit::textEdited, &m_property, QOverload<const QString&>::of(&AbstractProperty::setValueString));
  connect(m_valueLineEdit, &LineEditWithFocusOutSignal::focusOut, this,
    [this]()
    {
      m_valueLineEdit->setText(m_property.toString());
    });

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
}
