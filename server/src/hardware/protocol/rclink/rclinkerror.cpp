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

#include "rclinkerror.hpp"

namespace {

class Category final : public std::error_category
{
public:
  const char* name() const noexcept override
  {
    return "rclink";
  }

  std::string message(int ev) const override
  {
    switch(static_cast<RCLink::Errc>(ev))
    {
      using enum RCLink::Errc;

      case BufferFull:
        return "Buffer full";

      default:
        return "Unknown RCLink error";
    }
  }

  std::error_condition default_error_condition(int ev) const noexcept override
  {
    switch(static_cast<RCLink::Errc>(ev))
    {
      using enum RCLink::Errc;

      case BufferFull:
        return std::errc::no_space_on_device;

      default:
        return std::error_condition(ev, *this);
    }
  }
};

const Category category{};

}

namespace RCLink {

std::error_code makeErrorCode(Errc ec)
{
  return std::error_code(static_cast<int>(ec), category);
}

}
