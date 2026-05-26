/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "cbussessionlisttablemodel.hpp"
#include "cbussessionlist.hpp"
#include "../../../compat/stdformat.hpp"
#include "../../../utils/utf8.hpp"

constexpr uint32_t columnAddress = 0;
constexpr uint32_t columnLong = 1;
constexpr uint32_t columnSession = 2;
constexpr uint32_t columnControlledBy = 3;
constexpr uint32_t columnSpeed = 4;
constexpr uint32_t columnDirection = 5;
constexpr uint32_t columnF0 = 6;
constexpr uint32_t columnF1 = 7;
constexpr uint32_t columnF2 = 8;
constexpr uint32_t columnF3 = 9;
constexpr uint32_t columnF4 = 10;
constexpr uint32_t columnF5 = 11;
constexpr uint32_t columnF6 = 12;
constexpr uint32_t columnF7 = 13;
constexpr uint32_t columnF8 = 14;
constexpr uint32_t columnF9 = 15;
constexpr uint32_t columnF10 = 16;
constexpr uint32_t columnF11 = 17;
constexpr uint32_t columnF12 = 18;
constexpr uint32_t columnF13 = 19;
constexpr uint32_t columnF14 = 20;
constexpr uint32_t columnF15 = 21;
constexpr uint32_t columnF16 = 22;
constexpr uint32_t columnF17 = 23;
constexpr uint32_t columnF18 = 24;
constexpr uint32_t columnF19 = 25;
constexpr uint32_t columnF20 = 26;
constexpr uint32_t columnF21 = 27;
constexpr uint32_t columnF22 = 28;
constexpr uint32_t columnF23 = 29;
constexpr uint32_t columnF24 = 30;
constexpr uint32_t columnF25 = 31;
constexpr uint32_t columnF26 = 32;
constexpr uint32_t columnF27 = 33;
constexpr uint32_t columnF28 = 34;
constexpr uint32_t columnSteps = 35;

CBUSSessionListTableModel::CBUSSessionListTableModel(CBUSSessionList& list)
  : m_list{list.shared_ptr<CBUSSessionList>()}
{
  assert(m_list);
  m_list->m_models.push_back(this);

  setColumnHeaders({
    std::string_view{"hardware:address"},
    std::string_view{"cbus_session_list:long"},
    std::string_view{"cbus_session_list:session"},
    std::string_view{"cbus_session_list:controlled_by"},
    std::string_view{"cbus_session_list:speed"},
    std::string_view{"cbus_session_list:direction"},
    std::string_view{"function:f0"},
    std::string_view{"function:f1"},
    std::string_view{"function:f2"},
    std::string_view{"function:f3"},
    std::string_view{"function:f4"},
    std::string_view{"function:f5"},
    std::string_view{"function:f6"},
    std::string_view{"function:f7"},
    std::string_view{"function:f8"},
    std::string_view{"function:f9"},
    std::string_view{"function:f10"},
    std::string_view{"function:f11"},
    std::string_view{"function:f12"},
    std::string_view{"function:f13"},
    std::string_view{"function:f14"},
    std::string_view{"function:f15"},
    std::string_view{"function:f16"},
    std::string_view{"function:f17"},
    std::string_view{"function:f18"},
    std::string_view{"function:f19"},
    std::string_view{"function:f20"},
    std::string_view{"function:f21"},
    std::string_view{"function:f22"},
    std::string_view{"function:f23"},
    std::string_view{"function:f24"},
    std::string_view{"function:f25"},
    std::string_view{"function:f26"},
    std::string_view{"function:f27"},
    std::string_view{"function:f28"},
    std::string_view{"hardware:speed_steps"}
  });

  setRowCount(m_list->m_sessions.size());
}

CBUSSessionListTableModel::~CBUSSessionListTableModel()
{
  auto it = std::find(m_list->m_models.begin(), m_list->m_models.end(), this);
  assert(it != m_list->m_models.end());
  m_list->m_models.erase(it);
}

std::string CBUSSessionListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < m_list->m_sessions.size())
  {
    const auto& session = m_list->m_sessions[row];

    switch(column)
    {

      case columnAddress:
        return std::to_string(session.address);

      case columnLong:
        return session.isLongAddress ? UTF8_CHECKMARK : "";

      case columnSession:
        if(session.session)
        {
          return std::to_string(*session.session);
        }
        break;

      case columnControlledBy:
        if(session.external)
        {
          return *session.external ? "CBUS/VLCB" : "Traintastic";
        }
        break;

      case columnSpeed:
        if(session.speed)
        {
          if(*session.speed == 0)
          {
            return "$throttle.stop$";
          }
          if(*session.speed == 1)
          {
            return "$throttle.estop$";
          }
          return std::to_string(*session.speed - 1);
        }
        break;

      case columnDirection:
        if(session.direction != Direction::Unknown)
        {
          if(const auto* it = EnumValues<Direction>::value.find(session.direction); it != EnumValues<Direction>::value.end()) [[likely]]
          {
            return std::format("${}:{}$", EnumName<Direction>::value, it->second);
          }
        }
        break;

      case columnF0:
      case columnF1:
      case columnF2:
      case columnF3:
      case columnF4:
      case columnF5:
      case columnF6:
      case columnF7:
      case columnF8:
      case columnF9:
      case columnF10:
      case columnF11:
      case columnF12:
      case columnF13:
      case columnF14:
      case columnF15:
      case columnF16:
      case columnF17:
      case columnF18:
      case columnF19:
      case columnF20:
      case columnF21:
      case columnF22:
      case columnF23:
      case columnF24:
      case columnF25:
      case columnF26:
      case columnF27:
      case columnF28:
        switch(session.functions[column - columnF0])
        {
          using enum TriState;

          case False:
            return "$function:off$";

          case True:
            return "$function:on$";

          case Undefined:
            break;
        };
        break;

      case columnSteps:
        return std::to_string(session.steps);
    }
  }
  return {};
}
