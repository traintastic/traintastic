/**
 * server/src/os/windows/trayicon.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2023 Reinder Feenstra
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
#include <version.hpp>
#include "consolewindow.hpp"
#include "registry.hpp"
#include "../../core/eventloop.hpp"
#include "../../core/objectproperty.tpp"
#include "../../traintastic/traintastic.hpp"
#include "../../utils/setthreadname.hpp"

namespace Windows {

std::unique_ptr<std::thread> TrayIcon::s_thread;
HWND TrayIcon::s_window = nullptr;
HMENU TrayIcon::s_menu = nullptr;
HMENU TrayIcon::s_menuSettings = nullptr;

void TrayIcon::add(bool isRestart)
{
  assert(!s_thread);
  s_thread = std::make_unique<std::thread>(run, isRestart);
}

void TrayIcon::remove()
{
  assert(s_thread);
  PostMessage(s_window, WM_QUIT, 0, 0);
  s_thread->join();
  s_thread.reset();
}

void TrayIcon::run(bool isRestart)
{
  setThreadName("trayicon");

  const LPCSTR windowClassName = "TraintasticServerTrayIcon";

  // register window class, once:
  if(!isRestart)
  {
    WNDCLASSEX windowClass;
    memset(&windowClass, 0, sizeof(windowClass));
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = GetModuleHandle(nullptr);
    windowClass.lpszClassName = windowClassName;
    if(!RegisterClassExA(&windowClass))
      return;
  }

  // create window (for receiving messages):
  s_window = CreateWindowExA(0, windowClassName, nullptr, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  if(!s_window)
    return;

  UpdateWindow(s_window);

  // create menu:
  s_menu = CreatePopupMenu();
  menuAddItem(s_menu, MenuItem::ShowHideConsole, "Show/hide console", hasConsoleWindow());
  menuAddSeperator(s_menu);
  
  s_menuSettings = menuAddSubMenu(s_menu, "Settings");
  menuAddItem(s_menuSettings, MenuItem::AllowClientServerRestart, "Allow client to restart server");
  menuAddItem(s_menuSettings, MenuItem::AllowClientServerShutdown, "Allow client to shutdown server");
  menuAddSeperator(s_menuSettings);
  menuAddItem(s_menuSettings, MenuItem::StartAutomaticallyAtLogon, "Start automatically at logon");  
  
  HMENU menuAdvanced = menuAddSubMenu(s_menu, "Advanced");
  menuAddItem(menuAdvanced, MenuItem::OpenDataDirectory, "Open data directory");

  menuAddSeperator(s_menu);
  menuAddItem(s_menu, MenuItem::Restart, "Restart");
  menuAddItem(s_menu, MenuItem::Shutdown, "Shutdown");

  bool startUpApproved = false;
  Registry::getStartUpApproved(startUpApproved);
  menuSetItemChecked(s_menuSettings, MenuItem::StartAutomaticallyAtLogon, startUpApproved);

  // setup tray icon:
  static NOTIFYICONDATA notifyIconData;
  memset(&notifyIconData, 0, sizeof(notifyIconData));
  notifyIconData.cbSize = sizeof(notifyIconData);
  notifyIconData.hWnd = s_window;
  notifyIconData.uID = 0;
  notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP;
  notifyIconData.uCallbackMessage = WM_NOTIFYICON_CALLBACK;
  notifyIconData.hIcon = static_cast<HICON>(LoadImageA(GetModuleHandleA(nullptr), "APPICON", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
  const std::string_view toolTip{"Traintastic server v" TRAINTASTIC_VERSION_FULL};
  std::memcpy(notifyIconData.szTip, toolTip.data(), std::min(toolTip.size(), sizeof(notifyIconData.szTip) - 1));

  const std::string_view infoTitle{"Traintastic server"};
  const std::string_view infoMessage{"Traintastic server is running in the system tray."};
  const std::string_view infoMessageRestarted{"Traintastic server restarted"};
  std::memcpy(notifyIconData.szInfoTitle, infoTitle.data(), std::min(infoTitle.size(), sizeof(notifyIconData.szInfoTitle) - 1));
  if(isRestart)
    std::memcpy(notifyIconData.szInfo, infoMessageRestarted.data(), std::min(infoMessageRestarted.size(), sizeof(notifyIconData.szInfo) - 1));
  else
    std::memcpy(notifyIconData.szInfo, infoMessage.data(), std::min(infoMessage.size(), sizeof(notifyIconData.szInfo) - 1));
  notifyIconData.dwInfoFlags = NIIF_INFO | NIIF_LARGE_ICON;
  notifyIconData.uFlags |= NIF_INFO;

  Shell_NotifyIcon(NIM_ADD, &notifyIconData);

  // Get settings:
  EventLoop::call(getSettings);

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
      case MenuItem::Shutdown:
        EventLoop::call(
          []()
          {
            Traintastic::instance->exit();
          });
        return 0;

      case MenuItem::Restart:
        EventLoop::call(
          []()
          {
            Traintastic::instance->restart();
          });
        return 0;

      case MenuItem::ShowHideConsole:
        setConsoleWindowVisible(!isConsoleWindowVisible());
        return 0;

      case MenuItem::AllowClientServerRestart:
      {
        bool value;
        {
          std::lock_guard lock{s_settings.mutex};
          value = !s_settings.allowClientServerRestart;
        }
        EventLoop::call(
          [value]()
          {
            Traintastic::instance->settings->allowClientServerRestart = value;
            getSettings();
          });
        break;
      }
      case MenuItem::AllowClientServerShutdown:
      {
        bool value;
        {
          std::lock_guard lock{s_settings.mutex};
          value = !s_settings.allowClientServerShutdown;
        }
        EventLoop::call(
          [value]()
          {
            Traintastic::instance->settings->allowClientServerShutdown = value;
            getSettings();
          });
        break;
      }
      case MenuItem::StartAutomaticallyAtLogon:
      {
        bool startUpApproved = !menuGetItemChecked(s_menuSettings, MenuItem::StartAutomaticallyAtLogon);
        if(startUpApproved)
          Registry::addRun();
        Registry::setStartUpApproved(startUpApproved);
        if(Registry::getStartUpApproved(startUpApproved))
          menuSetItemChecked(s_menuSettings, MenuItem::StartAutomaticallyAtLogon, startUpApproved);
        break;
      }
      case MenuItem::OpenDataDirectory:
      {
        const auto dataDir = Traintastic::instance->dataDir().string();
        ShellExecuteA(nullptr, "open", dataDir.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
        break;
      }
    }
    break;

    case WM_TRAINTASTIC_SETTINGS:
    {
      std::lock_guard lock{s_settings.mutex};
      menuSetItemChecked(s_menuSettings, MenuItem::AllowClientServerRestart, s_settings.allowClientServerRestart);
      menuSetItemChecked(s_menuSettings, MenuItem::AllowClientServerShutdown, s_settings.allowClientServerShutdown);
      break;
    }
  }
  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void TrayIcon::menuAddItem(HMENU menu, MenuItem id, const LPCSTR text, bool enabled)
{
  assert(menu);
  MENUITEMINFO item;
  memset(&item, 0, sizeof(item));
  item.cbSize = sizeof(item);
  item.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE;
  item.fType = MFT_STRING;
  item.fState = enabled ? MFS_ENABLED : MFS_DISABLED;
  item.wID = static_cast<UINT>(id);
  item.dwTypeData = const_cast<LPSTR>(text);
  int n = GetMenuItemCount(menu);
  InsertMenuItem(menu, n, TRUE, &item);
}

void TrayIcon::menuAddSeperator(HMENU menu)
{
  assert(menu);
  MENUITEMINFO item;
  memset(&item, 0, sizeof(item));
  item.cbSize = sizeof(item);
  item.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE;
  item.fType = MFT_SEPARATOR;
  InsertMenuItem(menu, GetMenuItemCount(menu), TRUE, &item);
}

HMENU TrayIcon::menuAddSubMenu(HMENU menu, const LPCSTR text)
{
  assert(menu);
  HMENU subMenu = CreatePopupMenu();
  MENUITEMINFO item;
  memset(&item, 0, sizeof(item));
  item.cbSize = sizeof(item);
  item.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE | MIIM_SUBMENU;
  item.fType = MFT_STRING;
  item.fState = MFS_ENABLED;
  item.hSubMenu = subMenu;
  item.dwTypeData = const_cast<LPSTR>(text);
  InsertMenuItem(menu, GetMenuItemCount(menu), TRUE, &item);
  return subMenu;
}

bool TrayIcon::menuGetItemChecked(HMENU menu, MenuItem id)
{
  assert(menu);
  return GetMenuState(menu, static_cast<UINT>(id), MF_BYCOMMAND) & MF_CHECKED;
}

void TrayIcon::menuSetItemChecked(HMENU menu, MenuItem id, bool checked)
{
  assert(menu);
  CheckMenuItem(menu, static_cast<UINT>(id), checked ? MF_CHECKED : MF_UNCHECKED);
}

void TrayIcon::getSettings()
{
  assert(isEventLoopThread());
  const auto& settings = *Traintastic::instance->settings.value();
  {
    std::lock_guard lock{s_settings.mutex};
    s_settings.allowClientServerRestart = settings.allowClientServerRestart;
    s_settings.allowClientServerShutdown = settings.allowClientServerShutdown;
  }
  PostMessage(s_window, WM_TRAINTASTIC_SETTINGS, 0, 0);
}

}
