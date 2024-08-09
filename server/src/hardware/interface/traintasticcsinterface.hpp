/**
 * server/src/hardware/interface/traintasticcsinterface.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_TRAINTASTICCSINTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_TRAINTASTICCSINTERFACE_HPP

#include "interface.hpp"
#include "../../core/serialdeviceproperty.hpp"
#include "../../core/objectproperty.hpp"

namespace TraintasticCS
{
  class Kernel;
  class Settings;
}

/**
 * \brief Traintastic CS hardware interface
 */
class TraintasticCSInterface final
  : public Interface
{
  CLASS_ID("interface.traintastic_cs")
  DEFAULT_ID("traintastic_cs")
  CREATE_DEF(TraintasticCSInterface)

  private:
    std::unique_ptr<TraintasticCS::Kernel> m_kernel;
    boost::signals2::connection m_traintasticCSPropertyChanged;

    void addToWorld() final;
    void destroying() final;

  protected:
    bool setOnline(bool& value, bool simulation) final;

  public:
    SerialDeviceProperty device;
    ObjectProperty<TraintasticCS::Settings> traintasticCS;

    TraintasticCSInterface(World& world, std::string_view _id);
};

#endif
