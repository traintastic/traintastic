/**
 * client/src/clock/analogclock.cpp
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

#include "analogclock.hpp"
#include <QPainter>

AnalogClock::AnalogClock(QWidget* parent)
  : QWidget(parent)
{
  setMinimumSize(100, 100);
}

int AnalogClock::hour() const
{
  return m_hour;
}

void AnalogClock::setHour(int value)
{
  if(value >= -1 && value < 24 && m_hour != value)
  {
    m_hour = value;
    update();
  }
}

int AnalogClock::minute() const
{
  return m_minute;
}

void AnalogClock::setMinute(int value)
{
  if(value >= -1 && value < 60 && m_minute != value)
  {
    m_minute = value;
    update();
  }
}

void AnalogClock::setTime(int hour, int minute)
{
  if((hour >= -1 && hour < 24 && m_hour != hour) || (minute >= -1 && minute < 60 && m_minute != minute))
  {
    m_hour = hour;
    m_minute = minute;
    update();
  }
}

void AnalogClock::paintEvent(QPaintEvent*)
{
  const auto color = palette().window().color().lightnessF() < 0.5 ? Qt::white : Qt::black;

  QRectF r{rect()};

  const qreal size = qMin(r.width(), r.height());
  if(r.width() > size)
  {
    r.moveTo((r.width() - size) / 2, 0);
    r.setWidth(size);
  }
  else if(r.height() > size)
  {
    r.moveTo(0, (r.height() - size) / 2);
    r.setHeight(size);
  }

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);

  painter.setPen(color);
  painter.drawEllipse(r);

  QPen pen{color};
  pen.setWidth(size / 25);
  pen.setCapStyle(Qt::RoundCap);
  painter.setPen(pen);

  painter.translate(r.left() + size / 2, r.top() + size / 2);

  // hour:
  painter.save();
  int hour12 = m_hour % 12;
  if(hour12 != 0 || m_minute != 0)
    painter.rotate((360.0 / 12) * (hour12 + m_minute / 60.0));
  painter.drawLine(0, 0, 0, -size * 0.33);
  painter.restore();

  // minute:
  painter.save();
  if(m_minute != 0)
    painter.rotate((360.0 / 60) * m_minute);
  painter.drawLine(0, 0, 0, -size * 0.44);
  painter.restore();
}

bool AnalogClock::isTimeValid() const
{
  return m_hour != -1 && m_minute != -1;
}
