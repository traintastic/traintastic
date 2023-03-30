/**
 * server/src/hardware/output/list/outputlist.cpp
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

#include "outputlist.hpp"
#include "outputlisttablemodel.hpp"
#include "../outputcontroller.hpp"
#include "../../../world/getworld.hpp"
#include "../../../core/attributes.hpp"
#include "../../../core/objectproperty.tpp"
#include "../../../utils/displayname.hpp"

OutputList::OutputList(Object& _parent, std::string_view parentPropertyName, OutputListColumn _columns)
  : ObjectList<Output>(_parent, parentPropertyName)
  , columns{_columns}
  , create{*this, "create",
      [this]()
      {
        auto& world = getWorld(parent());
        auto output = Output::create(world, world.getUniqueId(Output::defaultId));
        if(const auto controller = std::dynamic_pointer_cast<OutputController>(parent().shared_from_this()))
          output->interface = controller;
        return output;
      }}
  , delete_{*this, "delete", std::bind(&OutputList::deleteMethodHandler, this, std::placeholders::_1)}
  , outputKeyboard{*this, "output_keyboard",
      [this]()
      {
        if(const auto controller = std::dynamic_pointer_cast<OutputController>(parent().shared_from_this()))
          return controller->outputKeyboard(OutputController::defaultOutputChannel);
        return std::shared_ptr<OutputKeyboard>();
      }}
  , outputKeyboardChannel{*this, "output_keyboard_channel",
      [this](uint32_t channel)
      {
        if(const auto controller = std::dynamic_pointer_cast<OutputController>(parent().shared_from_this()))
          return controller->outputKeyboard(channel);
        return std::shared_ptr<OutputKeyboard>();
      }}
{
  const bool editable = contains(getWorld(parent()).state.value(), WorldState::Edit);

  Attributes::addDisplayName(create, DisplayName::List::create);
  Attributes::addEnabled(create, editable);
  m_interfaceItems.add(create);

  Attributes::addDisplayName(delete_, DisplayName::List::delete_);
  Attributes::addEnabled(delete_, editable);
  m_interfaceItems.add(delete_);

  if(auto* controller = dynamic_cast<OutputController*>(&_parent))
  {
    const auto* channels = controller->outputChannels();
    if(channels && !channels->empty())
    {
      Attributes::addDisplayName(outputKeyboardChannel, DisplayName::Hardware::outputKeyboard);
      Attributes::addValues(outputKeyboardChannel, channels);
      Attributes::addAliases(outputKeyboardChannel, channels, controller->outputChannelNames());
      m_interfaceItems.add(outputKeyboardChannel);
    }
    else
    {
      Attributes::addDisplayName(outputKeyboard, DisplayName::Hardware::outputKeyboard);
      m_interfaceItems.add(outputKeyboard);
    }
  }
}

TableModelPtr OutputList::getModel()
{
  return std::make_shared<OutputListTableModel>(*this);
}

void OutputList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<Output>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(create, editable);
  Attributes::setEnabled(delete_, editable);
}

bool OutputList::isListedProperty(std::string_view name)
{
  return OutputListTableModel::isListedProperty(name);
}
