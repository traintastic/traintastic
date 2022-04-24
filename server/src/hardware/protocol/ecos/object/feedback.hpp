/**
 * server/src/hardware/protocol/ecos/object/feedback.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_OBJECT_FEEDBACK_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_OBJECT_FEEDBACK_HPP

#include "object.hpp"
#include "../messages.hpp"
#include "../../../../enum/tristate.hpp"

namespace ECoS {

class Kernel;
struct Line;

class Feedback final : public Object
{
  private:
    std::vector<TriState> m_state;

  protected:
    void update(std::string_view option, std::string_view value) final;

  public:
    static const std::initializer_list<std::string_view> options;

    Feedback(Kernel& kernel, uint16_t id);
    Feedback(Kernel& kernel, const Line& data);

    inline TriState operator [](uint8_t port) const
    {
      assert(port < ports());
      return m_state[port];
    }

    uint8_t ports() const { return static_cast<uint8_t>(m_state.size()); }
};

}

#endif