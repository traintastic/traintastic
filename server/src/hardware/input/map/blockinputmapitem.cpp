/**
 * server/src/hardware/input/map/blockinputmapitem.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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
#include "../../../core/attributes.hpp"
#include "../../../core/objectproperty.tpp"
#include "../../../world/getworld.hpp"
#include "../../../utils/displayname.hpp"
#include "../../../utils/sensor.hpp"
#include "../../../board/tile/rail/blockrailtile.hpp"

BlockInputMapItem::BlockInputMapItem(BlockInputMap& parent, uint32_t itemId) :
  m_parent{parent},
  m_itemId{itemId},
  name{this, "name", std::string("sensor").append(std::to_string(m_itemId)), PropertyFlags::ReadWrite | PropertyFlags::Store},
  input{this, "input", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](const std::shared_ptr<Input>& value)
    {
      if(value)
        inputPropertyChanged(input->value);
      else
        setValue(SensorState::Unknown);
    },
    [this](const std::shared_ptr<Input>& value)
    {
      if(input)
        disconnectInput(*input);

      if(value)
        connectInput(*value);

      return true;
    }},
  type{this, "type", SensorType::OccupyDetector, PropertyFlags::ReadWrite | PropertyFlags::Store},
  invert{this, "invert", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](bool /*value*/)
    {
      if(input)
        inputPropertyChanged(input->value);
    }}
{
  auto& world = getWorld(m_parent);
  const bool editable = contains(world.state.value(), WorldState::Edit);
  const bool stopped = !contains(world.state.value(), WorldState::Run);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);
  Attributes::addEnabled(input, editable && stopped);
  Attributes::addObjectList(input, world.inputs);
  m_interfaceItems.add(input);
  Attributes::addEnabled(type, false/*editable && stopped*/);
  Attributes::addValues(type, sensorTypeValues);
  m_interfaceItems.add(type);
  Attributes::addEnabled(invert, editable && stopped);
  m_interfaceItems.add(invert);
}

BlockInputMapItem::~BlockInputMapItem()
{
  assert(!input);
  assert(!m_inputPropertyChanged.connected());
  assert(!m_inputDestroying.connected());
}

std::string BlockInputMapItem::getObjectId() const
{
  return m_parent.getObjectId().append(".").append(m_parent.items.name()).append(".item").append(std::to_string(m_itemId));
}

void BlockInputMapItem::save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const
{
  InputMapItem::save(saver, data, state);
  data["item_id"] = m_itemId;
}

void BlockInputMapItem::loaded()
{
  InputMapItem::loaded();

  if(input)
    connectInput(*input);
}

void BlockInputMapItem::destroying()
{
  input = nullptr;
  InputMapItem::destroying();
}

void BlockInputMapItem::worldEvent(WorldState state, WorldEvent event)
{
  InputMapItem::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);
  const bool stopped = !contains(state, WorldState::Run);

  Attributes::setEnabled(name, editable);
  Attributes::setEnabled(input, editable && stopped);
  Attributes::setEnabled(type, false/*editable && stopped*/);
  Attributes::setEnabled(invert, editable && stopped);
}

void BlockInputMapItem::connectInput(Input& object)
{
  object.consumers.appendInternal(m_parent.parent().shared_from_this());
  m_inputDestroying = object.onDestroying.connect(
    [this]([[maybe_unused]] Object& obj)
    {
      assert(input.value().get() == &obj);
      input = nullptr;
    });
  m_inputPropertyChanged = object.propertyChanged.connect(std::bind(&BlockInputMapItem::inputPropertyChanged, this, std::placeholders::_1));
}

void BlockInputMapItem::disconnectInput(Input& object)
{
  m_inputPropertyChanged.disconnect();
  m_inputDestroying.disconnect();
  object.consumers.removeInternal(m_parent.parent().shared_from_this());
}

void BlockInputMapItem::inputPropertyChanged(BaseProperty& property)
{
  assert(input);
  if(&property == static_cast<BaseProperty*>(&input->value))
    setValue(toSensorState(type, input->value.value() ^ invert.value()));
}

void BlockInputMapItem::setValue(SensorState value)
{
  if(m_value != value)
  {
    m_value = value;
    static_cast<BlockRailTile&>(m_parent.parent()).inputItemValueChanged(*this);
  }
}
