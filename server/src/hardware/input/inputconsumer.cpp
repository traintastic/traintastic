/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#include "inputconsumer.hpp"
#include "input.hpp"
#include "inputcontroller.hpp"
#include "../../core/attributes.hpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../world/world.hpp"

InputConsumer::InputConsumer(Object& object, const World& world)
  : m_object{object}
  , interface{&object, "interface", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript,
      [this](const std::shared_ptr<InputController>& /*newValue*/)
      {
        interfaceChanged();
      },
      [this](const std::shared_ptr<InputController>& newValue)
      {
        if(interface.value())
        {
          releaseInput();
        }

        if(newValue)
        {
          if(!newValue->isInputChannel(channel))
          {
            channel.setValueInternal(newValue->inputChannels().front());
          }

          if(auto addressMinMax = newValue->inputAddressMinMax(channel); !inRange(address.value(), addressMinMax))
          {
            const auto addr = newValue->getUnusedInputAddress(channel);
            address.setValueInternal(addr ? *addr : addressMinMax.first);
          }

          setInput(newValue->getInput(channel, address, m_object));
        }

        return true;
      }}
  , channel{&object, "channel", InputChannel::Input, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript,
      [this](const InputChannel& /*newValue*/)
      {
        channelChanged();
      },
      [this](const InputChannel& newValue)
      {
        if(auto obj = interface->getInput(newValue, address, m_object))
        {
          setInput(obj);
          return true;
        }
        return false;
      }}
  , address{&object, "address", 0, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript, nullptr,
      [this](const uint32_t& newValue)
      {
        if(auto obj = interface->getInput(channel, newValue, m_object))
        {
          setInput(obj);
          return true;
        }
        return false;
      }}
{
  const auto worldState = world.state.value();
  const bool editableAndStopped = contains(worldState, WorldState::Edit) && !contains(worldState, WorldState::Run);

  Attributes::addDisplayName(interface, DisplayName::Hardware::interface);
  Attributes::addEnabled(interface, editableAndStopped);
  Attributes::addObjectList(interface, world.inputControllers);

  Attributes::addDisplayName(channel, DisplayName::Hardware::channel);
  Attributes::addEnabled(channel, editableAndStopped);
  Attributes::addValues(channel, std::span<const InputChannel>());
  Attributes::addVisible(channel, false);

  Attributes::addDisplayName(address, DisplayName::Hardware::address);
  Attributes::addEnabled(address, editableAndStopped);
  Attributes::addVisible(address, false);
  Attributes::addMinMax<uint32_t>(address, Input::addressMinDefault, Input::addressMaxDefault);
}

InputConsumer::~InputConsumer()
{
  releaseInput();
}

void InputConsumer::loaded()
{
  if(interface)
  {
    if(auto object = interface->getInput(channel, address, m_object))
    {
      setInput(object);
      interfaceChanged();
    }
    else
    {
      interface.setValueInternal(nullptr);
      //! \todo log warning
    }
  }
}

void InputConsumer::worldEvent(WorldState worldState, WorldEvent /*worldEvent*/)
{
  const bool editableAndStopped = contains(worldState, WorldState::Edit) && !contains(worldState, WorldState::Run);

  Attributes::setEnabled(interface, editableAndStopped);
  Attributes::setEnabled(channel, editableAndStopped);
  Attributes::setEnabled(address, editableAndStopped);
}

void InputConsumer::setInput(std::shared_ptr<Input> value)
{
  releaseInput();
  assert(!m_input);
  m_input = value;
  if(m_input)
  {
    m_inputDestroying = m_input->onDestroying.connect(
      [this](Object& object)
      {
        (void)object; // silence unused warning
        assert(m_input.get() == &object);
        interface.setValue(nullptr);
      });
    m_inputValueChanged = m_input->onValueChanged.connect(std::bind_front(&InputConsumer::inputValueChanged, this));
    if(m_input->value != TriState::Undefined)
    {
      inputValueChanged(m_input->value == TriState::True, m_input);
    }
  }
}

void InputConsumer::releaseInput()
{
  if(m_input)
  {
    m_inputDestroying.disconnect();
    m_inputValueChanged.disconnect();
    if(m_input->interface)
    {
      m_input->interface->releaseInput(*m_input, m_object);
    }
    m_input.reset();
  }
}

void InputConsumer::interfaceChanged()
{
  Attributes::setValues(channel, interface ? interface->inputChannels() : std::span<const InputChannel>());
  Attributes::setVisible(channel, interface && interface->inputChannels().size() > 1);
  Attributes::setVisible(address, interface);

  channelChanged();
}

void InputConsumer::channelChanged()
{
  if(interface)
  {
    const auto limits = interface->inputAddressMinMax(channel);
    Attributes::setMinMax(address, limits.first, limits.second);
  }
  else
  {
    Attributes::setMinMax(address, Input::addressMinDefault, Input::addressMaxDefault);
  }
}
