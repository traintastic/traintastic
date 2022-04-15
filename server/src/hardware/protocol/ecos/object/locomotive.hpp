/**
 * server/src/hardware/protocol/ecos/object/locomotive.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_OBJECT_LOCOMOTIVE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_OBJECT_LOCOMOTIVE_HPP

#include "object.hpp"
#include <map>
#include <traintastic/enum/tristate.hpp>
#include "locomotiveprotocol.hpp"
#include "../messages.hpp"
#include "../../../../enum/direction.hpp"

namespace ECoS {

class Kernel;
struct Line;

class Locomotive final : public Object
{
  private:
    struct Function
    {
      bool value = false;
    };

    bool m_control = false;
    uint16_t m_address = 0;
    LocomotiveProtocol m_protocol = LocomotiveProtocol::Unknown;
    uint8_t m_speedStep = 0;
    Direction m_direction = Direction::Forward;
    std::map<uint8_t, Function> m_functions;

    void requestControl();

  protected:
    void update(std::string_view option, std::string_view value) final;

  public:
    static const std::initializer_list<std::string_view> options;

    static constexpr uint8_t getSpeedSteps(LocomotiveProtocol protocol);
    static constexpr uint8_t getFunctionCount(LocomotiveProtocol protocol);

    Locomotive(Kernel& kernel, uint16_t id);
    Locomotive(Kernel& kernel, const Line& data);

    bool receiveReply(const Reply& reply) final;
    bool receiveEvent(const Event& event) final;

    uint16_t address() const { return m_address; }
    LocomotiveProtocol protocol() const { return m_protocol; }

    void stop();

    uint8_t speedStep() const { return m_speedStep; }
    void setSpeedStep(uint8_t value);
    uint8_t speedSteps() const { return getSpeedSteps(m_protocol); }

    Direction direction() const { return m_direction; }
    void setDirection(Direction value);

    uint8_t getFunctionCount() const { return getFunctionCount(m_protocol); }
    TriState getFunctionValue(uint8_t index) const;
    void setFunctionValue(uint8_t index, bool value);
};

constexpr uint8_t Locomotive::getSpeedSteps(LocomotiveProtocol protocol)
{
  switch(protocol)
  {
    case LocomotiveProtocol::DCC14:
    case LocomotiveProtocol::MM14:
      return 14;

    case LocomotiveProtocol::MM27:
      return 27;

    case LocomotiveProtocol::DCC28:
    case LocomotiveProtocol::MM28:
      return 28;

    case LocomotiveProtocol::SX32:
      return 32;

    case LocomotiveProtocol::DCC128:
      return 128;

    case LocomotiveProtocol::MMFKT:
      return 0; // ??

    case LocomotiveProtocol::Unknown:
      break;
  }
  return 0;
}

constexpr uint8_t Locomotive::getFunctionCount(LocomotiveProtocol protocol)
{
  switch(protocol)
  {
    case LocomotiveProtocol::DCC14:
    case LocomotiveProtocol::DCC28:
    case LocomotiveProtocol::DCC128:
      return 29;

    case LocomotiveProtocol::MM14:
    case LocomotiveProtocol::MM27:
    case LocomotiveProtocol::MM28:
    case LocomotiveProtocol::SX32:
    case LocomotiveProtocol::MMFKT:
      return 9; // ??

    case LocomotiveProtocol::Unknown:
      break;
  }
  return 0;
}

}

#endif