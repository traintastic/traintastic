/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "feedbackmap.hpp"
#include "feedbackmapitem.hpp"
#include "feedbackmapinputcondition.hpp"
#include "../input.hpp"
#include "../inputcontroller.hpp"
#include "../../../core/attributes.hpp"
#include "../../../core/method.tpp"
#include "../../../core/objectproperty.tpp"
#include "../../../core/objectvectorproperty.tpp"
#include "../../../log/log.hpp"
#include "../../../utils/displayname.hpp"
#include "../../../utils/inrange.hpp"
#include "../../../world/getworld.hpp"

FeedbackMap::FeedbackMap(Object& _parent, std::string_view parentPropertyName)
  : SubObject(_parent, parentPropertyName)
  , parentObject{this, "parent", nullptr, PropertyFlags::Constant | PropertyFlags::NoStore | PropertyFlags::NoScript}
  , interface{this, "interface", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript,
      [this](const std::shared_ptr<InputController>& /*newValue*/)
      {
        interfaceChanged();
      },
      [this](const std::shared_ptr<InputController>& newValue)
      {
        m_interfaceDestroying.disconnect();

        if(newValue)
        {
          if(auto* object = dynamic_cast<Object*>(newValue.get())) /*[[likely]]*/
          {
            m_interfaceDestroying = object->onDestroying.connect(
              [this](Object& /*object*/)
              {
                interface = nullptr;
              });
          }

          if(!interface)
          {
            // No interface was assigned.
            assert(!newValue->inputChannels().empty());
            channel.setValueInternal(newValue->inputChannels().front());
          }
          else if(!newValue->isInputChannel(channel))
          {
            // New interface doesn't support channel, use the first one.
            channel.setValueInternal(newValue->inputChannels().front());
          }

          Attributes::setMinMax(addresses, newValue->inputAddressMinMax(channel));

          if(!interface) // No interface was assigned.
          {
            assert(addresses.empty());
            assert(m_inputs.empty());

            if(addresses.empty())
            {
              uint32_t address;
              if(auto unusedAddress = newValue->getUnusedInputAddress(channel))
              {
                address = *unusedAddress;
              }
              else
              {
                address = interface->inputAddressMinMax(channel).first;
              }
              addresses.appendInternal(address);
              addInput(channel, inputLocation(channel, node, address), *newValue);
              updateInputConditions();
            }
          }
        }
        else // no interface
        {
          for(auto& it : m_inputs)
          {
            if(it.first) /*[[likely]]*/
            {
              releaseInput(it);
            }
          }

          m_inputs.clear();
          addresses.clearInternal();
          addressesSizeChanged();
        }
        return true;
      }}
  , channel{this, "channel", InputChannel::Input, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript,
      [this](const InputChannel newValue)
      {
        // Release inputs for previous channel:
        releaseInputs(m_inputs);

        // Get inputs for new channel:
        for(uint32_t address : addresses)
        {
          addInput(newValue, inputLocation(newValue, node, address));
        }

        channelChanged();
      }}
  , node{this, "node", 0, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript,
      nullptr,
      [this](uint32_t& value)
      {
        if(!interface) [[unlikely]]
        {
          return false;
        }

        Inputs newInputs;
        for(auto address : addresses)
        {
          auto inputConnPair = getInput(channel, InputNodeAddress(value, address), *interface);
          if(inputConnPair.first) [[likely]]
          {
            newInputs.emplace_back(inputConnPair);
          }
        }

        if(newInputs.size() != m_inputs.size())
        {
          releaseInputs(newInputs);
          assert(newInputs.empty());
          return false;
        }

        releaseInputs(m_inputs);
        assert(m_inputs.empty());
        m_inputs = std::move(newInputs);
        return true;
      }}
  , addresses{*this, "addresses", {}, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript,
      nullptr,
      [this](uint32_t index, uint32_t& value)
      {
        (void)index;

        if(!interface) [[unlikely]]
        {
          return false;
        }

        if(std::find(addresses.begin(), addresses.end(), value) != addresses.end())
        {
          return false; // Duplicate addresses aren't allowed.
        }

        auto inputConnPair = getInput(channel, inputLocation(channel, node, value), *interface);
        if(!inputConnPair.first) [[unlikely]]
        {
          return false; // Input doesn't exist.
        }

        if(index < m_inputs.size() && m_inputs[index].first)
        {
          releaseInput(m_inputs[index]);
        }

        m_inputs[index] = inputConnPair;

        return true;
      }}
  , items{*this, "items", {}, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
  , addAddress{*this, "add_address", MethodFlags::NoScript,
      [this]()
      {
        if(interface && addresses.size() < addressesSizeMax) [[likely]]
        {
          assert(!addresses.empty());
          const auto addressRange = interface->inputAddressMinMax(channel);
          if(addresses.size() >= (addressRange.second - addressRange.first + 1)) [[unlikely]]
          {
            return; // All addresses used.
          }
          const uint32_t address = getUnusedAddress();
          addresses.appendInternal(address);
          addInput(channel, inputLocation(channel, node, address));
          addressesSizeChanged();
        }
      }}
  , removeAddress{*this, "remove_address", MethodFlags::NoScript,
      [this]()
      {
        if(interface && addresses.size() > addressesSizeMin) [[likely]]
        {
          addresses.eraseInternal(addresses.size() - 1);
          if(m_inputs.back().first)
          {
            releaseInput(m_inputs.back());
          }
          m_inputs.pop_back();
          addressesSizeChanged();
        }
      }}
{
  auto& world = getWorld(&_parent);
  const bool editable = contains(world.state.value(), WorldState::Edit);

  m_interfaceItems.add(parentObject);

  Attributes::addDisplayName(interface, DisplayName::Hardware::interface);
  Attributes::addEnabled(interface, editable);
  Attributes::addObjectList(interface, world.inputControllers);
  m_interfaceItems.add(interface);

  Attributes::addDisplayName(node, DisplayName::Hardware::node);
  Attributes::addEnabled(node, editable);
  Attributes::addVisible(node, false);
  Attributes::addMinMax<uint32_t>(node, 0, 0);
  m_interfaceItems.add(node);

  Attributes::addDisplayName(channel, DisplayName::Hardware::channel);
  Attributes::addEnabled(channel, editable);
  Attributes::addValues(channel, std::span<const InputChannel>());
  Attributes::addVisible(channel, false);
  m_interfaceItems.add(channel);

  Attributes::addDisplayName(addresses, DisplayName::Hardware::address);
  Attributes::addEnabled(addresses, editable);
  Attributes::addVisible(addresses, false);
  Attributes::addMinMax<uint32_t>(addresses, 0, 0);
  m_interfaceItems.add(addresses);

  m_interfaceItems.add(items);

  Attributes::addDisplayName(addAddress, DisplayName::OutputMap::addAddress);
  Attributes::addEnabled(addAddress, false);
  Attributes::addVisible(addAddress, false);
  m_interfaceItems.add(addAddress);

  Attributes::addDisplayName(removeAddress, DisplayName::OutputMap::removeAddress);
  Attributes::addEnabled(removeAddress, false);
  Attributes::addVisible(removeAddress, false);
  m_interfaceItems.add(removeAddress);

  updateEnabled();
}

FeedbackMap::~FeedbackMap() = default;

void FeedbackMap::load(WorldLoader& loader, const nlohmann::json& data)
{
  SubObject::load(loader, data);

  if(interface)
  {
    for(uint32_t address : addresses)
    {
      addInput(channel, inputLocation(channel, node, address));
    }
    updateInputConditions();
  }
}

void FeedbackMap::loaded()
{
  if(interface)
  {
    interfaceChanged();
  }
  SubObject::loaded();
}

void FeedbackMap::worldEvent(WorldState state, WorldEvent event)
{
  SubObject::worldEvent(state, event);
  updateEnabled();
}

void FeedbackMap::interfaceChanged()
{
  const auto inputChannels = interface ? interface->inputChannels() : std::span<const InputChannel>{};
  Attributes::setValues(channel, inputChannels);
  Attributes::setVisible(channel, interface);
  channelChanged();
}

void FeedbackMap::channelChanged()
{
  if(interface)
  {
    Attributes::setVisible({addresses, addAddress, removeAddress}, true);
    Attributes::setVisible(node, hasNodeAddressLocation(channel));

    if(hasNodeAddressLocation(channel))
    {
      Attributes::setMinMax<uint32_t>(node, 0, 65535);// FIXME: interface->inputNodeMinMax(channel));
    }

    const auto addressRange = interface->inputAddressMinMax(channel);
    const uint32_t addressCount = (addressRange.second - addressRange.first + 1);
    Attributes::setMinMax(addresses, addressRange);

    while(addressCount < addresses.size()) // Reduce number of addresses if larger than address space.
    {
      addresses.eraseInternal(addresses.size() - 1);
    }

    // Make sure all addresses are in range:
    for(size_t i = 0; i < addresses.size(); i++)
    {
      if(!inRange(addresses[i], addressRange))
      {
        addresses.setValueInternal(i, getUnusedAddress());
      }
    }

    addressesSizeChanged();
  }
  else
  {
    Attributes::setVisible({node, addresses, addAddress, removeAddress}, false);
    Attributes::setMinMax(node, std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());
    Attributes::setMinMax(addresses, std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());
  }
}

void FeedbackMap::addressesSizeChanged()
{
  assert(addresses.size() == m_inputs.size());
  updateAddressDisplayName();
  updateInputConditions();
  updateEnabled();
}

void FeedbackMap::updateInputConditions()
{
  for(const auto& item : items)
  {
    while(m_inputs.size() > item->inputConditions.size())
    {
      item->inputConditions.appendInternal(std::make_shared<FeedbackMapInputCondition>(*this, item->inputConditions.size()));
    }

    while(m_inputs.size() < item->inputConditions.size())
    {
      item->inputConditions.back()->destroy();
      item->inputConditions.removeInternal(item->inputConditions.back());
    }

    assert(m_inputs.size() == item->inputConditions.size());
  }
}

void FeedbackMap::updateEnabled()
{
  const bool editable = contains(getWorld(parent()).state.value(), WorldState::Edit);

  Attributes::setEnabled(interface, editable);
  Attributes::setEnabled(channel, editable);
  Attributes::setEnabled(node, editable);
  Attributes::setEnabled(addresses, editable);
  Attributes::setEnabled(addAddress, editable && addresses.size() < addressesSizeMax);
  Attributes::setEnabled(removeAddress, editable && addresses.size() > addressesSizeMin);
}

uint32_t FeedbackMap::getUnusedAddress() const
{
  assert(interface);
  const auto addressRange = interface->inputAddressMinMax(channel);
  assert((addressRange.second - addressRange.first + 1) > addresses.size());
  uint32_t address = addresses.empty() ? addressRange.first : addresses.back();
  do
  {
    address++;
    if(!inRange(address, addressRange))
    {
      address = addressRange.first;
    }
  }
  while(std::find(addresses.begin(), addresses.end(), address) != addresses.end());

  return address;
}

void FeedbackMap::addInput(InputChannel ch, const InputLocation& location)
{
  addInput(ch, location, *interface);
}

void FeedbackMap::addInput(InputChannel ch, const InputLocation& location, InputController& inputController)
{
  m_inputs.emplace_back(getInput(ch, location, inputController));
  assert(m_inputs.back().first);
}

FeedbackMap::InputConnectionPair FeedbackMap::getInput(InputChannel ch, const InputLocation& location, InputController& inputController)
{
  auto input = inputController.getInput(ch, location, parent());
  if(!input)
    return {};

  boost::signals2::connection conn = input->onValueChanged.connect(std::bind_front(&FeedbackMap::inputValueChanged, this));

  return {input, conn};
}

void FeedbackMap::releaseInput(InputConnectionPair& inputConnPair)
{
  inputConnPair.second.disconnect();
  interface->releaseInput(*inputConnPair.first, parent());
}

void FeedbackMap::releaseInputs(Inputs& inputs)
{
  while(!inputs.empty())
  {
    if(inputs.back().first) [[likely]]
    {
      releaseInput(inputs.back());
    }
    inputs.pop_back();
  }
}

void FeedbackMap::inputValueChanged(bool value, const std::shared_ptr<Input>& input)
{
  const size_t invalidIndex = std::numeric_limits<size_t>::max();
  size_t inputIndex = invalidIndex;
  for(size_t i = 0; i < m_inputs.size(); ++i)
  {
    if(m_inputs[i].first == input)
    {
      inputIndex = i;
      break;
    }
  }
  if(inputIndex == invalidIndex) [[unlikely]]
  {
    assert(false);
    return;
  }

  const auto condition = value ? InputCondition::On : InputCondition::Off;
  size_t matchCount = 0;
  size_t matchIndex = 0;
  for(size_t i = 0; i < items.size(); ++i)
  {
    if(items[i]->inputConditions[inputIndex]->condition.value() != condition)
    {
      continue; // eliminate when condition does not match
    }

    if(items[i]->matches())
    {
      matchCount++;
      matchIndex = i;
    }
  }

  if(matchCount == 0 && m_lastMatchResult != MatchResult::None)
  {
    m_lastMatchResult = MatchResult::None;
    matchResultChanged(m_lastMatchResult, invalidMatchIndex);
  }
  else if(matchCount == 1 && (m_lastMatchResult != MatchResult::Match || m_lastMatchIndex != matchIndex))
  {
    m_lastMatchResult = MatchResult::Match;
    m_lastMatchIndex = matchIndex;
    matchResultChanged(m_lastMatchResult, m_lastMatchIndex);
  }
  else if(matchCount > 1 && m_lastMatchResult != MatchResult::Conflict) // more than one match -> conflict
  {
    Log::log(parent(), LogMessage::E3011_FEEDBACK_CONFLICT_MULTIPLE_OPTIONS_ARE_VALID);
    m_lastMatchResult = MatchResult::Conflict;
    matchResultChanged(m_lastMatchResult, invalidMatchIndex);
  }
}

void FeedbackMap::updateAddressDisplayName()
{
  switch(channel)
  {
    using enum InputChannel;

    case Input:
    case LocoNet:
    case RBus:
    case S88:
    case S88_Left:
    case S88_Middle:
    case S88_Right:
    case ECoSDetector:
      Attributes::setDisplayName(addresses, addresses.size() == 1 ? DisplayName::Hardware::address : DisplayName::Hardware::addresses);
      Attributes::setDisplayName(addAddress, DisplayName::OutputMap::addAddress);
      Attributes::setDisplayName(removeAddress, DisplayName::OutputMap::removeAddress);
      break;

    case LongEvent:
    case ShortEvent:
      Attributes::setDisplayName(addresses, addresses.size() == 1 ? DisplayName::Hardware::event : DisplayName::Hardware::events);
      Attributes::setDisplayName(addAddress, DisplayName::OutputMap::addEvent);
      Attributes::setDisplayName(removeAddress, DisplayName::OutputMap::removeEvent);
      break;
  }
}
