/**
 * server/src/os/windows/trayicon.hpp
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

#ifndef TRAINTASTIC_SERVER_OS_WINDOWS_TRAYICON_HPP
#define TRAINTASTIC_SERVER_OS_WINDOWS_TRAYICON_HPP

#include <memory>
#include <thread>
#include <string>
#define _WINSOCKAPI_ // prevent windows.h including winsock.h
#include <windows.h>
#undef _WINSOCKAPI_
//#include <shellapi.h>

namespace Windows {

class TrayIcon
{
  private:
    TrayIcon() = default;

  protected:
    enum class MenuItem : UINT
    {
      Quit = 1,
    };
    
    static constexpr UINT WM_NOTIFYICON_CALLBACK = WM_USER + 1;

    static std::unique_ptr<std::thread> s_thread;
    static HWND s_window;   
    static HMENU s_menu;

    static void run();
    static LRESULT CALLBACK windowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

    static void menuAddItem(MenuItem id, const std::string& text);

  public:
    static void add();
    static void remove();
};

}

#endif
