/**
 * server/src/hardware/output/outputcontroller.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022,2024 Reinder Feenstra
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

#include "outputcontroller.hpp"
#include "singleoutput.hpp"
#include "pairoutput.hpp"
#include "aspectoutput.hpp"
#include "ecosstateoutput.hpp"
#include "list/outputlist.hpp"
#include "list/outputlisttablemodel.hpp"
#include "keyboard/singleoutputkeyboard.hpp"
#include "keyboard/pairoutputkeyboard.hpp"
#include "../protocol/dcc/dcc.hpp"
#include "../protocol/motorola/motorola.hpp"
#include "../../core/attributes.hpp"
#include "../../core/controllerlist.hpp"
#include "../../core/objectproperty.tpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../world/getworld.hpp"
#include "../../world/world.hpp"

OutputController::OutputController(IdObject& interface)
  : outputs{&interface, "outputs", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
{
  Attributes::addDisplayName(outputs, DisplayName::Hardware::outputs);
}

OutputType OutputController::outputType(OutputChannel channel) const
{
  switch(channel)
  {
    case OutputChannel::Output:
    case OutputChannel::Turnout:
      return OutputType::Single;

    case OutputChannel::Accessory:
    case OutputChannel::AccessoryDCC:
    case OutputChannel::AccessoryMotorola:
      return OutputType::Pair;

    case OutputChannel::DCCext:
      return OutputType::Aspect;

    case OutputChannel::ECoSObject:
      return OutputType::ECoSState;
  }
  assert(false);
  return static_cast<OutputType>(0);
}

std::pair<uint32_t, uint32_t> OutputController::outputAddressMinMax(OutputChannel channel) const
{
  // Handle standard ranges, other or limited ranges must be implemented in the interface.
  switch(channel)
  {
    case OutputChannel::AccessoryDCC:
    case OutputChannel::DCCext:
      return {DCC::Accessory::addressMin, DCC::Accessory::addressMax};

    case OutputChannel::AccessoryMotorola:
      return {Motorola::Accessory::addressMin, Motorola::Accessory::addressMax};

    case OutputChannel::Output:
    case OutputChannel::Accessory:
    case OutputChannel::Turnout:
      break;

    case OutputChannel::ECoSObject:
      return noAddressMinMax;
  }
  assert(false);
  return {0, 0};
}

std::pair<tcb::span<const uint16_t>, tcb::span<const std::string>> OutputController::getOutputECoSObjects(OutputChannel /*channel*/) const
{
  assert(false);
  return {{}, {}};
}

bool OutputController::isOutputChannel(OutputChannel channel) const
{
  const auto channels = outputChannels();
  // FIXME: return std::find(channels.begin(), channels.end(), channel) != channels.end();
  for(auto ch : channels)
  {
    if(channel == ch)
    {
      return true;
    }
  }
  return false;
}

bool OutputController::isOutputId(OutputChannel channel, uint32_t id) const
{
  switch(channel)
  {
    case OutputChannel::AccessoryDCC:
    case OutputChannel::DCCext:
    case OutputChannel::AccessoryMotorola:
    case OutputChannel::Output:
    case OutputChannel::Accessory:
    case OutputChannel::Turnout:
      return inRange(id, outputAddressMinMax(channel)); // id == address

    case OutputChannel::ECoSObject:
      assert(false);
      break;
  }
  return false;
}

bool OutputController::isOutputAvailable(OutputChannel channel, uint32_t id) const
{
  assert(isOutputChannel(channel));
  return
    isOutputId(channel, id) &&
    m_outputs.find({channel, id}) == m_outputs.end();
}

uint32_t OutputController::getUnusedOutputAddress(OutputChannel channel) const
{
  assert(isOutputChannel(channel));
  const auto end = m_outputs.cend();
  const auto range = outputAddressMinMax(channel);
  for(uint32_t address = range.first; address < range.second; address++)
    if(m_outputs.find({channel, address}) == end)
      return address;
  return AddressOutput::invalidAddress;
}

std::shared_ptr<Output> OutputController::getOutput(OutputChannel channel, uint32_t id, Object& usedBy)
{
  if(!isOutputChannel(channel) || !isOutputId(channel, id))
  {
    return {};
  }

  // Check if already exists:
  if(auto it = m_outputs.find({channel, id}); it != m_outputs.end())
  {
    it->second->m_usedBy.emplace(usedBy.shared_from_this());
    return it->second;
  }

  // Create new output:
  std::shared_ptr<Output> output;
  switch(outputType(channel))
  {
    case OutputType::Single:
      output = std::make_shared<SingleOutput>(shared_ptr(), channel, id);
      break;

    case OutputType::Pair:
      output = std::make_shared<PairOutput>(shared_ptr(), channel, id);
      break;

    case OutputType::Aspect:
      output = std::make_shared<AspectOutput>(shared_ptr(), channel, id);
      break;

    case OutputType::ECoSState:
      output = std::make_shared<ECoSStateOutput>(shared_ptr(), channel, id);
      break;
  }
  assert(output);
  output->m_usedBy.emplace(usedBy.shared_from_this());
  m_outputs.emplace(OutputMapKey{channel, id}, output);
  outputs->addObject(output);
  getWorld(outputs.object()).outputs->addObject(output);

  if(auto keyboard = m_outputKeyboards[channel].lock())
  {
    keyboard->fireOutputUsedChanged(id, true);
  }

  return output;
}

void OutputController::releaseOutput(Output& output, Object& usedBy)
{
  auto outputShared = output.shared_ptr<Output>();
  output.m_usedBy.erase(usedBy.shared_from_this());
  if(output.m_usedBy.empty())
  {
    const auto channel = output.channel.value();
    const auto id = output.id();

    m_outputs.erase({channel, id});
    outputs->removeObject(outputShared);
    getWorld(outputs.object()).outputs->removeObject(outputShared);
    outputShared->destroy();
    outputShared.reset();

    if(auto keyboard = m_outputKeyboards[channel].lock())
    {
      keyboard->fireOutputUsedChanged(id, false);
    }
  }
}

void OutputController::updateOutputValue(OutputChannel channel, uint32_t id, OutputValue value)
{
  assert(isOutputChannel(channel));
  if(auto it = m_outputs.find({channel, id}); it != m_outputs.end())
  {
    if(auto* single = dynamic_cast<SingleOutput*>(it->second.get()))
    {
      single->updateValue(std::get<TriState>(value));
    }
    else if(auto* pair = dynamic_cast<PairOutput*>(it->second.get()))
    {
      pair->updateValue(std::get<OutputPairValue>(value));
    }
    else if(auto* aspect = dynamic_cast<AspectOutput*>(it->second.get()))
    {
      aspect->updateValue(std::get<int16_t>(value));
    }
    else if(auto* ecosState = dynamic_cast<ECoSStateOutput*>(it->second.get()))
    {
      ecosState->updateValue(std::get<uint8_t>(value));
    }
  }

  if(auto keyboard = m_outputKeyboards[channel].lock())
  {
    keyboard->fireOutputValueChanged(id, value);
  }
}

bool OutputController::hasOutputKeyboard(OutputChannel channel) const
{
  assert(isOutputChannel(channel));
  switch(outputType(channel))
  {
    case OutputType::Single:
    case OutputType::Pair:
      return true;

    case OutputType::Aspect:
    case OutputType::ECoSState:
      return false;
  }
  assert(false);
  return false;
}

std::shared_ptr<OutputKeyboard> OutputController::outputKeyboard(OutputChannel channel)
{
  assert(isOutputChannel(channel));
  auto keyboard = m_outputKeyboards[channel].lock();
  if(!keyboard)
  {
    switch(outputType(channel))
    {
      case OutputType::Single:
        keyboard = std::make_shared<SingleOutputKeyboard>(*this, channel);
        break;

      case OutputType::Pair:
        keyboard = std::make_shared<PairOutputKeyboard>(*this, channel);
        break;

      case OutputType::Aspect: /*[[unlikely]]*/
      case OutputType::ECoSState: /*[[unlikely]]*/
        break; // not supported (yet)
    }
    assert(keyboard);
    m_outputKeyboards[channel] = keyboard;
  }
  return keyboard;
}

void OutputController::addToWorld(OutputListColumn columns)
{
  auto& object = interface();
  outputs.setValueInternal(std::make_shared<OutputList>(object, outputs.name(), columns));
  object.world().outputControllers->add(std::dynamic_pointer_cast<OutputController>(object.shared_from_this()));
}

void OutputController::destroying()
{
  auto& object = interface();
  while(!outputs->empty())
  {
    const auto& output = outputs->front();
    assert(output->interface.value() == std::dynamic_pointer_cast<OutputController>(object.shared_from_this()));
    output->interface.setValueInternal(nullptr);
    outputs->removeObject(output);
  }
  object.world().outputControllers->remove(std::dynamic_pointer_cast<OutputController>(object.shared_from_this()));
}

IdObject& OutputController::interface()
{
  auto* object = dynamic_cast<IdObject*>(this);
  assert(object);
  return *object;
}

std::shared_ptr<OutputController> OutputController::shared_ptr()
{
  auto self = std::dynamic_pointer_cast<OutputController>(outputs.object().shared_from_this());
  assert(self);
  return self;
}
