/**
 * server/src/hardware/output/map/outputmap.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023-2025 Reinder Feenstra
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

#include "outputmap.hpp"
#include <cassert>
#include "outputmapitem.hpp"
#include "outputmapsingleoutputaction.hpp"
#include "outputmappairoutputaction.hpp"
#include "outputmapaspectoutputaction.hpp"
#include "outputmapecosstateoutputaction.hpp"
#include "../outputcontroller.hpp"
#include "../../interface/interface.hpp"
#include "../../../core/attributes.hpp"
#include "../../../core/method.tpp"
#include "../../../core/objectproperty.tpp"
#include "../../../core/objectvectorproperty.tpp"
#include "../../../world/getworld.hpp"
#include "../../../world/worldloader.hpp"
#include "../../../world/worldsaver.hpp"
#include "../../../utils/displayname.hpp"
#include "../../../utils/inrange.hpp"

namespace
{

template<typename T>
void swap(Property<T>& a, Property<T>& b)
{
  T tmp = a;
  a = b.value();
  b = tmp;
}

}

OutputMap::OutputMap(Object& _parent, std::string_view parentPropertyName)
  : SubObject(_parent, parentPropertyName)
  , parentObject{this, "parent", nullptr, PropertyFlags::Constant | PropertyFlags::NoStore | PropertyFlags::NoScript}
  , interface{this, "interface", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript,
      [this](const std::shared_ptr<OutputController>& /*newValue*/)
      {
        interfaceChanged();
      },
      [this](const std::shared_ptr<OutputController>& newValue)
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
            assert(!newValue->outputChannels().empty());
            channel.setValueInternal(newValue->outputChannels().front());
          }
          else if(!newValue->isOutputChannel(channel) || newValue->outputType(channel) != interface->outputType(channel))
          {
            // New interface doesn't support channel or channel has different output type.
            const auto channels = newValue->outputChannels();
            const auto it = std::find_if(channels.begin(), channels.end(),
              [&controller=*newValue, outputType=interface->outputType(channel)](OutputChannel outputChannel)
              {
                return controller.outputType(outputChannel) == outputType;
              });

            if(it != channels.end())
            {
              // Found channel with same output type.
              channel.setValueInternal(*it);
            }
            else
            {
              // No channel found with same output type.
              channel.setValueInternal(newValue->outputChannels().front());
              // reset mapping
            }
          }

          Attributes::setMinMax(addresses, newValue->outputAddressMinMax(channel));

          if(!interface) // No interface was assigned.
          {
            assert(addresses.empty());
            assert(m_outputs.empty());

            switch(channel)
            {
              case OutputChannel::Output:
              case OutputChannel::Accessory:
              case OutputChannel::AccessoryDCC:
              case OutputChannel::AccessoryMotorola:
              case OutputChannel::DCCext:
              case OutputChannel::Turnout:
              {
                const uint32_t address = newValue->getUnusedOutputAddress(channel);
                addresses.appendInternal(address);
                addOutput(channel, address, *newValue);
                break;
              }
              case OutputChannel::ECoSObject:
                if(newValue->isOutputId(channel, ecosObject))
                {
                  addOutput(channel, ecosObject, *newValue);
                }
                break;
            }
            updateOutputActions(newValue->outputType(channel));
          }
        }
        else // no interface
        {
          for(auto& it : m_outputs)
          {
            if(it.first) /*[[likely]]*/
            {
              releaseOutput(it);
            }
          }

          m_outputs.clear();
          addresses.clearInternal();
          addressesSizeChanged();
        }
        return true;
      }}
  , channel{this, "channel", OutputChannel::Accessory, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript,
      [this](const OutputChannel newValue)
      {
        // Release outputs for previous channel:
        while(!m_outputs.empty())
        {
          if(m_outputs.back().first) /*[[likely]]*/
          {
            releaseOutput(m_outputs.back());
          }
          m_outputs.pop_back();
        }

        // Get outputs for current channel:
        switch(newValue)
        {
          case OutputChannel::Output:
          case OutputChannel::Accessory:
          case OutputChannel::AccessoryDCC:
          case OutputChannel::AccessoryMotorola:
          case OutputChannel::DCCext:
          case OutputChannel::Turnout:
            ecosObject.setValueInternal(0);
            for(uint32_t address : addresses)
            {
              addOutput(newValue, address);
            }
            break;

          case OutputChannel::ECoSObject:
            addresses.clearInternal();
            if(interface->isOutputId(newValue, ecosObject))
            {
              addOutput(newValue, ecosObject);
            }
            break;
        }

        channelChanged();
      },
      [this](const OutputChannel newValue)
      {
        if(!interface) /*[[unlikely]]*/
        {
          return false;
        }

        // If output type is different reset all actions:
        if(interface->outputType(channel) != interface->outputType(newValue))
        {
          for(const auto& item : items)
          {
            item->outputActions.clearInternal();
          }
        }
        return true;
      }}
  , addresses{*this, "addresses", {}, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript,
      nullptr,
      [this](uint32_t index, uint32_t& value)
      {
        (void)index;

        if(!interface) /*[[unlikely]]*/
        {
          return false;
        }

        if(std::find(addresses.begin(), addresses.end(), value) != addresses.end())
        {
          return false; // Duplicate addresses aren't allowed.
        }

        auto outputConnPair = getOutput(channel, value, *interface);
        if(!outputConnPair.first) /*[[unlikely]]*/
        {
          return false; // Output doesn't exist.
        }

        if(index < m_outputs.size() && m_outputs[index].first)
        {
          releaseOutput(m_outputs[index]);
        }

        m_outputs[index] = outputConnPair;

        return true;
      }}
  , ecosObject{this, "ecos_object", 0, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript,
      nullptr,
      [this](uint16_t& value)
      {
        if(!interface) /*[[unlikely]]*/
        {
          return false;
        }

        if(value == 0 || interface->isOutputId(channel, value)) // 0 = no object
        {
          if(!m_outputs.empty())
          {
            releaseOutput(m_outputs.front());
            m_outputs.clear();
          }
          if(value != 0)
          {
            addOutput(channel, value);
          }
          updateOutputActions();
          return true;
        }

        return false;
      }}
  , items{*this, "items", {}, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
  , addAddress{*this, "add_address", MethodFlags::NoScript,
      [this]()
      {
        if(interface && addresses.size() < addressesSizeMax) /*[[likely]]*/
        {
          assert(!addresses.empty());
          const auto addressRange = interface->outputAddressMinMax(channel);
          if(addresses.size() >= (addressRange.second - addressRange.first + 1)) /*[[unlikely]]*/
          {
            return; // All addresses used.
          }
          const uint32_t address = getUnusedAddress();
          addresses.appendInternal(address);
          addOutput(channel, address);
          addressesSizeChanged();
        }
      }}
  , removeAddress{*this, "remove_address", MethodFlags::NoScript,
      [this]()
      {
        if(interface && addresses.size() > addressesSizeMin) /*[[likely]]*/
        {
          addresses.eraseInternal(addresses.size() - 1);
          if(m_outputs.back().first)
          {
            releaseOutput(m_outputs.back());
          }
          m_outputs.pop_back();
          addressesSizeChanged();
        }
      }}
  , swapOutputs{*this, "swap_outputs", MethodFlags::NoScript,
      [this]()
      {
        if(!interface)
        {
          return;
        }

        switch(interface->outputType(channel))
        {
          case OutputType::Pair:
            if(m_outputs.size() == 1 && items.size() == 2)
            {
              swap(static_cast<OutputMapPairOutputAction&>(*items[0]->outputActions[0]).action, static_cast<OutputMapPairOutputAction&>(*items[1]->outputActions[0]).action);
            }
            break;

          default:
            break;
        }
      }}
{
  auto& world = getWorld(&_parent);
  const bool editable = contains(world.state.value(), WorldState::Edit);

  m_interfaceItems.add(parentObject);

  Attributes::addDisplayName(interface, DisplayName::Hardware::interface);
  Attributes::addEnabled(interface, editable);
  Attributes::addObjectList(interface, world.outputControllers);
  m_interfaceItems.add(interface);

  Attributes::addDisplayName(channel, DisplayName::Hardware::channel);
  Attributes::addEnabled(channel, editable);
  Attributes::addValues(channel, std::span<const OutputChannel>());
  Attributes::addVisible(channel, false);
  m_interfaceItems.add(channel);

  Attributes::addDisplayName(addresses, DisplayName::Hardware::address);
  Attributes::addEnabled(addresses, editable);
  Attributes::addVisible(addresses, false);
  Attributes::addMinMax<uint32_t>(addresses, 0, 0);
  m_interfaceItems.add(addresses);

  Attributes::addAliases(ecosObject, std::span<const uint16_t>{}, std::span<const std::string>{});
  Attributes::addDisplayName(ecosObject, "output_map:ecos_object");
  Attributes::addEnabled(ecosObject, editable);
  Attributes::addValues(ecosObject, std::span<const uint16_t>{});
  Attributes::addVisible(ecosObject, false);
  m_interfaceItems.add(ecosObject);

  m_interfaceItems.add(items);

  Attributes::addDisplayName(addAddress, "output_map:add_address");
  Attributes::addEnabled(addAddress, false);
  Attributes::addVisible(addAddress, false);
  m_interfaceItems.add(addAddress);

  Attributes::addDisplayName(removeAddress, "output_map:remove_address");
  Attributes::addEnabled(removeAddress, false);
  Attributes::addVisible(removeAddress, false);
  m_interfaceItems.add(removeAddress);

  Attributes::addDisplayName(swapOutputs, "output_map:swap_outputs");
  Attributes::addVisible(swapOutputs, false);
  m_interfaceItems.add(swapOutputs);

  updateEnabled();
}

OutputMap::~OutputMap() = default;

void OutputMap::load(WorldLoader& loader, const nlohmann::json& data)
{
  SubObject::load(loader, data);

  if(interface)
  {
    switch(channel)
    {
      case OutputChannel::Output:
      case OutputChannel::Accessory:
      case OutputChannel::AccessoryDCC:
      case OutputChannel::AccessoryMotorola:
      case OutputChannel::DCCext:
      case OutputChannel::Turnout:
        for(uint32_t address : addresses)
        {
          addOutput(channel, address);
        }
        break;

      case OutputChannel::ECoSObject:
        if(interface->isOutputId(channel, ecosObject))
        {
          addOutput(channel, ecosObject);
        }
        break;
    }

    updateOutputActions();
  }
}

void OutputMap::loaded()
{
  if(interface)
  {
    interfaceChanged();
  }
  SubObject::loaded();
}

void OutputMap::worldEvent(WorldState state, WorldEvent event)
{
  SubObject::worldEvent(state, event);

  updateEnabled();
}

void OutputMap::interfaceChanged()
{
  const auto outputChannels = interface ? interface->outputChannels() : std::span<const OutputChannel>{};
  Attributes::setValues(channel, outputChannels);
  Attributes::setVisible(channel, interface);

  m_outputECoSObjectsChanged.disconnect();

  if(std::find(outputChannels.begin(), outputChannels.end(), OutputChannel::ECoSObject) != outputChannels.end())
  {
    m_outputECoSObjectsChanged = interface->outputECoSObjectsChanged.connect(
      [this]()
      {
        const auto aliases = interface->getOutputECoSObjects(OutputChannel::ECoSObject);
        Attributes::setAliases(ecosObject, aliases.first, aliases.second);
        Attributes::setValues(ecosObject, aliases.first);
      });
  }

  channelChanged();
}

void OutputMap::channelChanged()
{
  if(interface)
  {
    switch(channel.value())
    {
      case OutputChannel::Output:
      case OutputChannel::Accessory:
      case OutputChannel::AccessoryDCC:
      case OutputChannel::AccessoryMotorola:
      case OutputChannel::DCCext:
      case OutputChannel::Turnout:
      {
        Attributes::setVisible({addresses, addAddress, removeAddress}, true);
        Attributes::setVisible(ecosObject, false);
        Attributes::setAliases(ecosObject, std::span<const uint16_t>{}, std::span<const std::string>{});
        Attributes::setValues(ecosObject, std::span<const uint16_t>{});

        if(addresses.empty())
        {
          const auto address = interface->getUnusedOutputAddress(channel);
          addresses.appendInternal(address);
          addOutput(channel, address);
          updateOutputActions();
        }

        const auto addressRange = interface->outputAddressMinMax(channel);
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
        break;
      }
      case OutputChannel::ECoSObject:
      {
        Attributes::setVisible({addresses, addAddress, removeAddress}, false);
        Attributes::setVisible(ecosObject, true);
        const auto aliases = interface->getOutputECoSObjects(channel);
        Attributes::setAliases(ecosObject, aliases.first, aliases.second);
        Attributes::setValues(ecosObject, aliases.first);

        updateOutputActions();
        break;
      }
    }
  }
  else
  {
    Attributes::setVisible({addresses, addAddress, removeAddress, ecosObject}, false);
    Attributes::setMinMax(addresses, std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());
    Attributes::setAliases(ecosObject, std::span<const uint16_t>{}, std::span<const std::string>{});
    Attributes::setValues(ecosObject, std::span<const uint16_t>{});
  }
}

void OutputMap::addressesSizeChanged()
{
  Attributes::setDisplayName(addresses, addresses.size() == 1 ? DisplayName::Hardware::address : DisplayName::Hardware::addresses);
  assert(addresses.size() == m_outputs.size());

  updateOutputActions();

  updateEnabled();
}

void OutputMap::updateOutputActions()
{
  assert(interface);
  updateOutputActions(interface->outputType(channel));
}

void OutputMap::updateOutputActions(OutputType outputType)
{
  for(const auto& item : items)
  {
    while(m_outputs.size() > item->outputActions.size())
    {
      std::shared_ptr<OutputMapOutputAction> outputAction = createOutputAction(outputType, item->outputActions.size(), getDefaultOutputActionValue(*item, outputType, item->outputActions.size()));
      assert(outputAction);
      item->outputActions.appendInternal(outputAction);
    }

    while(m_outputs.size() < item->outputActions.size())
    {
      item->outputActions.back()->destroy();
      item->outputActions.removeInternal(item->outputActions.back());
    }

    assert(m_outputs.size() == item->outputActions.size());
  }

  switch(outputType)
  {
    case OutputType::Pair:
      Attributes::setVisible(swapOutputs, m_outputs.size() == 1 && items.size() == 2);
      break;

    default:
      Attributes::setVisible(swapOutputs, false);
      break;
  }
}

void OutputMap::updateEnabled()
{
  const bool editable = contains(getWorld(parent()).state.value(), WorldState::Edit);

  Attributes::setEnabled(interface, editable);
  Attributes::setEnabled(channel, editable);
  Attributes::setEnabled(addresses, editable);
  Attributes::setEnabled(addAddress, editable && addresses.size() < addressesSizeMax);
  Attributes::setEnabled(removeAddress, editable && addresses.size() > addressesSizeMin);
  Attributes::setEnabled(ecosObject, editable);
}

uint32_t OutputMap::getUnusedAddress() const
{
  assert(interface);
  const auto addressRange = interface->outputAddressMinMax(channel);
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

std::shared_ptr<OutputMapOutputAction> OutputMap::createOutputAction(OutputType outputType, size_t index, std::optional<OutputActionValue> actionValue)
{
  switch(outputType)
  {
    case OutputType::Single:
    {
      auto singleOutputAction = std::make_shared<OutputMapSingleOutputAction>(*this, index);
      if(actionValue)
      {
        singleOutputAction->action.setValueInternal(std::get<SingleOutputAction>(*actionValue));
      }
      return singleOutputAction;
    }
    case OutputType::Pair:
    {
      auto pairOutputAction = std::make_shared<OutputMapPairOutputAction>(*this, index);
      if(actionValue)
      {
        pairOutputAction->action.setValueInternal(std::get<PairOutputAction>(*actionValue));
      }
      return pairOutputAction;
    }
    case OutputType::Aspect:
    {
      auto aspectOutputAction = std::make_shared<OutputMapAspectOutputAction>(*this, index);
      if(actionValue)
      {
        aspectOutputAction->aspect.setValueInternal(std::get<int16_t>(*actionValue));
      }
      return aspectOutputAction;
    }
    case OutputType::ECoSState:
    {
      auto ecosStateOutputAction = std::make_shared<OutputMapECoSStateOutputAction>(*this, index);
      if(actionValue)
      {
        ecosStateOutputAction->state.setValueInternal(std::get<int16_t>(*actionValue));
      }
      return ecosStateOutputAction;
    }
  }
  assert(false);
  return {};
}

int OutputMap::getMatchingActionOnCurrentState()
{
  int i = 0;
  int wildcardIdx = -1;

  for(const auto& item : items)
  {
    OutputMapItem::MatchResult value = item->matchesCurrentOutputState();
    if(value == OutputMapItem::MatchResult::FullMatch)
    {
      return i; // We got a full match
    }
    if(value == OutputMapItem::MatchResult::WildcardMatch)
    {
      // We give wildcard matches a lower priority.
      // Save it for later, in the meantime we check for a better full match
      if(wildcardIdx == -1)
        wildcardIdx = i;
    }

    i++;
  }

  // No full match, do we have a wildcard match?
  if(wildcardIdx != -1)
    return wildcardIdx;

  return -1; // No match found
}

void OutputMap::updateStateFromOutput()
{
  // Default implementation is no-op
}

void OutputMap::addOutput(OutputChannel ch, uint32_t id)
{
  addOutput(ch, id, *interface);
}

void OutputMap::addOutput(OutputChannel ch, uint32_t id, OutputController& outputController)
{
  m_outputs.emplace_back(getOutput(ch, id, outputController));
  assert(m_outputs.back().first);
}

OutputMap::OutputConnectionPair OutputMap::getOutput(OutputChannel ch, uint32_t id, OutputController& outputController)
{
  auto output = outputController.getOutput(ch, id, parent());
  if(!output)
    return {};

  boost::signals2::connection conn = output->onValueChangedGeneric.connect([this](const std::shared_ptr<Output>&){ updateStateFromOutput(); });
  return {output, conn};
}

void OutputMap::releaseOutput(OutputConnectionPair &outputConnPair)
{
  outputConnPair.second.disconnect();
  interface->releaseOutput(*outputConnPair.first, parent());
}
