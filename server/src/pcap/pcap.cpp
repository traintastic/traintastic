/**
 * server/src/pcap/pcap.cpp
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

#include "pcap.hpp"
#include <chrono>

void PCAP::writeHeader(uint32_t network)
{
  GlobalHeader header;
  header.thiszone = 0; //! \todo set system timezone offset
  header.sigfigs = 0;
  header.snaplen = 255;
  header.network = network;
  write(&header, sizeof(header));
}

void PCAP::writeRecord(const void* data, uint32_t size)
{
  const auto us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  RecordHeader header{static_cast<uint32_t>(us / 1'000'000), static_cast<uint32_t>(us % 1'000'000), size, size};
  write(&header, sizeof(header));
  write(data, size);
}
