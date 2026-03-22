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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_DINAMOPOLARITY_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_DINAMOPOLARITY_HPP

#include <cstdint>

namespace Dinamo {

enum class Polarity : int8_t
{
  Negative = -1,
  Positive = 1,
};

constexpr std::string_view toString(const Dinamo::Polarity value)
{
  switch(value)
  {
    using enum Dinamo::Polarity;

    case Negative:
      return "negative";

    case Positive:
      return "positive";

    default: [[unlikely]]
      return {};
  }
}

}

constexpr Dinamo::Polarity operator~(const Dinamo::Polarity value) noexcept
{
  return static_cast<Dinamo::Polarity>(-static_cast<int8_t>(value));
}

#endif
