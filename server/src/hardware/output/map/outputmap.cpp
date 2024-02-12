/**
 * server/src/hardware/output/map/outputmap.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023-2024 Reinder Feenstra
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
#include "../outputcontroller.hpp"
#include "../../../core/attributes.hpp"
#include "../../../core/method.tpp"
#include "../../../core/objectproperty.tpp"
#include "../../../core/objectvectorproperty.tpp"
#include "../../../world/getworld.hpp"
#include "../../../world/worldloader.hpp"
#include "../../../world/worldsaver.hpp"
#include "../../../utils/displayname.hpp"
#include "../../../utils/inrange.hpp"

OutputMap::OutputMap(Object& _parent, std::string_view parentPropertyName)
  : SubObject(_parent, parentPropertyName)
  , interface{this, "interface", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript,
      [this](const std::shared_ptr<OutputController>& /*newValue*/)
      {
        interfaceChanged();
      },
      [this](const std::shared_ptr<OutputController>& newValue)
      {
        if(newValue)
        {
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
            auto it = std::find_if(channels.begin(), channels.end(),
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

          if(!interface) // No interface was assigned.
          {
            assert(addresses.empty());
            assert(m_outputs.empty());
            const uint32_t address = newValue->getUnusedOutputAddress(channel);
            addresses.appendInternal(address);
            m_outputs.emplace_back(newValue->getOutput(channel, address, parent()));
            const auto outputType = newValue->outputType(channel);
            for(auto& item : items)
            {
              item->outputActions.appendInternal(createOutputAction(outputType, 0));
            }
          }
        }
        else // no interface
        {
          for(auto& output : m_outputs)
          {
            if(output)
            {
              interface->releaseOutput(*output, parent());
            }
            m_outputs.clear();
            addresses.clearInternal();
            addressesSizeChanged();
          }
        }
        return true;
      }}
  , channel{this, "channel", OutputChannel::Accessory, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript,
      [this](const OutputChannel newValue)
      {
        // Release outputs for previous channel:
        while(!m_outputs.empty())
        {
          if(m_outputs.back()) /*[[likely]]*/
          {
            interface->releaseOutput(*m_outputs.back(), parent());
          }
          m_outputs.pop_back();
        }

        // Get outputs for current channel:
        for(uint32_t address : addresses)
        {
          m_outputs.emplace_back(interface->getOutput(newValue, address, parent()));
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

        auto output = interface->getOutput(channel, value, parent());
        if(!output) /*[[unlikely]]*/
        {
          return false; // Output doesn't exist.
        }

        if(index < m_outputs.size() && m_outputs[index])
        {
          interface->releaseOutput(*m_outputs[index], parent());
        }

        m_outputs[index] = output;

        return true;
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
          m_outputs.emplace_back(interface->getOutput(channel, address, parent()));
          addressesSizeChanged();
        }
      }}
  , removeAddress{*this, "remove_address", MethodFlags::NoScript,
      [this]()
      {
        if(interface && addresses.size() > addressesSizeMin) /*[[likely]]*/
        {
          addresses.eraseInternal(addresses.size() - 1);
          if(m_outputs.back())
          {
            interface->releaseOutput(*m_outputs.back(), parent());
          }
          m_outputs.pop_back();
          addressesSizeChanged();
        }
      }}
{
  auto& world = getWorld(&_parent);
  const bool editable = contains(world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(interface, DisplayName::Hardware::interface);
  Attributes::addEnabled(interface, editable);
  Attributes::addObjectList(interface, world.outputControllers);
  m_interfaceItems.add(interface);

  Attributes::addDisplayName(channel, DisplayName::Hardware::channel);
  Attributes::addEnabled(channel, editable);
  Attributes::addValues(channel, tcb::span<const OutputChannel>());
  Attributes::addVisible(channel, false);
  m_interfaceItems.add(channel);

  Attributes::addDisplayName(addresses, DisplayName::Hardware::address);
  Attributes::addEnabled(addresses, editable);
  Attributes::addVisible(addresses, false);
  Attributes::addMinMax<uint32_t>(addresses, 0, 0);
  m_interfaceItems.add(addresses);

  m_interfaceItems.add(items);

  Attributes::addEnabled(addAddress, false);
  Attributes::addVisible(addAddress, false);
  m_interfaceItems.add(addAddress);

  Attributes::addEnabled(removeAddress, false);
  Attributes::addVisible(removeAddress, false);
  m_interfaceItems.add(removeAddress);

  updateEnabled();
}

void OutputMap::load(WorldLoader& loader, const nlohmann::json& data)
{
  SubObject::load(loader, data);

  if(interface)
  {
    for(uint32_t address : addresses)
    {
      m_outputs.emplace_back(interface->getOutput(channel, address, parent()));
    }

    const auto outputType = interface->outputType(channel);
    const auto addressCount = addresses.size();
    for(auto& item : items)
    {
      for(size_t i = 0; i < addressCount; i++)
      {
        item->outputActions.appendInternal(createOutputAction(outputType, i));
      }
    }
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
  Attributes::setValues(channel, interface ? interface->outputChannels() : tcb::span<const OutputChannel>{});
  Attributes::setVisible(channel, interface);
  Attributes::setVisible(addresses, interface);
  Attributes::setVisible(addAddress, interface);
  Attributes::setVisible(removeAddress, interface);

  channelChanged();
}

void OutputMap::channelChanged()
{
  if(interface)
  {
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
  }
  else
  {
    Attributes::setMinMax(addresses, std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());
  }
}

void OutputMap::addressesSizeChanged()
{
  Attributes::setDisplayName(addresses, addresses.size() == 1 ? DisplayName::Hardware::address : DisplayName::Hardware::addresses);
  assert(addresses.size() == m_outputs.size());

  const auto outputType = interface->outputType(channel);
  for(const auto& item : items)
  {
    while(addresses.size() > item->outputActions.size())
    {
      std::shared_ptr<OutputMapOutputAction> outputAction = createOutputAction(outputType, item->outputActions.size());
      assert(outputAction);
      item->outputActions.appendInternal(outputAction);
    }

    while(addresses.size() < item->outputActions.size())
    {
      item->outputActions.back()->destroy();
      item->outputActions.removeInternal(item->outputActions.back());
    }

    assert(addresses.size() == item->outputActions.size());
  }

  updateEnabled();
}

void OutputMap::updateEnabled()
{
  const bool editable = contains(getWorld(parent()).state.value(), WorldState::Edit);

  Attributes::setEnabled(interface, editable);
  Attributes::setEnabled(channel, editable);
  Attributes::setEnabled(addresses, editable);
  Attributes::setEnabled(addAddress, editable && addresses.size() < addressesSizeMax);
  Attributes::setEnabled(removeAddress, editable && addresses.size() > addressesSizeMin);
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

std::shared_ptr<OutputMapOutputAction> OutputMap::createOutputAction(OutputType outputType, size_t index)
{
  switch(outputType)
  {
    case OutputType::Single:
      return std::make_shared<OutputMapSingleOutputAction>(*this, index);

    case OutputType::Pair:
      return std::make_shared<OutputMapPairOutputAction>(*this, index);

    case OutputType::Aspect:
      return std::make_shared<OutputMapAspectOutputAction>(*this, index);
  }
  assert(false);
  return {};
}
