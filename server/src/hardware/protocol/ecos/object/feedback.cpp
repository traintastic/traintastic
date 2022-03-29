/**
 * server/src/hardware/protocol/ecos/object/feedback.cpp
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

#include "feedback.hpp"
#include <cassert>
#include "../kernel.hpp"
#include "../messages.hpp"
#include "../../../../utils/fromchars.hpp"
#include "../../../../utils/startswith.hpp"

namespace ECoS {

const std::initializer_list<std::string_view> Feedback::options = {Option::ports};

Feedback::Feedback(Kernel& kernel, uint16_t id)
  : Object(kernel, id)
{
  requestView();
  send(get(m_id, {Option::state}));
}

Feedback::Feedback(Kernel& kernel, const Line& data)
  : Feedback(kernel, data.objectId)
{
  const auto values = data.values;
  if(auto ports = values.find(Option::ports); ports != values.end())
  {
    size_t n;
    if(fromChars(ports->second, n).ec == std::errc())
      m_state.resize(n, TriState::Undefined);
  }
}

void Feedback::update(std::string_view option, std::string_view value)
{
  if(option == Option::state)
  {
    uint32_t n;
    if(startsWith(value, "0x") && fromChars(value.substr(2), n, 16).ec == std::errc())
    {
      for(size_t i = 0; i < m_state.size(); i++)
      {
        const auto v = toTriState(n & (1 << i));
        if(m_state[i] != v)
        {
          m_state[i] = v;
          m_kernel.feedbackStateChanged(*this, i, v);
        }
      }
    }
  }
}

}
