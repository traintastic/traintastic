/**
 * server/src/hardware/decoder/decoderlist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#include "decoderlist.hpp"
#include "decoderlisttablemodel.hpp"
#include "../commandstation/commandstation.hpp"
#include "../../world/getworld.hpp"
#include "../../core/attributes.hpp"

DecoderList::DecoderList(Object& _parent, const std::string& parentPropertyName) :
  ObjectList<Decoder>(_parent, parentPropertyName),
  add{*this, "add",
    [this]()
    {
      auto world = getWorld(&this->parent());
      if(!world)
        return std::shared_ptr<Decoder>();
      auto decoder = Decoder::create(world, world->getUniqueId("decoder"));
      //addObject(decoder);
      if(auto* cs = dynamic_cast<CommandStation*>(&this->parent()))
        decoder->commandStation = cs->shared_ptr<CommandStation>();
      //else if(world->commandStations->length() == 1)
      //  decoder->commandStation = cs->shared_ptr<CommandStation>();
      return decoder;
    }}
  , remove{*this, "remove",
      [this](const std::shared_ptr<Decoder>& decoder)
      {
        if(!decoder)
          return;
        decoder->destroy();
        assert(!containsObject(decoder));
      }}
{
  auto world = getWorld(&_parent);
  const bool editable = world && contains(world->state.value(), WorldState::Edit);

  Attributes::addEnabled(add, editable);
  m_interfaceItems.add(add);
  Attributes::addEnabled(remove, editable);
  m_interfaceItems.add(remove);
}

TableModelPtr DecoderList::getModel()
{
  return std::make_shared<DecoderListTableModel>(*this);
}

void DecoderList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<Decoder>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  add.setAttributeEnabled(editable);
  remove.setAttributeEnabled(editable);
}

bool DecoderList::isListedProperty(const std::string& name)
{
  return DecoderListTableModel::isListedProperty(name);
}
