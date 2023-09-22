/**
 * server/src/hardware/protocol/kernelbase.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#include "kernelbase.hpp"
#include "../../core/eventloop.hpp"

KernelBase::KernelBase(std::string logId_)
  : m_ioContext{1}
  , logId{logId_}
{
}

void KernelBase::setOnStarted(std::function<void()> callback)
{
  assert(isEventLoopThread());
  assert(!m_started);
  m_onStarted = std::move(callback);
}

void KernelBase::setOnError(std::function<void()> callback)
{
  assert(isEventLoopThread());
  assert(!m_started);
  m_onError = std::move(callback);
}

void KernelBase::error()
{
  if(!m_onError) /*[[unlikely]]*/
    return;

  if(isEventLoopThread())
  {
    m_onError();
  }
  else
  {
    EventLoop::call(
      [this]()
      {
        m_onError();
      });
  }
}

void KernelBase::started()
{
  if(!m_onStarted) /*[[unlikely]]*/
    return;

  if(isEventLoopThread())
  {
    m_onStarted();
  }
  else
  {
    EventLoop::call(
      [this]()
      {
        m_onStarted();
      });
  }
}
