/**
 * client/src/widget/propertyspinbox.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2024 Reinder Feenstra
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

#include "propertyspinbox.hpp"
#include <QToolTip>
#include "../network/property.hpp"
#include "../network/object.hpp"
#include "../network/connection.hpp"
#include "../network/error.hpp"
#include <traintastic/locale/locale.hpp>

PropertySpinBox::PropertySpinBox(Property& property, QWidget* parent) :
  QSpinBox(parent),
  m_property{property}
{
  Q_ASSERT(m_property.type() == ValueType::Integer);
  setEnabled(m_property.getAttributeBool(AttributeName::Enabled, true));
  setVisible(m_property.getAttributeBool(AttributeName::Visible, true));
  updateRange();
  if(auto unit = m_property.getAttributeString(AttributeName::Unit, ""); !unit.isEmpty())
  {
    setSuffix(unit.prepend(" "));
  }
  setValue(m_property.toInt());
  connect(&m_property, &AbstractProperty::valueChangedInt, this,
    [this](int value)
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

        case AttributeName::Min:
        case AttributeName::Max:
          updateRange();
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
  connect(this, QOverload<int>::of(&PropertySpinBox::valueChanged),
    [this](int value)
    {
      cancelRequest();
      m_requestId = m_property.setValueInt(value,
        [this](std::optional<const Error> error)
        {
          if(error)
          {
            showError(error->toString());
          }
        });
    });
}

PropertySpinBox::~PropertySpinBox()
{
  cancelRequest();
}

void PropertySpinBox::cancelRequest()
{
  if(m_requestId != Connection::invalidRequestId)
  {
    m_property.object().connection()->cancelRequest(m_requestId);
    m_requestId = Connection::invalidRequestId;
  }
}

void PropertySpinBox::showError(const QString& error)
{
  // TODO: replace by a proper error tip
  if(!error.isEmpty())
    QToolTip::showText(mapToGlobal(rect().bottomLeft()), Locale::tr(error));
  else
    QToolTip::hideText();
}

void PropertySpinBox::focusOutEvent(QFocusEvent* event)
{
  QSpinBox::focusOutEvent(event);
  setValue(m_property.toInt());
}

void PropertySpinBox::updateRange()
{
  setRange(
    m_property.getAttributeInt(AttributeName::Min, std::numeric_limits<int>::min()),
    m_property.getAttributeInt(AttributeName::Max, std::numeric_limits<int>::max()));
}
