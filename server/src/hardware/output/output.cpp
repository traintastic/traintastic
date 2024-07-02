/**
 * server/src/hardware/output/output.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022,2024 Reinder Feenstra
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

#include "output.hpp"
#include "outputcontroller.hpp"
#include "list/outputlist.hpp"
#include "../../world/world.hpp"
#include "list/outputlisttablemodel.hpp"
#include "../../core/attributes.hpp"
#include "../../core/method.tpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../utils/displayname.hpp"

Output::Output(std::shared_ptr<OutputController> outputController, OutputChannel channel_, OutputType type_)
  : interface{this, "interface", std::move(outputController), PropertyFlags::Constant | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , channel{this, "channel", channel_, PropertyFlags::Constant | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , type{this, "type", type_, PropertyFlags::Constant | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , onValueChangedGeneric{*this, "on_value_changed_generic", EventFlags::Public}
{
  m_interfaceItems.add(interface);

  Attributes::addValues(channel, outputChannelValues);
  m_interfaceItems.add(channel);

  m_interfaceItems.add(onValueChangedGeneric);
}

std::string Output::getObjectId() const
{
  assert(false); // Object is not stored or serialized for network, method may not be called.
  return "";
}
