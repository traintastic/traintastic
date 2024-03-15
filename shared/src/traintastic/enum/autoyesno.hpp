/**
 * shared/src/traintastic/enum/autoyesno.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_AUTOYESNO_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_AUTOYESNO_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"

enum class AutoYesNo : uint8_t
{
  Auto = 0,
  Yes = 1,
  No = 2,
};

TRAINTASTIC_ENUM(AutoYesNo, "auto_yes_no", 3,
{
  {AutoYesNo::Auto, "auto"},
  {AutoYesNo::Yes, "yes"},
  {AutoYesNo::No, "no"},
});

constexpr std::array<AutoYesNo, 3> autoYesNoValues
{
  AutoYesNo::Auto,
  AutoYesNo::Yes,
  AutoYesNo::No,
};

#endif
