/**
 * client/src/clock/clock.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#include "clock.hpp"
#include <QVBoxLayout>
#include <traintastic/locale/locale.hpp>
#include "analogclock.hpp"
#include "../network/object.hpp"
#include "../network/abstractproperty.hpp"
#include "../theme/theme.hpp"

Clock::Clock(ObjectPtr clock, QWidget* parent)
  : QWidget(parent)
  , m_clock{std::move(clock)}
  , m_hour{m_clock->getProperty("hour")}
  , m_minute{m_clock->getProperty("minute")}
  , m_analogClock{new AnalogClock(this)}
{
  setWindowTitle(Locale::tr("world:clock"));
  setWindowIcon(Theme::getIcon("clock"));

  auto* l = new QVBoxLayout();
  l->addWidget(m_analogClock);
  setLayout(l);

  connect(m_hour, &AbstractProperty::valueChanged, this, &Clock::timeChanged);
  connect(m_minute, &AbstractProperty::valueChanged, this, &Clock::timeChanged);

  timeChanged();
}

void Clock::timeChanged()
{
  m_analogClock->setTime(m_hour->toInt(), m_minute->toInt());
}
