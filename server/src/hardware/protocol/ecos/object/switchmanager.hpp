/**
 * server/src/hardware/protocol/ecos/object/switchmanager.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_OBJECT_SWITCHMANAGER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_OBJECT_SWITCHMANAGER_HPP

#include "object.hpp"
#include "switchprotocol.hpp"

namespace ECoS {

class Kernel;

class SwitchManager final : public Object
{
  protected:
    void update(std::string_view option, std::string_view value) final;

  public:
    SwitchManager(Kernel& kernel);

    void setSwitch(SwitchProtocol protocol, uint16_t address, bool port);

    bool receiveReply(const Reply& reply) final;
    bool receiveEvent(const Event& event) final;
};

}

#endif
