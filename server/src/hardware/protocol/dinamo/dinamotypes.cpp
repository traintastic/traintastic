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

#include "dinamotypes.hpp"
#include <format>

namespace Dinamo {

std::string Version::toString() const
{
  auto s = std::format("{}.{}{}", major, minor, subRelease);
  if(bugFix != 0)
  {
    s += ('A' + bugFix - 1);
  }
  return s;
}

std::string_view toString(SystemType type)
{
  switch(type)
  {
    using enum SystemType;

    case RM_H:
      return "RM-H";

    case RM_U:
      return "RM-U";

    case RM_C:
      return "RM-C";

    case UCCI:
      return "UCCI";
  }
  return {};
}

}
