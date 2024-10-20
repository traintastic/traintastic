/**
 * server/src/hardware/interface/dinamointerface.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_DINAMOINTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_DINAMOINTERFACE_HPP

#include "interface.hpp"
#include "../../core/serialdeviceproperty.hpp"

namespace Dinamo {
  class Kernel;
  class Settings;
}

class DinamoInterface final
  : public Interface
{
  CLASS_ID("interface.dinamo")
  DEFAULT_ID("dinamo")
  CREATE_DEF(DinamoInterface)

private:
  std::unique_ptr<Dinamo::Kernel> m_kernel;
  boost::signals2::connection m_dinamoPropertyChanged;

  void addToWorld() final;
  void destroying() final;
  void worldEvent(WorldState state, WorldEvent event) final;
  bool setOnline(bool& value, bool simulation) final;

public:
  SerialDeviceProperty device;
  ObjectProperty<Dinamo::Settings> dinamo;

  DinamoInterface(World& world, std::string_view objectId);
  ~DinamoInterface();
};

#endif
