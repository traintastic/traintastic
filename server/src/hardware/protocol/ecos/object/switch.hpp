/**
 * server/src/hardware/protocol/ecos/object/switch.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_OBJECT_SWITCH_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_OBJECT_SWITCH_HPP

#include "object.hpp"
#include "../messages.hpp"

namespace ECoS {

class Kernel;
struct Line;

class Switch final : public Object
{
  public:
    enum class Protocol
    {
      Unknown = 0,
      DCC = 1,
      MM = 2,
    };

  private:
    uint16_t m_address = 0;
    Protocol m_protocol = Protocol::Unknown;

  public:
    static const std::initializer_list<std::string_view> options;

    Switch(Kernel& kernel, uint16_t id);
    Switch(Kernel& kernel, const Line& data);

    bool receiveReply(const Reply& reply) final;
    bool receiveEvent(const Event& event) final;

    uint16_t address() const { return m_address; }
    Protocol protocol() const { return m_protocol; }
};

}

#endif