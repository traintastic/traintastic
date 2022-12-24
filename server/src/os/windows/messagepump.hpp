/**
 * server/src/os/windows/messagepump.hpp
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

#ifndef TRAINTASTIC_SERVER_OS_WINDOWS_MESSAGEPUMP_HPP
#define TRAINTASTIC_SERVER_OS_WINDOWS_MESSAGEPUMP_HPP

#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>
#include <windows.h>

namespace Windows {

class MessagePump
{
  public:
    template<class... A>
    class Callback
    {
      public:
        using CallbackType = std::function<void(A...)>;

      private:
        std::mutex m_mutex;
        CallbackType m_callback;
      
      public:
        void operator ()(A... a)
        {
          std::lock_guard<std::mutex> l{m_mutex};
          if(m_callback)
            m_callback(std::forward<A>(a)...);
        }

        void set(CallbackType callback)
        {
          std::lock_guard<std::mutex> l{m_mutex};
          m_callback = std::move(callback);
        }
    };

    using OnDeviceChangeComPort = Callback<WPARAM, std::string>;

  private:
    static std::atomic_size_t s_useCount;
    static std::thread s_thread;
    static HWND s_window;
    static OnDeviceChangeComPort s_onDeviceChangeComPort;

    MessagePump() = delete;
    MessagePump(const MessagePump&) = delete;
    MessagePump& operator =(const MessagePump&) = delete;

    static LRESULT CALLBACK windowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

  public:
    static void start();
    static void stop();

    inline static void setOnDeviceChangeComPort(OnDeviceChangeComPort::CallbackType callback)
    {
      s_onDeviceChangeComPort.set(std::move(callback));
    }
};

}

#endif
