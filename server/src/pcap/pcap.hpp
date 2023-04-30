/**
 * server/src/pcap/pcap.hpp
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

#ifndef TRAINTASTIC_SERVER_PCAP_PCAP_HPP
#define TRAINTASTIC_SERVER_PCAP_PCAP_HPP

#include <cstdint>
#include <cstdlib>

class PCAP
{
  private:
    struct GlobalHeader
    {
      uint32_t magic_number = 0xA1B2C3D4;  //!< magic number
      uint16_t version_major = 2; //!< major version number
      uint16_t version_minor = 4; //!< minor version number
      int32_t  thiszone; //!< GMT to local correction
      uint32_t sigfigs = 0; //!< accuracy of timestamps
      uint32_t snaplen; //!< max length of captured packets, in octets
      uint32_t network; //!< data link type
    };

    struct RecordHeader
    {
      uint32_t ts_sec;   //!< timestamp seconds
      uint32_t ts_usec;  //!< timestamp microseconds
      uint32_t incl_len; //!< number of octets of packet saved in file
      uint32_t orig_len; //!< actual length of packet
    };

  protected:
    PCAP() = default;

    void writeHeader(uint32_t network);

    virtual void write(const void* buffer, size_t size) = 0;

  public:
    virtual ~PCAP() = default;

    void writeRecord(const void* data, uint32_t size);
};

#endif
