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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_CBUS_CBUSSESSIONLIST_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_CBUS_CBUSSESSIONLIST_HPP

#include "../../../core/subobject.hpp"
#include "../../../core/table.hpp"
#include <optional>
#include <traintastic/enum/direction.hpp>
#include <traintastic/enum/tristate.hpp>

class CBUSSessionListTableModel;

class CBUSSessionList : public SubObject, public Table
{
  CLASS_ID("cbus_session_list")

  friend class CBUSInterface;
  friend class CBUSSessionListTableModel;

public:
  struct Session
  {
    std::optional<uint8_t> session;
    uint16_t address;
    bool isLongAddress;
    std::optional<bool> external;
    std::optional<uint8_t> speed;
    Direction direction = Direction::Unknown;
    std::array<TriState, 29> functions;
    uint8_t steps = 128;

    Session(uint8_t session_, bool external_, uint16_t address_, bool isLongAddress_)
      : session{session_}
      , address{address_}
      , isLongAddress{isLongAddress_}
      , external{external_}
    {
      std::fill(functions.begin(), functions.end(), TriState::Undefined);
    }
  };

  using Sessions = std::vector<Session>;
  using const_iterator = Sessions::const_iterator;
  using iterator = Sessions::iterator;

  CBUSSessionList(Object& parent_, std::string_view parentPropertyName);

  TableModelPtr getModel() final;

  const_iterator begin() const noexcept { return m_sessions.begin(); }
  iterator begin() noexcept { return m_sessions.begin(); }
  const_iterator end() const noexcept { return m_sessions.end(); }
  iterator end() noexcept { return m_sessions.end(); }

  iterator findEngine(uint16_t address, bool isLongAddress);
  iterator findSession(uint8_t session);

private:
  Sessions m_sessions;
  std::vector<CBUSSessionListTableModel*> m_models;

  void add(Session&& session);
  void clear();

  void rowChanged(uint32_t row);
};

#endif
