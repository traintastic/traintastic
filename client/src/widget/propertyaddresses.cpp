/**
 * client/src/widget/propertyaddresses.cpp
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

#include "propertyaddresses.hpp"
#include <memory>
#include "../network/connection.hpp"
#include "../network/abstractvectorproperty.hpp"
#include "../network/method.hpp"
#include "../network/object.hpp"
#include "../network/error.hpp"
#include "../dialog/objectselectlistdialog.hpp"
#include "../mainwindow.hpp"
#include "../misc/methodaction.hpp"
#include "../theme/theme.hpp"
#include <QHBoxLayout>
#include <QSpinBox>
#include <QToolButton>

PropertyAddresses::PropertyAddresses(AbstractVectorProperty& property, Method* addMethod, Method* removeMethod, QWidget* parent)
  : QWidget(parent)
  , m_property{property}
{
  const bool addressesVisible = m_property.getAttributeBool(AttributeName::Visible, true);

  connect(&m_property, &AbstractVectorProperty::attributeChanged, this,
    [this](AttributeName name, const QVariant& value)
    {
      switch(name)
      {
        case AttributeName::Enabled:
          for(int i = 0; i < m_size; i++)
          {
            layout()->itemAt(i)->widget()->setEnabled(value.toBool());
          }
          break;

        case AttributeName::Visible:
          // All widgets, QToolButton visibility isn't controller by action visibility.
          for(int i = 0; i < layout()->count(); i++)
          {
            if(auto* widget = layout()->itemAt(i)->widget())
            {
              widget->setVisible(value.toBool());
            }
          }
          break;

        case AttributeName::Min:
          for(int i = 0; i < m_size; i++)
          {
            qobject_cast<QSpinBox*>(layout()->itemAt(i)->widget())->setMinimum(value.toInt());
          }
          break;

        case AttributeName::Max:
          for(int i = 0; i < m_size; i++)
          {
            qobject_cast<QSpinBox*>(layout()->itemAt(i)->widget())->setMaximum(value.toInt());
          }
          break;

        default:
          break;
      }
    });

  connect(&m_property, &AbstractVectorProperty::valueChanged, this,
    [this]()
    {
      updateSize();
    });

  auto* l = new QHBoxLayout();
  l->setContentsMargins(0, 0, 0, 0);
  l->addStretch();

  if(addMethod) /*[[likely]]*/
  {
    auto* button = new QToolButton(this);
    button->setDefaultAction(new MethodAction(Theme::getIcon("add"), *addMethod));
    button->setVisible(addressesVisible); // QToolButton visibility isn't controller by action visibility.
    l->addWidget(button);
  }

  if(removeMethod) /*[[likely]]*/
  {
    auto* button = new QToolButton(this);
    button->setDefaultAction(new MethodAction(Theme::getIcon("remove"), *removeMethod));
    button->setVisible(addressesVisible); // QToolButton visibility isn't controller by action visibility.
    l->addWidget(button);
  }

  setLayout(l);

  updateSize();
}

void PropertyAddresses::updateSize()
{
  const int newSize = m_property.size();

  while(m_size > newSize)
  {
    auto* w = layout()->itemAt(m_size - 1)->widget();
    layout()->removeWidget(w);
    delete w;
    m_size--;
  }

  if(m_size < newSize)
  {
    const bool enabled = m_property.getAttributeBool(AttributeName::Enabled, true);
    const bool visible = m_property.getAttributeBool(AttributeName::Visible, true);

    while(m_size < newSize)
    {
      auto* spinBox = new QSpinBox(this);
      spinBox->setEnabled(enabled);
      spinBox->setVisible(visible);
      spinBox->setRange(m_property.getAttributeInt(AttributeName::Min, 0), m_property.getAttributeInt(AttributeName::Max, std::numeric_limits<int>::max()));
      spinBox->setValue(m_property.getInt(m_size));
      qobject_cast<QHBoxLayout*>(layout())->insertWidget(m_size, spinBox);
      connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
        [this](int value)
        {
          m_property.setInt(layout()->indexOf(qobject_cast<QWidget*>(sender())) , value);
        });
      m_size++;
    }
  }

  assert(m_size == newSize);
}
