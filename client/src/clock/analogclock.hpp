/**
 * client/src/clock/analogclock.hpp
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

#ifndef TRAINTASTIC_CLIENT_CLOCK_ANALOGCLOCK_HPP
#define TRAINTASTIC_CLIENT_CLOCK_ANALOGCLOCK_HPP

#include <QWidget>

class AnalogClock : public QWidget
{
  private:
    int m_hour = -1;
    int m_minute = -1;

    bool isTimeValid() const;

  protected:
    void paintEvent(QPaintEvent*) final;

  public:
    explicit AnalogClock(QWidget* parent = nullptr);

    int hour() const;
    void setHour(int value);

    int minute() const;
    void setMinute(int value);

    void setTime(int hour, int minute);
};

#endif
