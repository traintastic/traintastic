/**
 * server/src/core/serialdeviceproperty.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#include "serialdeviceproperty.hpp"
#include "object.hpp"
#include "attributes.hpp"
#include "../os/serialportlist.hpp"
#include "../utils/displayname.hpp"

SerialDeviceProperty::SerialDeviceProperty(Object* object, std::string_view name, const std::string& value, PropertyFlags flags)
  : Property<std::string>(object, name, value, flags)
{
  ::Attributes::addDisplayName(*this, DisplayName::Serial::device);
  ::Attributes::addValues(*this, &SerialPortList::instance().get());

  m_serialPortListChanged = SerialPortList::instance().changed.connect(
    [this]()
    {
      auto it = attributes().find(AttributeName::Values);
      assert(it != attributes().end());
      static_cast<VectorRefAttribute<std::string>&>(*it->second).internalChanged();
    });
}
