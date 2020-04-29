/**
 * server/src/core/eventloop.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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
#include <thread>
#include <condition_variable>
#include <functional>

#include <iostream>

class EventLoop
{
  private:
    inline static EventLoop* s_instance;
    std::queue<std::function<void()>> m_queue;
    std::mutex m_queueMutex;
    std::condition_variable m_condition;
    bool m_run;
    std::thread m_thread;

    EventLoop() :
      m_run{true},
      m_thread(&EventLoop::run, this)
    {
    }

    EventLoop(const EventLoop&) = delete;

    ~EventLoop()
    {
    }

    void add(std::function<void()>&& f)
    {
      std::lock_guard<std::mutex> lock(m_queueMutex);
      m_queue.emplace(f);
      m_condition.notify_one();
    }

    void run()
    {
      std::unique_lock<std::mutex> lock(m_queueMutex);

      while(m_run)
      {
        if(m_queue.empty())
          m_condition.wait(lock, [this]{ return !m_queue.empty(); });

        if(m_queue.empty())
          continue; // a suspisius wakeup may occur

        std::function<void()>& f{m_queue.front()};

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

        m_queue.pop();
      }
    }

    void exit()
    {
      add([this](){ m_run = false; });
      m_thread.join();
    }

  public:
    static void start()
    {
      s_instance = new EventLoop();
    }

    static void stop()
    {
      s_instance->exit();
      delete s_instance;
      s_instance = nullptr;
    }

    template<typename _Callable, typename... _Args>
    inline static void call(_Callable&& __f, _Args&&... __args)
    {
      s_instance->add(std::bind(__f, __args...));
    }
};

#endif
