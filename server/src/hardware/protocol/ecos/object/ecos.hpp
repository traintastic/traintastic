/**
 * server/src/hardware/protocol/ecos/object/ecos.hpp
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
      CentralStation, // Marklin CS1 "Reloaded"
    };

    using ProtocolVersion = Version::MajorMinor<uint8_t>;
    using ApplicationVersion = Version::MajorMinorPatch<uint8_t>;
    using HardwareVersion = Version::MajorMinor<uint8_t>;

  private:
    Model m_model = Model::Unknown;
    ProtocolVersion m_protocolVersion;
    ApplicationVersion m_applicationVersion;
    std::string m_applicationVersionSuffix;
    HardwareVersion m_hardwareVersion;
    bool m_railcom = false;
    bool m_railcomPlus = false;
    TriState m_go = TriState::Undefined;

  protected:
    void update(std::string_view option, std::string_view value) final;

  public:
    ECoS(Kernel& kernel);

    Model model() const { return m_model; }
    ProtocolVersion protocolVersion() const { return m_protocolVersion; }
    ApplicationVersion applicationVersion() const { return m_applicationVersion; }
    const std::string& applicationVersionSuffix() const { return m_applicationVersionSuffix; }
    HardwareVersion hardwareVersion() const { return m_hardwareVersion; }
    bool railcom() const { return m_railcom; }
    bool railcomPlus() const { return m_railcomPlus; }

    bool receiveReply(const Reply& reply) final;
    bool receiveEvent(const Event& event) final;

    void go();
    void stop();
};

constexpr bool fromString(std::string_view text, ECoS::Model& model)
{
  if(text == "ECoS")
  {
    model = ECoS::Model::ECoS;
  }
  else if(text == "ECoS2")
  {
    model = ECoS::Model::ECoS2;
  }
  else if(text == "CentralStation")
  {
    model = ECoS::Model::CentralStation;
  }
  else
  {
    return false;
  }
  return true;
}

constexpr std::string_view toString(ECoS::Model value)
{
  switch(value)
  {
    case ECoS::Model::ECoS:
      return "ECoS";

    case ECoS::Model::ECoS2:
      return "ECoS2";

    case ECoS::Model::CentralStation:
      return "CentralStation";

    case ECoS::Model::Unknown:
      break;
  }
  return {};
}

}

#endif
