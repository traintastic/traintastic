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

#include "dinamosettings.hpp"
#include "../../../core/attributes.hpp"
#include "../../../utils/displayname.hpp"

namespace {

constexpr uint8_t hfiLevelMin = 0;
constexpr uint8_t hfiLevelMax = 15;

}

DinamoSettings::DinamoSettings(Object& _parent, std::string_view parentPropertyName)
  : SubObject(_parent, parentPropertyName)
  , setHFILevel{this, "set_hfi_level", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](bool value)
    {
      Attributes::setEnabled(hfiLevel, value);
    }}
  , hfiLevel{this, "hfi_level", hfiLevelMin, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , debugLogRXTX{this, "debug_log_rx_tx", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , debugLogTrainBlocks{this, "debug_log_train_blocks", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  m_interfaceItems.add(setHFILevel);

  Attributes::addEnabled(hfiLevel, setHFILevel);
  Attributes::addMinMax(hfiLevel, hfiLevelMin, hfiLevelMax);
  m_interfaceItems.add(hfiLevel);

  Attributes::addDisplayName(debugLogRXTX, DisplayName::Hardware::debugLogRXTX);
  //Attributes::addGroup(debugLogRXTX, Group::debug);
  m_interfaceItems.add(debugLogRXTX);

  //Attributes::addGroup(debugLogTrainBlocks, Group::debug);
  m_interfaceItems.add(debugLogTrainBlocks);
}

Dinamo::Config DinamoSettings::config() const
{
  return Dinamo::Config{
    .setHFILevel = setHFILevel,
    .hfiLevel = hfiLevelMin,
    .debugLogRXTX = debugLogRXTX,
  };
}

void DinamoSettings::loaded()
{
  SubObject::loaded();

  Attributes::setEnabled(hfiLevel, setHFILevel);
}
