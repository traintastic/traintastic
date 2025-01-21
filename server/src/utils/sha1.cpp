/**
 * server/src/utils/sha1.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#include "sha1.hpp"
#include <fstream>
#include <boost/uuid/detail/sha1.hpp>

Sha1::Digest Sha1::of(const std::string& value)
{
  boost::uuids::detail::sha1 sha1;
	sha1.process_bytes(value.c_str(), value.size());
  Digest digest;
  boost::uuids::detail::sha1::digest_type tmp;
	sha1.get_digest(tmp);
  std::memcpy(digest.m_hash, tmp, sizeof(tmp)); // FIXME: get rid of this copy
  return digest;
}

Sha1::Digest Sha1::of(const std::filesystem::path& filename)
{
  std::ifstream file(filename, std::ios::in | std::ios::binary);
  if(!file.good())
    return Digest::null();

  boost::uuids::detail::sha1 sha1;
  char buffer[64 * 1024];
  while(file.good())
  {
    file.read(buffer, sizeof(buffer));
    sha1.process_bytes(buffer, file.gcount());
  }

  if(!file.eof())
    return Digest::null();

  Digest digest;
  boost::uuids::detail::sha1::digest_type tmp;
	sha1.get_digest(tmp);
  std::memcpy(digest.m_hash, tmp, sizeof(tmp)); // FIXME: get rid of this copy
  return digest;
}
