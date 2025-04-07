/**
 * server/src/core/serialdeviceproperty.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_SERIALDEVICEPROPERTY_HPP
#define TRAINTASTIC_SERVER_CORE_SERIALDEVICEPROPERTY_HPP

#include "property.hpp"
#include <boost/signals2/connection.hpp>

class SerialDeviceProperty final : public Property<std::string>
{
  private:
    boost::signals2::scoped_connection m_serialPortListChanged;

  public:
    SerialDeviceProperty(Object* object, std::string_view name, const std::string& value, PropertyFlags flags);
    ~SerialDeviceProperty() final = default;
};

#endif
