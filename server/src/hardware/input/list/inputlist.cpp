/**
 * server/src/hardware/input/list/inputlist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2025 Reinder Feenstra
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

#include "inputlist.hpp"
#include "inputlisttablemodel.hpp"
#include "../inputcontroller.hpp"
#include "../monitor/inputmonitor.hpp"
#include "../../../world/getworld.hpp"
#include "../../../core/attributes.hpp"
#include "../../../core/method.tpp"
#include "../../../core/objectproperty.tpp"
#include "../../../utils/displayname.hpp"

InputList::InputList(Object& _parent, std::string_view parentPropertyName, InputListColumn _columns)
  : ObjectList<Input>(_parent, parentPropertyName)
  , columns{_columns}
  , inputMonitor{*this, "input_monitor",
      [this](InputChannel channel)
      {
        if(const auto controller = std::dynamic_pointer_cast<InputController>(parent().shared_from_this()))
        {
          return controller->inputMonitor(channel);
        }
        return std::shared_ptr<InputMonitor>();
      }}
{
  if(auto* controller = dynamic_cast<InputController*>(&_parent))
  {
    Attributes::addDisplayName(inputMonitor, DisplayName::Hardware::inputMonitor);
    Attributes::addValues(inputMonitor, controller->inputChannels());
    m_interfaceItems.add(inputMonitor);
  }
}

TableModelPtr InputList::getModel()
{
  return std::make_shared<InputListTableModel>(*this);
}

bool InputList::isListedProperty(std::string_view name)
{
  return InputListTableModel::isListedProperty(name);
}
