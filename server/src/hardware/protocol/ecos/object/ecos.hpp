/**
 * server/src/hardware/protocol/ecos/object/ecos.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_OBJECT_ECOS_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_OBJECT_ECOS_HPP

#include "object.hpp"
#include <traintastic/enum/tristate.hpp>
#include "../../../../utils/version.hpp"

namespace ECoS {

class Kernel;

class ECoS final : public Object
{
  public:
    enum class Model
    {
      Unknown = 0,
      ECoS = 1,
      ECoS2 = 2,
    };

    using ProtocolVersion = Version::MajorMinor<uint8_t>;
    using ApplicationVersion = Version::MajorMinorPatch<uint8_t>;
    using HardwareVersion = Version::MajorMinor<uint8_t>;

  private:
    Model m_model = Model::Unknown;
    ProtocolVersion m_protocolVersion;
    ApplicationVersion m_applicationVersion;
    HardwareVersion m_hardwareVersion;
    TriState m_go = TriState::Undefined;

  protected:
    void update(std::string_view option, std::string_view value) final;

  public:
    ECoS(Kernel& kernel);

    Model model() const { return m_model; }
    ProtocolVersion protocolVersion() const { return m_protocolVersion; }
    ApplicationVersion applicationVersion() const { return m_applicationVersion; }
    HardwareVersion hardwareVersion() const { return m_hardwareVersion; }

    bool receiveReply(const Reply& reply) final;
    bool receiveEvent(const Event& event) final;

    void go();
    void stop();
};

}

#endif
