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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_DINAMOTYPES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_DINAMOTYPES_HPP

#include <cstdint>
#include <string>
#include <string_view>

namespace Dinamo {

struct Version
{
  uint8_t major = 0;
  uint8_t minor = 0;
  uint8_t subRelease = 0;
  uint8_t bugFix = 0;

  auto operator<=>(const Version&) const = default;

  std::string toString() const;
};

struct ProtocolVersion : Version
{
  bool isSupported() const noexcept
  {
    return major == 3;
  }
};

enum class SystemType : uint8_t
{
  RM_H = 1,  //!< RM-H
  RM_U = 2,  //!< RM-U
  RM_C = 3,  //!< RM-C
  UCCI = 10, //!< UCCI or UCCI/E
};

struct System
{
  SystemType type;
  Version version;
};

std::string_view toString(SystemType type);

}

#endif
