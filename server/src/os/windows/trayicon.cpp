/**
 * server/src/os/windows/trayicon.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#include "trayicon.hpp"
#include <cassert>
#include "../../core/eventloop.hpp"
#include "../../core/traintastic.hpp"

namespace Windows {

std::unique_ptr<std::thread> TrayIcon::s_thread;
HWND TrayIcon::s_window = nullptr;
HMENU TrayIcon::s_menu = nullptr;

void TrayIcon::add()
{
  assert(!s_thread);
  s_thread = std::make_unique<std::thread>(run);
}

void TrayIcon::remove()
{
  assert(s_thread);
  PostMessage(s_window, WM_QUIT, 0, 0);
  s_thread->join();
  s_thread.reset();
}

void TrayIcon::run()
{
  const LPCSTR windowClassName = "TraintasticServerTrayIcon";

  // register window class:
  WNDCLASSEX windowClass;
  memset(&windowClass, 0, sizeof(windowClass));
  windowClass.cbSize = sizeof(windowClass);
  windowClass.lpfnWndProc = windowProc;
  windowClass.hInstance = GetModuleHandle(nullptr);
  windowClass.lpszClassName = windowClassName;
  if(!RegisterClassExA(&windowClass)) 
    return;

  // create window (for receiving messages):
  s_window = CreateWindowExA(0, windowClassName, nullptr, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  if(!s_window)
    return;
  
  UpdateWindow(s_window);

  // create menu:
  s_menu = CreatePopupMenu();
  menuAddItem(MenuItem::Quit, "Quit");

  // setup tray icon:
  static NOTIFYICONDATA notifyIconData;
  memset(&notifyIconData, 0, sizeof(notifyIconData));
  notifyIconData.cbSize = sizeof(notifyIconData);
  notifyIconData.hWnd = s_window;
  notifyIconData.uID = 0;
  notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE;
  notifyIconData.uCallbackMessage = WM_NOTIFYICON_CALLBACK; 
  notifyIconData.hIcon = static_cast<HICON>(LoadImageA(GetModuleHandleA(nullptr), "APPICON", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
  Shell_NotifyIcon(NIM_ADD, &notifyIconData);

  // message loop:
  while(true)
  {
    MSG msg;
    GetMessage(&msg, NULL, 0, 0);
    if(msg.message == WM_QUIT)
      break;
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // remove tray icon:
  Shell_NotifyIcon(NIM_DELETE, &notifyIconData);
  
  if(notifyIconData.hIcon)
    DestroyIcon(notifyIconData.hIcon);
  
  if(s_menu)
    DestroyMenu(s_menu);
  
  UnregisterClass(windowClassName, GetModuleHandle(nullptr));
}

LRESULT CALLBACK TrayIcon::windowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  switch(uMsg)
  {
    case WM_CLOSE:
      DestroyWindow(hWnd);
      return 0;
    
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  
    case WM_NOTIFYICON_CALLBACK:
      if(lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP)
      {
        POINT p;
        GetCursorPos(&p);
        SetForegroundWindow(hWnd);
        WORD cmd = TrackPopupMenu(s_menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, p.x, p.y, 0, hWnd, NULL);
        SendMessage(hWnd, WM_COMMAND, cmd, 0);
        return 0;
      }
      break;

  case WM_COMMAND:
    switch(static_cast<MenuItem>(wParam))
    {
      case MenuItem::Quit:
        EventLoop::call(
          []()
          {
            Traintastic::instance->exit();
          });
        return 0;
    }
    break;
  }
  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void TrayIcon::menuAddItem(MenuItem id, const std::string& text)
{
  assert(s_menu);
  MENUITEMINFO item;
  memset(&item, 0, sizeof(item));
  item.cbSize = sizeof(item);
  item.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE;
  item.fType = 0;
  item.fState = 0;
  item.wID = static_cast<UINT>(id);
  item.dwTypeData = const_cast<LPSTR>(text.c_str());
  InsertMenuItem(s_menu, 0, TRUE, &item);
}

}
