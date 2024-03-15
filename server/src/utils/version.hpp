/**
 * server/src/utils/version.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_UTILS_VERSION_HPP
#define TRAINTASTIC_SERVER_UTILS_VERSION_HPP

#include <type_traits>
#include "fromchars.hpp"

namespace Version
{
  template<class T>
  struct MajorMinor
  {
    static_assert(std::is_unsigned_v<T>);

    using Type = T;

    T major = 0;
    T minor = 0;
  };

  template<class T>
  struct MajorMinorPatch : MajorMinor<T>
  {
    T patch = 0;
  };

  template<class T>
  std::from_chars_result fromChars(std::string_view sv, MajorMinor<T>& version)
  {
    T major;
    auto r = ::fromChars(sv, major);
    if(r.ec != std::errc())
      return r;

    if(r.ptr == (sv.data() + sv.size()) || *r.ptr != '.')
      return {r.ptr, std::errc::invalid_argument};

    T minor;
    r = ::fromChars(sv.substr(1 + r.ptr - sv.data()), minor);

    if(r.ec == std::errc())
    {
      version.major = major;
      version.minor = minor;
    }

    return r;
  }

  template<class T>
  std::from_chars_result fromChars(std::string_view sv, MajorMinorPatch<T>& version)
  {
    MajorMinor<T> mm;
    auto r = fromChars(sv, mm);
    if(r.ec != std::errc())
      return r;

    if(r.ptr == (sv.data() + sv.size()) || *r.ptr != '.')
      return {r.ptr, std::errc::invalid_argument};

    T patch;
    r = ::fromChars(sv.substr(1 + r.ptr - sv.data()), patch);

    if(r.ec == std::errc())
    {
      version.major = mm.major;
      version.minor = mm.minor;
      version.patch = patch;
    }

    return r;
  }
}

template<typename T>
inline std::string toString(const Version::MajorMinor<T>& version)
{
  return std::to_string(version.major).append(".").append(std::to_string(version.minor));
}

template<typename T>
inline std::string toString(const Version::MajorMinorPatch<T>& version)
{
  return toString(static_cast<const Version::MajorMinor<T>&>(version)).append(".").append(std::to_string(version.patch));
}

#endif
