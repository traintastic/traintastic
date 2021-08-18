/**
 * client/src/widget/throttle/throttlefunctionbutton.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#include "throttlefunctionbutton.hpp"
#include <QPainter>
#include <QMouseEvent>
#include <QSvgRenderer>
#include <traintastic/enum/decoderfunctiontype.hpp>
#include <traintastic/enum/decoderfunctionfunction.hpp>
#include "../../network/object.hpp"
#include "../../network/abstractproperty.hpp"

ThrottleFunctionButton::ThrottleFunctionButton(ObjectPtr object, QWidget* parent)
  : AbstractThrottleButton(std::move(object), parent)
  , m_number{m_object->getProperty("number")}
  , m_name{m_object->getProperty("name")}
  , m_type{m_object->getProperty("type")}
  , m_function{m_object->getProperty("function")}
  , m_value{m_object->getProperty("value")}
{
  connect(m_number, &AbstractProperty::valueChangedInt, this,
    [this](int value)
    {
      setText("F" + QString::number(value));
    });
  connect(m_name, &AbstractProperty::valueChangedString, this,
    [this](const QString& value)
    {
      setToolTip(value);
    });
  connect(m_function, &AbstractProperty::valueChanged, this, &ThrottleFunctionButton::functionOrValueChanged);
  connect(m_value, &AbstractProperty::valueChanged, this, &ThrottleFunctionButton::functionOrValueChanged);
  setToolTip(m_name->toString());
  setText("F" + QString::number(m_number->toInt()));
  functionOrValueChanged();
}

int ThrottleFunctionButton::number() const
{
  return m_number->toInt();
}

DecoderFunctionFunction ThrottleFunctionButton::function() const
{
  return m_function->toEnum<DecoderFunctionFunction>();
}

void ThrottleFunctionButton::click()
{
  m_value->setValueBool(!m_value->toBool());
}

void ThrottleFunctionButton::press()
{
  if(m_type->toEnum<DecoderFunctionType>() == DecoderFunctionType::Hold)
    m_value->setValueBool(true);
}

void ThrottleFunctionButton::release()
{
  if(m_type->toEnum<DecoderFunctionType>() == DecoderFunctionType::Hold)
    m_value->setValueBool(false);
  else
    click();
}

void ThrottleFunctionButton::functionOrValueChanged()
{
  const bool active = m_value->toBool();
  setTextColor(active ? Qt::white : Qt::gray);
  setResource(
    QString(":/dark/decoder_function.")
      .append(EnumValues<DecoderFunctionFunction>::value.at(m_function->toEnum<DecoderFunctionFunction>()))
      .append(active ? ".on" : ".off")
      .append(".svg"));
}

void ThrottleFunctionButton::mousePressEvent(QMouseEvent* event)
{
  if(m_type->toEnum<DecoderFunctionType>() == DecoderFunctionType::Hold)
    press();
  else
    AbstractThrottleButton::mousePressEvent(event);
}

void ThrottleFunctionButton::mouseReleaseEvent(QMouseEvent* event)
{
  if(m_type->toEnum<DecoderFunctionType>() == DecoderFunctionType::Hold)
    release();
  else
    AbstractThrottleButton::mouseReleaseEvent(event);
}
