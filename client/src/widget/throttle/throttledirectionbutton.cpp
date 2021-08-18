/**
 * client/src/widget/throttle/throttledirectionbutton.cpp
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

#include "throttledirectionbutton.hpp"
#include <traintastic/locale/locale.hpp>
#include "../../network/object.hpp"
#include "../../network/abstractproperty.hpp"

ThrottleDirectionButton::ThrottleDirectionButton(ObjectPtr object, Direction direction, QWidget* parent)
  : AbstractThrottleButton(std::move(object), parent)
  , m_direction{direction}
  , m_directionProperty{m_object->getProperty("direction")}
{
  connect(m_directionProperty, &AbstractProperty::valueChanged, this, &ThrottleDirectionButton::directionChanged);
  setToolTip(Locale::tr(QString(EnumName<Direction>::value).append(":").append(EnumValues<Direction>::value.at(m_direction))));

  setText(m_direction == Direction::Reverse ? "<" : ">");
  directionChanged();
}

void ThrottleDirectionButton::click()
{
  m_directionProperty->setValueEnum(m_direction);
}

void ThrottleDirectionButton::directionChanged()
{
  const bool active = m_directionProperty->toEnum<Direction>() == m_direction;
  setTextColor(active ? Qt::white : Qt::gray);
  //! \todo setResource();
}
