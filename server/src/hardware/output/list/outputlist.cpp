/**
 * server/src/hardware/output/list/outputlist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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
#include "../../../utils/displayname.hpp"

OutputList::OutputList(Object& _parent, std::string_view parentPropertyName)
  : ObjectList<Output>(_parent, parentPropertyName)
  , m_parentIsOutputController(dynamic_cast<OutputController*>(&_parent))
  , add{*this, "add",
      [this]()
      {
        auto& world = getWorld(parent());
        auto output = Output::create(world, world.getUniqueId(Output::defaultId));
        if(const auto controller = std::dynamic_pointer_cast<OutputController>(parent().shared_from_this()))
        {
          if(const uint32_t address = controller->getUnusedOutputAddress(); address != Output::invalidAddress)
          {
            output->address = address;
            output->interface = controller;
          }
        }
        return output;
      }}
  , remove{*this, "remove",
    [this](const std::shared_ptr<Output>& output)
    {
      if(containsObject(output))
        output->destroy();
      assert(!containsObject(output));
    }}
  , outputKeyboard{*this, "output_keyboard",
      [this]()
      {
        if(const auto controller = std::dynamic_pointer_cast<OutputController>(parent().shared_from_this()))
          return controller->outputKeyboard();
        return std::shared_ptr<OutputKeyboard>();
      }}
{
  const bool editable = contains(getWorld(parent()).state.value(), WorldState::Edit);

  Attributes::addDisplayName(add, DisplayName::List::add);
  Attributes::addEnabled(add, editable);
  m_interfaceItems.add(add);

  Attributes::addDisplayName(remove, DisplayName::List::remove);
  Attributes::addEnabled(remove, editable);
  m_interfaceItems.add(remove);

  if(m_parentIsOutputController)
  {
    Attributes::addDisplayName(outputKeyboard, DisplayName::Hardware::outputKeyboard);
    m_interfaceItems.add(outputKeyboard);
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

  Attributes::setEnabled(add, editable);
  Attributes::setEnabled(remove, editable);
}

bool OutputList::isListedProperty(std::string_view name)
{
  return OutputListTableModel::isListedProperty(name);
}
