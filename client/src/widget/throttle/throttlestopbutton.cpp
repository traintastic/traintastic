/**
 * client/src/widget/DecoderStopButton.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#include "throttlestopbutton.hpp"
#include <traintastic/locale/locale.hpp>
#include "../../network/object.hpp"
#include "../../network/abstractproperty.hpp"

ThrottleStopButton::ThrottleStopButton(ObjectPtr object, QWidget* parent)
  : AbstractThrottleButton(std::move(object), parent)
  , m_throttle{m_object->getProperty("throttle")}
  , m_throttleSpeed{m_object->getProperty("throttle_speed")}
  , m_emergencyStop{m_object->getProperty("emergency_stop")}
{
  setText("0");
}

void ThrottleStopButton::click()
{
  if(m_emergencyStop && m_emergencyStop->toBool())
  {
    if(m_throttleSpeed)
      m_throttleSpeed->setValueDouble(0);
    else if(m_throttle)
      m_throttle->setValueDouble(0);
    m_emergencyStop->setValueBool(false);
  }
  else
  {
    if(m_throttleSpeed && !qFuzzyIsNull(m_throttleSpeed->toDouble()))
      m_throttleSpeed->setValueDouble(0);
    else if(m_throttle && !qFuzzyIsNull(m_throttle->toDouble()))
      m_throttle->setValueDouble(0);
    else if(m_emergencyStop)
      m_emergencyStop->setValueBool(true);
  }
}
