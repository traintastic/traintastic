/**
 * server/src/hardware/protocol/ecos/object/switch.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_OBJECT_SWITCH_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_OBJECT_SWITCH_HPP

#include "object.hpp"
#include "switchprotocol.hpp"
#include "switchtype.hpp"
#include "../messages.hpp"

enum class OutputPairValue : uint8_t;

namespace ECoS {

class Kernel;
struct Line;

class Switch final : public Object
{
  public:
    using AddrExt = std::vector<std::pair<uint16_t, OutputPairValue>>;

    enum class Mode
    {
      Unknown = 0,
      Switch = 1,
      Pulse = 2,
    };

    enum class Symbol
    {
      None = -1,
      TurnoutLeft = 0,
      TurnoutRight = 1,
      Turnout3Way = 2,
      SemaforeDBHp0Hp1Hp2 = 7,
    };

  private:
    std::string m_name1;
    std::string m_name2;
    std::string m_name3;
    SwitchType m_type = SwitchType::Unknown;
    uint16_t m_address = 0;
    AddrExt m_addrext;
    Symbol m_symbol = Symbol::None;
    SwitchProtocol m_protocol = SwitchProtocol::Unknown;
    uint8_t m_state = 0;
    Mode m_mode = Mode::Unknown;
    uint16_t m_duration = 0;
    uint8_t m_variant = 0;

  protected:
    void update(std::string_view option, std::string_view value) final;

  public:
    Switch(Kernel& kernel, uint16_t id);

    bool receiveReply(const Reply& reply) final;
    bool receiveEvent(const Event& event) final;

    std::string nameUI() const;
    const std::string& name1() const { return m_name1; }
    const std::string& name2() const { return m_name2; }
    const std::string& name3() const { return m_name3; }
    SwitchType type() const { return m_type; }
    uint16_t address() const { return m_address; }
    const AddrExt& addrext() const { return m_addrext; }
    Symbol symbol() const { return m_symbol; }
    SwitchProtocol protocol() const { return m_protocol; }
    uint8_t state() const { return m_state; }
    Mode mode() const { return m_mode; }
    uint16_t duration() const { return m_duration; }
    uint8_t variant() const { return m_state; }

    void setState(uint8_t value);
};

std::string toString(const Switch::AddrExt& values);

constexpr std::string_view toString(Switch::Mode value)
{
  switch(value)
  {
    case Switch::Mode::Switch:
      return "SWITCH";

    case Switch::Mode::Pulse:
      return "PULSE";

    case Switch::Mode::Unknown:
      break;
  }
  return {};
}

}

#endif