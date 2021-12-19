/**
 * server/src/hardware/protocol/ecos/object/locomotive.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_OBJECT_LOCOMOTIVE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_OBJECT_LOCOMOTIVE_HPP

#include "object.hpp"
#include "../messages.hpp"

namespace ECoS {

class Kernel;

class Locomotive final : public Object
{
  public:
    enum class Protocol
    {
      Unknown = 0,
      MM14 = 1,
      MM27 = 2,
      MM28 = 3,
      DCC14 = 4,
      DCC28 = 5,
      DCC128 = 6,
      SX32 = 7,
      MMFKT = 8,
    };

  private:
    Protocol m_protocol = Protocol::Unknown;

  public:
    static const std::initializer_list<std::string_view> options;

    Locomotive(Kernel& kernel, uint16_t id);

    bool receiveReply(const Reply& reply) final;
    bool receiveEvent(const Event& event) final;

    Protocol protocol() const { return m_protocol; }
};

}

#endif