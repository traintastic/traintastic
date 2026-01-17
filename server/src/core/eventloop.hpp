/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2019-2026 Reinder Feenstra
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

    inline static std::unique_ptr<boost::asio::io_context> s_ioContext;
    inline static std::shared_ptr<boost::asio::io_context::work> s_keepAlive;

  public:
#ifdef TRAINTASTIC_TEST
    inline static std::thread::id threadId;
#else
    inline static const std::thread::id threadId = std::this_thread::get_id();
#endif

    static boost::asio::io_context& ioContext()
    {
      assert(s_ioContext);
      return *s_ioContext;
    }

    static void reset()
    {
      s_ioContext = std::make_unique<boost::asio::io_context>();
    }

    static void exec()
    {
#ifdef TRAINTASTIC_TEST
      threadId = std::this_thread::get_id();
#endif
      s_keepAlive = std::make_shared<boost::asio::io_context::work>(ioContext());
      ioContext().run();
      s_keepAlive.reset();
    }

    static void stop()
    {
      s_keepAlive.reset();
    }

    template<typename _Callable, typename... _Args>
    inline static void call(_Callable&& __f, _Args&&... __args)
    {
      ioContext().post(std::bind(__f, __args...));
    }

    template<typename T>
    inline static void deleteLater(T* object)
    {
      ioContext().post(
        [object]()
        {
          delete object;
        });
    }
};

inline bool isEventLoopThread()
{
  return std::this_thread::get_id() == EventLoop::threadId;
}

#endif
