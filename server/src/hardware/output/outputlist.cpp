/**
 * server/src/hardware/inpu/inpulist.cpp
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
#include "outputs.hpp"
#include "../../world/getworld.hpp"
#include "../../core/attributes.hpp"

OutputList::OutputList(Object& _parent, const std::string& parentPropertyName) :
  ObjectList<Output>(_parent, parentPropertyName),
  add{*this, "add",
    [this](std::string_view classId)
    {
      auto world = getWorld(&this->parent());
      if(!world)
        return std::shared_ptr<Output>();
      auto output = Outputs::create(world, classId, world->getUniqueId("output"));
      if(auto locoNetOutput = std::dynamic_pointer_cast<LocoNetOutput>(output); locoNetOutput && world->loconets->length == 1)
      {
        auto& loconet = world->loconets->operator[](0);
        if(uint16_t address = loconet->getUnusedOutputAddress(); address != LocoNetOutput::addressInvalid)
        {
          locoNetOutput->address = address;
          locoNetOutput->loconet = loconet;
        }
      }
      return output;
    }}
{
  auto w = getWorld(&_parent);
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addEnabled(add, editable);
  Attributes::addClassList(add, Outputs::classList);
  m_interfaceItems.add(add);
}

TableModelPtr OutputList::getModel()
{
  return std::make_shared<OutputListTableModel>(*this);
}

void OutputList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<Output>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  add.setAttributeEnabled(editable);
}

bool OutputList::isListedProperty(const std::string& name)
{
  return OutputListTableModel::isListedProperty(name);
}
