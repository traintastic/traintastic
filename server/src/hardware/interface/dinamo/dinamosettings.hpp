/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_DINAMO_DINAMOSETTINGS_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_DINAMO_DINAMOSETTINGS_HPP

#include "../../../core/subobject.hpp"
#include "../../../core/property.hpp"
#include "../../../hardware/protocol/dinamo/dinamoconfig.hpp"

class DinamoSettings final : public SubObject
{
  CLASS_ID("dinamo_settings")

public:
  Property<bool> setHFILevel;
  Property<uint8_t> hfiLevel;
  Property<bool> debugLogRXTX;
  Property<bool> debugLogTrainBlocks;

  DinamoSettings(Object& _parent, std::string_view parentPropertyName);

  Dinamo::Config config() const;

protected:
  void loaded() final;
};

#endif
