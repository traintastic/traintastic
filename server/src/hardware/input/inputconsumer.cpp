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
#include "../../core/eventloop.hpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../utils/category.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../utils/valuestep.hpp"
#include "../../world/world.hpp"

namespace {

bool roundToDelayStep(uint16_t& value)
{
  value = valueStepRound(value, InputConsumer::delayStep);
  return true;
}

}

InputConsumer::InputConsumer(Object& object, const World& world)
  : m_object{object}
  , m_inputFilterTimer{EventLoop::ioContext}
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
  , onDelay{&object, "on_delay", delayMin, PropertyFlags::ReadWrite | PropertyFlags::Store, nullptr, roundToDelayStep}
  , offDelay{&object, "off_delay", delayMin, PropertyFlags::ReadWrite | PropertyFlags::Store, nullptr, roundToDelayStep}
{
  const auto worldState = world.state.value();
  const bool editable = contains(worldState, WorldState::Edit);
  const bool editableAndStopped = editable && !contains(worldState, WorldState::Run);

  Attributes::addCategory(interface, Category::input);
  Attributes::addDisplayName(interface, DisplayName::Hardware::interface);
  Attributes::addEnabled(interface, editableAndStopped);
  Attributes::addObjectList(interface, world.inputControllers);

  Attributes::addCategory(channel, Category::input);
  Attributes::addDisplayName(channel, DisplayName::Hardware::channel);
  Attributes::addEnabled(channel, editableAndStopped);
  Attributes::addValues(channel, std::span<const InputChannel>());
  Attributes::addVisible(channel, false);

  Attributes::addCategory(address, Category::input);
  Attributes::addDisplayName(address, DisplayName::Hardware::address);
  Attributes::addEnabled(address, editableAndStopped);
  Attributes::addVisible(address, false);
  Attributes::addMinMax<uint32_t>(address, Input::addressMinDefault, Input::addressMaxDefault);

  Attributes::addCategory(onDelay, Category::input);
  Attributes::addDisplayName(onDelay, "input:on_delay");
  Attributes::addEnabled(onDelay, editable);
  Attributes::addMinMax(onDelay, delayMin, delayMax);
  Attributes::addStep(onDelay, delayStep);
  Attributes::addUnit(onDelay, delayUnit);
  Attributes::addVisible(onDelay, false);

  Attributes::addCategory(offDelay, Category::input);
  Attributes::addDisplayName(offDelay, "input:off_delay");
  Attributes::addEnabled(offDelay, editable);
  Attributes::addMinMax(offDelay, delayMin, delayMax);
  Attributes::addStep(offDelay, delayStep);
  Attributes::addUnit(offDelay, delayUnit);
  Attributes::addVisible(offDelay, false);
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
  const bool editable = contains(worldState, WorldState::Edit);
  const bool editableAndStopped = editable && !contains(worldState, WorldState::Run);

  Attributes::setEnabled(interface, editableAndStopped);
  Attributes::setEnabled(channel, editableAndStopped);
  Attributes::setEnabled(address, editableAndStopped);
  Attributes::setEnabled(onDelay, editable);
  Attributes::setEnabled(offDelay, editable);
}

void InputConsumer::addInterfaceItems(InterfaceItems& items)
{
  items.add(interface);
  items.add(channel);
  items.add(address);
  items.add(onDelay);
  items.add(offDelay);
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
    m_inputValueChanged = m_input->onValueChanged.connect(
      [this](bool inputValue, const std::shared_ptr<Input>& /*input*/)
      {
        {
          boost::system::error_code ec;
          m_inputFilterTimer.cancel(ec);
        }
        m_inputFilterTimer.expires_after(std::chrono::milliseconds(inputValue ? onDelay.value() : offDelay.value()));
        m_inputFilterTimer.async_wait(
          [this, inputValue](const boost::system::error_code& ec)
          {
            if(ec == boost::asio::error::operation_aborted)
              return;

            inputValueChanged(inputValue, m_input);
          });
      });

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
    boost::system::error_code ec;
    m_inputFilterTimer.cancel(ec);
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
  Attributes::setVisible({address, offDelay, onDelay}, interface);

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
