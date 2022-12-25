/**
 * server/src/os/windows/messagepump.cpp
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

#include "messagepump.hpp"
#include <cassert>
#include <dbt.h>
#include "../../utils/setthreadname.hpp"

namespace Windows {

std::atomic_size_t MessagePump::s_useCount{0};
std::thread MessagePump::s_thread;
HWND MessagePump::s_window = nullptr;
MessagePump::OnDeviceChangeComPort MessagePump::s_onDeviceChangeComPort;

static const auto WM_STOP = WM_USER;

void MessagePump::start()
{
  if(s_useCount++ != 0)
    return; // already running

  HANDLE event = CreateEventA(nullptr, false, false, nullptr);

  s_thread = std::thread{
    [event]()
    {
      setThreadName("messagepump");

      const LPCSTR windowClassName = "TraintasticServerMessagePump";

      WNDCLASSEX windowClass;
      memset(&windowClass, 0, sizeof(windowClass));
      windowClass.cbSize = sizeof(windowClass);
      windowClass.lpfnWndProc = windowProc;
      windowClass.hInstance = GetModuleHandle(nullptr);
      windowClass.lpszClassName = windowClassName;
      if(RegisterClassExA(&windowClass))
      {
        // create window (for receiving messages):
        s_window = CreateWindowExA(0, windowClassName, nullptr, 0, 0, 0, 0, 0, 0, 0, 0, 0);
      }

      SetEvent(event);

      if(!s_window)
        return;

      // message pump:
      while(true)
      {
        MSG msg;
        if(!GetMessageA(&msg, nullptr, 0, 0) || msg.message == WM_STOP)
          break;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }

      DestroyWindow(s_window);
      s_window = nullptr;

      UnregisterClassA(windowClassName, windowClass.hInstance);
    }};

  // wait for window handle to be allocated, stopping requires a window handle
  WaitForSingleObject(event, INFINITE);
  CloseHandle(event);
}

void MessagePump::stop()
{
  assert(s_useCount > 0);
  if(--s_useCount != 0)
    return; // don't stop

  PostMessage(s_window, WM_STOP, 0, 0);

  s_thread.join();
}

LRESULT CALLBACK MessagePump::windowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  switch(uMsg)
  {
    case WM_CREATE:
    {
      DEV_BROADCAST_DEVICEINTERFACE notificationFilter;
      ZeroMemory(&notificationFilter, sizeof(notificationFilter));
      notificationFilter.dbcc_size = sizeof(notificationFilter);
      notificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
      notificationFilter.dbcc_classguid = GUID_DEVINTERFACE_COMPORT;
      RegisterDeviceNotification(hWnd, &notificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
      break;
    }
    case WM_DEVICECHANGE:
      if(auto b = reinterpret_cast<PDEV_BROADCAST_DEVICEINTERFACE>(lParam))
      {
        if(IsEqualGUID(b->dbcc_classguid, GUID_DEVINTERFACE_COMPORT))
        {
          s_onDeviceChangeComPort(wParam, b->dbcc_name);
        }
      }
      break;
  }
  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

}
