/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025-2026 Reinder Feenstra
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

#include "lncverror.hpp"

namespace {

class Category final : public std::error_category
{
public:
  const char* name() const noexcept final
  {
    return "loconet.lncv";
  }

  std::string message(int ev) const final
  {
    switch (static_cast<LocoNet::LNCV::Errc>(ev))
    {
      using enum LocoNet::LNCV::Errc;

      case UnexpectedResponse:
        return "Unexpected response to LNCV request";

      case NoResponse:
        return "No response to LNCV request";

      case Rejected:
        return "LNCV request rejected";

      case NotFound:
        return "LNCV not found";

      case ReadOnly:
        return "LNCV is read only";

      default:
        return "Unknown LNCV error";
    }
  }

  std::error_condition default_error_condition(int ev) const noexcept final
  {
    switch (static_cast<LocoNet::LNCV::Errc>(ev))
    {
      using enum LocoNet::LNCV::Errc;

      case NoResponse:
        return std::errc::timed_out;

      case Rejected:
        return std::errc::operation_not_permitted;

      case NotFound:
        return std::errc::no_such_file_or_directory;

      case ReadOnly:
        return std::errc::read_only_file_system;

      default:
        return std::error_condition(ev, *this);
    }
  }
};

const Category category{};

}

namespace LocoNet::LNCV {

const std::error_category& errorCategory()
{
    return category;
}

std::error_code makeErrorCode(Errc ec)
{
    return {static_cast<int>(ec), errorCategory()};
}

}
