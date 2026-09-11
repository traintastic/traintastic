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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_DINAMOERROR_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_DINAMOERROR_HPP

#include <system_error>

namespace Dinamo {

enum class Errc
{
  BufferFull = 1,
  MessageTooLarge = 2,
};

std::error_code makeErrorCode(Errc ec);

namespace Error
{
  inline std::error_code bufferFull()
  {
    return makeErrorCode(Errc::BufferFull);
  }

  inline std::error_code messageTooLarge()
  {
    return makeErrorCode(Errc::MessageTooLarge);
  }
}

}

template<>
struct std::is_error_code_enum<Dinamo::Errc> : std::true_type {};

#endif
