/**
 * server/src/core/eventloop.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2022-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CORE_EVENTLOOP_HPP
#define TRAINTASTIC_SERVER_CORE_EVENTLOOP_HPP

#include <thread>
#include <boost/asio/io_context.hpp>

class EventLoop
{
  private:
    EventLoop() = default;
    ~EventLoop() = default;

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator =(const EventLoop&) = delete;

  public:
    inline static boost::asio::io_context ioContext;
#ifdef TRAINTASTIC_TEST
    inline static std::thread::id threadId;
#else
    inline static const std::thread::id threadId = std::this_thread::get_id();
#endif

    static void exec()
    {
#ifdef TRAINTASTIC_TEST
      threadId = std::this_thread::get_id();
#endif
      auto work = std::make_shared<boost::asio::io_context::work>(ioContext);
      ioContext.run();
    }

    static void stop()
    {
      ioContext.stop();
    }

    template<typename _Callable, typename... _Args>
    inline static void call(_Callable&& __f, _Args&&... __args)
    {
      ioContext.post(std::bind(__f, __args...));
    }
};

inline bool isEventLoopThread()
{
  return std::this_thread::get_id() == EventLoop::threadId;
}

#endif
