/**
 * server/src/clock/clock.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CLOCK_CLOCK_HPP
#define TRAINTASTIC_SERVER_CLOCK_CLOCK_HPP

#include "../core/subobject.hpp"
#include <boost/asio/steady_timer.hpp>
#include "time.hpp"
#include "../core/property.hpp"
#include "../core/event.hpp"

class Clock : public SubObject
{
  CLASS_ID("clock")

  private:
    static constexpr uint8_t hourMin = 0;
    static constexpr uint8_t hourMax = 23;
    static constexpr uint8_t minuteMin = 0;
    static constexpr uint8_t minuteMax = 59;
    static constexpr uint8_t multiplierMin = 1;
    static constexpr uint8_t multiplierMax = 120;

    boost::asio::steady_timer m_timer;
    Time m_time;
    std::chrono::microseconds m_tickInterval;
    std::chrono::time_point<std::chrono::steady_clock> m_nextTick;

    bool isEditable() const;
    void tick(const boost::system::error_code& ec);
    void update();

  protected:
    void loaded() final;
    void worldEvent(WorldState state, WorldEvent event) final;

  public:
    enum class ClockEvent
    {
      Resume,
      Tick,
      Freeze,
    };
    boost::signals2::signal<void(ClockEvent, uint8_t, Time)> onChange;

    Property<uint16_t> time;
    Property<uint8_t> hour;
    Property<uint8_t> minute;
    Property<uint8_t> multiplier;
    Property<bool> freeze;
    Property<bool> running;
    Property<bool> debugLog;
    Event<uint16_t, uint8_t> onResume;
    Event<uint16_t> onTick;
    Event<uint16_t> onFreeze;

    Clock(Object& _parent, std::string_view parentPropertyName);
};

#endif
