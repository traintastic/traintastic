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

#include "clock.hpp"
#include "../core/attributes.hpp"
#include "../core/eventloop.hpp"
#include "../log/log.hpp"
#include "../world/getworld.hpp"

Clock::Clock(Object& _parent, std::string_view parentPropertyName)
  : SubObject(_parent, parentPropertyName)
  , m_timer{EventLoop::ioContext}
  , time{this, "time", 0, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , hour{this, "hour", 0, PropertyFlags::ReadWrite | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly}
  , minute{this, "minute", 0, PropertyFlags::ReadWrite | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly}
  , multiplier{this, "multiplier", 1, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly}
  , freeze{this, "freeze", true, PropertyFlags::ReadWrite | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly,
      [this](bool /*value*/)
      {
        update();
      }}
  , running{this, "running", false, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , debugLog{this, "debug_log", false, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript}
  , onResume{*this, "on_resume", EventFlags::Scriptable}
  , onTick{*this, "on_tick", EventFlags::Scriptable}
  , onFreeze{*this, "on_freeze", EventFlags::Scriptable}
{
  const bool editable = isEditable();

  Attributes::addObjectEditor(time, false);
  m_interfaceItems.add(time);

  Attributes::addEnabled(hour, editable);
  Attributes::addMinMax(hour, hourMin, hourMax);
  m_interfaceItems.add(hour);

  Attributes::addEnabled(minute, editable);
  Attributes::addMinMax(minute, minuteMin, minuteMax);
  m_interfaceItems.add(minute);

  Attributes::addEnabled(multiplier, editable);
  Attributes::addMinMax(multiplier, multiplierMin, multiplierMax);
  m_interfaceItems.add(multiplier);

  m_interfaceItems.add(freeze);

  Attributes::addObjectEditor(running, false);
  m_interfaceItems.add(running);

  m_interfaceItems.add(debugLog);

  m_interfaceItems.add(onResume);
  m_interfaceItems.add(onTick);
  m_interfaceItems.add(onFreeze);
}

void Clock::loaded()
{
  SubObject::loaded();

  m_time.set(hour, minute);
  time.setValueInternal(m_time.ticks);

  update();
}

void Clock::worldEvent(WorldState state, WorldEvent event)
{
  SubObject::worldEvent(state, event);

  switch(event)
  {
    case WorldEvent::EditDisabled:
    case WorldEvent::EditEnabled:
    case WorldEvent::PowerOff:
    case WorldEvent::Stop:
    case WorldEvent::Run:
      update();
      break;

    default:
      break;
  }
}

bool Clock::isEditable() const
{
  const auto& world = getWorld(parent());
  return contains(world.state.value(), WorldState::Edit) && (!contains(world.state.value(), WorldState::Run) || freeze);
}

void Clock::tick(const boost::system::error_code& ec)
{
  if(ec || !running)
    return;

  // update clock:
  ++m_time;

  // debug log accuracy:
  if(debugLog)
    Log::log(classId, LogMessage::D1002_TICK_X_ERROR_X_US, m_time, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_nextTick).count());

  // restart timer:
  m_nextTick += m_tickInterval;
  m_timer.expires_after(m_nextTick - std::chrono::steady_clock::now());
  m_timer.async_wait(std::bind(&Clock::tick, this, std::placeholders::_1));

  // update properties:
  time.setValueInternal(m_time.ticks);
  hour.setValueInternal(m_time.hour());
  minute.setValueInternal(m_time.minute());

  // fire events:
  onChange(ClockEvent::Tick, multiplier, m_time);
  fireEvent(onTick, m_time.ticks);
}

void Clock::update()
{
  Attributes::setEnabled({hour, minute, multiplier}, isEditable());

  const bool run = contains(getWorld(parent()).state.value(), WorldState::Run) && !freeze;
  if(running != run)
  {
    if(run) // resume clock
    {
      m_time.set(hour, minute); // when clock is stopped time can be adjusted
      time.setValueInternal(m_time.ticks);

      using namespace std::chrono_literals;
      m_tickInterval = 60'000'000us / multiplier.value();
      m_nextTick = std::chrono::steady_clock::now() + m_tickInterval;

      m_timer.expires_after(m_nextTick - std::chrono::steady_clock::now());
      m_timer.async_wait(std::bind(&Clock::tick, this, std::placeholders::_1));

      if(debugLog)
        Log::log(classId, LogMessage::D1001_RESUME_X_MULTIPLIER_X, m_time, multiplier.value());

      onChange(ClockEvent::Resume, multiplier, m_time);
      fireEvent(onResume, m_time.ticks, multiplier.value());
    }
    else // freeze clock
    {
      m_timer.cancel();

      if(debugLog)
        Log::log(classId, LogMessage::D1003_FREEZE_X, m_time);

      onChange(ClockEvent::Freeze, multiplier, m_time);
      fireEvent(onFreeze, m_time.ticks);
    }
    running.setValueInternal(run);
  }
}
