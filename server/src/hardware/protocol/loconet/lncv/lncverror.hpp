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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_LNCV_LNCVERROR_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_LNCV_LNCVERROR_HPP

#include <system_error>

namespace LocoNet::LNCV {

enum class Errc
{
  // zero means no error
  UnexpectedResponse = 1,
  NoResponse = 2,
  Rejected = 3,
  NotFound = 4,
  ReadOnly = 5,
};

std::error_code makeErrorCode(Errc ec);

namespace Error
{
  inline std::error_code unexpectedResponse()
  {
    return makeErrorCode(Errc::UnexpectedResponse);
  }

  inline std::error_code noResponse()
  {
    return makeErrorCode(Errc::NoResponse);
  }

  inline std::error_code rejected()
  {
    return makeErrorCode(Errc::Rejected);
  }

  inline std::error_code notFound()
  {
    return makeErrorCode(Errc::NotFound);
  }

  inline std::error_code readOnly()
  {
    return makeErrorCode(Errc::ReadOnly);
  }
}

}

template<>
struct std::is_error_code_enum<LocoNet::LNCV::Errc> : std::true_type {};

#endif
