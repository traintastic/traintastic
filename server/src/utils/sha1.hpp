/**
 * server/src/utils/sha1.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_UTILS_SHA1_HPP
#define TRAINTASTIC_SERVER_UTILS_SHA1_HPP

#include <cstddef>
#include <cstring>
#include <string>
#include <traintastic/utils/stdfilesystem.hpp>

struct Sha1
{
  struct Digest
  {
    friend struct Sha1;

    private:
      unsigned int m_hash[5]; // = boost::uuids::detail::sha1::digest_type

    public:
      inline static Digest null() { return Digest{nullptr}; }

      Digest() = default;

      Digest(std::nullptr_t) :
        m_hash{0, 0, 0, 0, 0}
      {
      }

      bool operator ==(const Digest& other) const
      {
        return std::memcmp(this, &other, sizeof(Digest)) == 0;
      }

      bool operator !=(const Digest& other) const
      {
        return !operator ==(other);
      }
  };

  static Digest of(const std::string& value);
  static Digest of(const std::filesystem::path& filename);
};

#endif
