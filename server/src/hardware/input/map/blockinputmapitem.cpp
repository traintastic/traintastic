/**
 * server/src/hardware/input/map/blockinputmapitem.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2025 Reinder Feenstra
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

#include "blockinputmapitem.hpp"
#include "blockinputmap.hpp"
#include "../input.hpp"
#include "../../../core/attributes.hpp"
#include "../../../core/objectproperty.tpp"
#include "../../../world/getworld.hpp"
#include "../../../utils/displayname.hpp"
#include "../../../utils/sensor.hpp"
#include "../../../board/tile/rail/blockrailtile.hpp"

BlockInputMapItem::BlockInputMapItem(BlockInputMap& parent, uint32_t itemId) :
  InputConsumer(static_cast<Object&>(*this), getWorld(parent)),
  m_parent{parent},
  m_itemId{itemId},
  name{this, "name", std::string("sensor").append(std::to_string(m_itemId)), PropertyFlags::ReadWrite | PropertyFlags::Store},
  type{this, "type", SensorType::OccupancyDetector, PropertyFlags::ReadWrite | PropertyFlags::Store},
  invert{this, "invert", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](bool /*value*/)
    {
      if(input() && input()->value != TriState::Undefined)
      {
        inputValueChanged(input()->value == TriState::True, input());
      }
    }}
  , identification{this, "identification", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store,
      nullptr,
      [this](const std::shared_ptr<Identification>& value)
      {
        if(identification)
          disconnectIdentification(*identification);

        if(value)
          connectIdentification(*value);

        return true;
      }}
{
  auto& world = getWorld(m_parent);
  const bool editable = contains(world.state.value(), WorldState::Edit);
  const bool stopped = !contains(world.state.value(), WorldState::Run);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);

  m_interfaceItems.add(interface);
  m_interfaceItems.add(channel);
  m_interfaceItems.add(address);

  Attributes::addEnabled(type, false/*editable && stopped*/);
  Attributes::addValues(type, sensorTypeValues);
  m_interfaceItems.add(type);
  Attributes::addEnabled(invert, editable && stopped);
  m_interfaceItems.add(invert);
  Attributes::addEnabled(identification, editable && stopped);
  Attributes::addObjectList(identification, world.identifications);
  m_interfaceItems.add(identification);
}

BlockInputMapItem::~BlockInputMapItem()
{
  assert(!identification);
  assert(!m_identificationDestroying.connected());
}

std::string BlockInputMapItem::getObjectId() const
{
  return m_parent.getObjectId().append(".").append(m_parent.items.name()).append(".item").append(std::to_string(m_itemId));
}

//! \todo Remove in v0.4
void BlockInputMapItem::load(WorldLoader& loader, const nlohmann::json& data)
{
  if(data["type"] == "occupy_detector")
  {
    nlohmann::json dataCopy = data;
    dataCopy["type"] = "occupancy_detector";
    InputMapItem::load(loader, dataCopy);
  }
  else
  {
    InputMapItem::load(loader, data);
  }
}

void BlockInputMapItem::save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const
{
  InputMapItem::save(saver, data, state);
  data["item_id"] = m_itemId;
}

void BlockInputMapItem::loaded()
{
  InputMapItem::loaded();
  InputConsumer::loaded();

  if(identification)
    connectIdentification(*identification);
}

void BlockInputMapItem::destroying()
{
  identification = nullptr;
  InputMapItem::destroying();
}

void BlockInputMapItem::worldEvent(WorldState state, WorldEvent event)
{
  InputMapItem::worldEvent(state, event);
  InputConsumer::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);
  const bool stopped = !contains(state, WorldState::Run);

  Attributes::setEnabled(name, editable);
  Attributes::setEnabled(type, false/*editable && stopped*/);
  Attributes::setEnabled(invert, editable && stopped);
  Attributes::setEnabled(identification, editable && stopped);
}

void BlockInputMapItem::inputValueChanged(bool value, const std::shared_ptr<Input>& /*input*/)
{
  setValue(toSensorState(type, toTriState(value != invert.value())));
}

void BlockInputMapItem::connectIdentification(Identification& object)
{
  object.consumers.appendInternal(m_parent.parent().shared_from_this());
  m_identificationDestroying = object.onDestroying.connect(
    [this]([[maybe_unused]] Object& obj)
    {
      assert(identification.value().get() == &obj);
      identification = nullptr;
    });
  m_identificationEvent = object.onEvent.connect(
    [this](IdentificationEventType eventType, uint16_t identifier, Direction direction, uint8_t category)
    {
      static_cast<BlockRailTile&>(m_parent.parent()).identificationEvent(*this, eventType, identifier, direction, category);
    });
}

void BlockInputMapItem::disconnectIdentification(Identification& object)
{
  m_identificationEvent.disconnect();
  m_identificationDestroying.disconnect();
  object.consumers.removeInternal(m_parent.parent().shared_from_this());
}

void BlockInputMapItem::setValue(SensorState value)
{
  if(m_value != value)
  {
    m_value = value;
    static_cast<BlockRailTile&>(m_parent.parent()).inputItemValueChanged(*this);
  }
}
