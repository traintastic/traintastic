/**
 * server/src/core/eventloop.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_EVENTLOOP_HPP
#define TRAINTASTIC_SERVER_CORE_EVENTLOOP_HPP

#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include "../utils/setthreadname.hpp"

class EventLoop
{
  private:
    inline static std::queue<std::function<void()>> s_queue;
    inline static std::mutex s_queueMutex;
    inline static std::condition_variable s_condition;
    inline static std::atomic<bool> s_run;

    EventLoop() = default;
    ~EventLoop() = default;

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator =(const EventLoop&) = delete;

  public:
    inline static const std::thread::id threadId = std::this_thread::get_id();

    static void exec()
    {
      std::unique_lock<std::mutex> lock(s_queueMutex);

      s_run = true;
      while(s_run)
      {
        if(s_queue.empty())
          s_condition.wait(lock, []{ return !s_queue.empty(); });

        if(s_queue.empty())
          continue; // a suspisius wakeup may occur

        std::function<void()>& f{s_queue.front()};

        lock.unlock();

        try
        {
          f();
        }
        catch(const std::exception& e)
        {
          std::cerr << e.what() << std::endl;
        }

        lock.lock();

        s_queue.pop();
      }
    }

    static void stop()
    {
      s_run = false;
    }

    template<typename _Callable, typename... _Args>
    inline static void call(_Callable&& __f, _Args&&... __args)
    {
      std::lock_guard<std::mutex> lock(s_queueMutex);
      s_queue.emplace(std::bind(__f, __args...));
      s_condition.notify_one();
    }
};

inline bool isEventLoopThread()
{
  return std::this_thread::get_id() == EventLoop::threadId;
}

#endif
