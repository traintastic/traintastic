/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2021-2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_ECOS_ECOSSETTINGS_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_ECOS_ECOSSETTINGS_HPP

#include "../../../core/subobject.hpp"
#include "../../../core/property.hpp"
#include "../../protocol/ecos/config.hpp"

class ECoSSettings final : public SubObject
{
  public:
    CLASS_ID("ecos_settings")

    Property<bool> debugLogRXTX;

    ECoSSettings(Object& _parent, std::string_view parentPropertyName);

    ECoS::Config config() const;
};

#endif
