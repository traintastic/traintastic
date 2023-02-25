/**
 * server/src/hardware/input/list/inputlist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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
#include "../../../world/getworld.hpp"
#include "../../../core/attributes.hpp"
#include "../../../utils/displayname.hpp"

InputList::InputList(Object& _parent, std::string_view parentPropertyName, InputListColumn _columns)
  : ObjectList<Input>(_parent, parentPropertyName)
  , columns{_columns}
  , create{*this, "create",
      [this]()
      {
        auto& world = getWorld(parent());
        auto input = Input::create(world, world.getUniqueId(Input::defaultId));
        if(const auto controller = std::dynamic_pointer_cast<InputController>(parent().shared_from_this()))
          input->interface = controller;
        return input;
      }}
  , delete_{*this, "delete", std::bind(&InputList::deleteMethodHandler, this, std::placeholders::_1)}
  , inputMonitor{*this, "input_monitor",
      [this]()
      {
        if(const auto controller = std::dynamic_pointer_cast<InputController>(parent().shared_from_this()))
          return controller->inputMonitor(InputController::defaultInputChannel);
        return std::shared_ptr<InputMonitor>();
      }}
  , inputMonitorChannel{*this, "input_monitor_channel",
      [this](uint32_t channel)
      {
        if(const auto controller = std::dynamic_pointer_cast<InputController>(parent().shared_from_this()))
          return controller->inputMonitor(channel);
        return std::shared_ptr<InputMonitor>();
      }}
{
  const bool editable = contains(getWorld(parent()).state.value(), WorldState::Edit);

  Attributes::addDisplayName(create, DisplayName::List::create);
  Attributes::addEnabled(create, editable);
  m_interfaceItems.add(create);

  Attributes::addDisplayName(delete_, DisplayName::List::delete_);
  Attributes::addEnabled(delete_, editable);
  m_interfaceItems.add(delete_);

  if(auto* controller = dynamic_cast<InputController*>(&_parent))
  {
    const auto* channels = controller->inputChannels();
    if(channels && !channels->empty())
    {
      Attributes::addDisplayName(inputMonitorChannel, DisplayName::Hardware::inputMonitor);
      Attributes::addValues(inputMonitorChannel, channels);
      Attributes::addAliases(inputMonitorChannel, channels, controller->inputChannelNames());
      m_interfaceItems.add(inputMonitorChannel);
    }
    else
    {
      Attributes::addDisplayName(inputMonitor, DisplayName::Hardware::inputMonitor);
      m_interfaceItems.add(inputMonitor);
    }
  }
}

TableModelPtr InputList::getModel()
{
  return std::make_shared<InputListTableModel>(*this);
}

void InputList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<Input>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(create, editable);
  Attributes::setEnabled(delete_, editable);
}

bool InputList::isListedProperty(std::string_view name)
{
  return InputListTableModel::isListedProperty(name);
}
