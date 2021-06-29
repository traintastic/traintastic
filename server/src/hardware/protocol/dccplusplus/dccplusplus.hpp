/**
 * server/src/hardware/protocol/dccplusplus/dccplusplus.hpp
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


#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DCCPLUSPLUS_DCCPLUSPLUS_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DCCPLUSPLUS_DCCPLUSPLUS_HPP

#include "../../../core/subobject.hpp"
#include <string_view>
#include "../../../core/property.hpp"

class CommandStation;
class Decoder;
enum class DecoderChangeFlags;

namespace DCCPlusPlus {

class DCCPlusPlus : public SubObject
{
  CLASS_ID("protocol.dccplusplus")

  private:
    CommandStation* const m_commandStation; // valid if parent is command station, else nullptr
    std::function<bool(std::string_view)> m_send;
    std::atomic_bool m_debugLogRXTX;

  protected:
    void loaded() final;

  public:
    Property<bool> useEx;
    Property<bool> debugLogRXTX;

    DCCPlusPlus(Object& _parent, const std::string& parentPropertyName, std::function<bool(std::string_view)> send);

    bool send(std::string_view message);
    void receive(std::string_view message);

    void emergencyStopChanged(bool value);
    void powerOnChanged(bool value);
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber);
};

}

#endif
