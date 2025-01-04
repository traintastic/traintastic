/**
 * server/src/os/windows/trayicon.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2024 Reinder Feenstra
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
#include <mutex>
#include <string>
#include <vector>
#define _WINSOCKAPI_ // prevent windows.h including winsock.h
#include <windows.h>
#undef _WINSOCKAPI_

namespace Windows {

class TrayIcon
{
  private:
    TrayIcon() = default;

  protected:
    enum class MenuItem : UINT
    {
      Shutdown = 1,
      Restart = 2,
      ShowHideConsole = 3,
      AllowClientServerRestart = 4,
      AllowClientServerShutdown = 5,
      StartAutomaticallyAtLogon = 6,
      OpenDataDirectory = 7,
      Language = 8, // bit 8..15 are used for language index
    };

    struct TraintasticSettings
    {
      std::mutex mutex;
      bool allowClientServerRestart;
      bool allowClientServerShutdown;
      std::string language;
    };
    
    static constexpr UINT WM_NOTIFYICON_CALLBACK = WM_USER + 1;
    static constexpr UINT WM_TRAINTASTIC_SETTINGS = WM_USER + 2;
    static constexpr UINT WM_TRAINTASTIC_LANGUAGE_CHANGED = WM_USER + 3;

    static std::unique_ptr<std::thread> s_thread;
    static HWND s_window;   
    static HMENU s_menu;
    static HMENU s_menuSettings;
    static HMENU s_menuLanguage;
    static std::vector<std::string> s_languages;
    inline static TraintasticSettings s_settings = {
      {}, 
      false, 
      false, 
      "en-us",
    };

    static constexpr MenuItem menuItemLanguage(uint16_t index)
    {
      return static_cast<MenuItem>(static_cast<UINT>(MenuItem::Language) | (static_cast<UINT>(index) << 8));
    }

    static void run(bool isRestart);
    static LRESULT CALLBACK windowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

    static void menuAddItem(HMENU menu, MenuItem id, std::string_view text, bool enabled = true);
    static void menuAddSeperator(HMENU menu);
    static HMENU menuAddSubMenu(HMENU menu, std::string_view text);
    static bool menuGetItemChecked(HMENU menu, MenuItem id);
    static void menuSetItemChecked(HMENU menu, MenuItem id, bool checked);

    static void getSettings();

  public:
    static void add(bool isRestart = false);
    static void remove();
};

}

#endif
