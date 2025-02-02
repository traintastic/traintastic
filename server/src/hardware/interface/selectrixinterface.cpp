/**
 * server/src/hardware/interface/selectrixinterface.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023,2025 Reinder Feenstra
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

#include "selectrixinterface.hpp"
#include "../decoder/list/decoderlist.hpp"
#include "../decoder/list/decoderlisttablemodel.hpp"
#include "../input/input.hpp"
#include "../input/list/inputlist.hpp"
#include "../protocol/selectrix/kernel.hpp"
#include "../protocol/selectrix/settings.hpp"
#include "../protocol/selectrix/addresstype.hpp"
#include "../protocol/selectrix/utils.hpp"
#include "../protocol/selectrix/iohandler/serialiohandler.hpp"
#include "../protocol/selectrix/iohandler/simulationiohandler.hpp"
#include "../../core/attributes.hpp"
#include "../../core/method.tpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../world/world.hpp"

constexpr bool operator <(const SelectrixInterface::BusAddress& lhs, const SelectrixInterface::BusAddress& rhs)
{
  return lhs.bus < rhs.bus || (lhs.bus == rhs.bus && lhs.address < rhs.address);
}

constexpr auto decoderListColumns = DecoderListColumn::Id | DecoderListColumn::Name | DecoderListColumn::Address;
constexpr auto inputListColumns = InputListColumn::Id | InputListColumn::Name | InputListColumn::Channel | InputListColumn::Address;

constexpr std::array<uint32_t, 4> baudratesRautenhausSLX825 = {2400, 4800, 9600, 19200};

CREATE_IMPL(SelectrixInterface)

SelectrixInterface::SelectrixInterface(World& world, std::string_view _id)
  : Interface(world, _id)
  , DecoderController(*this, decoderListColumns)
  , InputController(static_cast<IdObject&>(*this))
  , device{this, "device", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , baudrate{this, "baudrate", 9600, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , selectrix{this, "selectrix", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "Selectrix";
  selectrix.setValueInternal(std::make_shared<Selectrix::Settings>(*this, selectrix.name()));

  Attributes::addEnabled(device, !online);
  m_interfaceItems.insertBefore(device, notes);

  Attributes::addDisplayName(baudrate, DisplayName::Serial::baudrate);
  Attributes::addEnabled(baudrate, !online);
  Attributes::addValues(baudrate, baudratesRautenhausSLX825);
  m_interfaceItems.insertBefore(baudrate, notes);

  Attributes::addDisplayName(selectrix, DisplayName::Hardware::selectrix);
  m_interfaceItems.insertBefore(selectrix, notes);

  m_interfaceItems.insertBefore(decoders, notes);

  m_interfaceItems.insertBefore(inputs, notes);
}

tcb::span<const DecoderProtocol> SelectrixInterface::decoderProtocols() const
{
  static constexpr std::array<DecoderProtocol, 1> protocols{DecoderProtocol::Selectrix};
  return tcb::span<const DecoderProtocol>{protocols.data(), protocols.size()};
}

bool SelectrixInterface::isDecoderAddressAvailable(DecoderProtocol protocol, uint16_t address) const
{
  if(!DecoderController::isDecoderAddressAvailable(protocol, address))
  {
    return false;
  }

  return m_usedBusAddresses.find({Selectrix::Bus::SX0, static_cast<uint8_t>(address)}) == m_usedBusAddresses.end();
}

bool SelectrixInterface::changeDecoderProtocolAddress(Decoder& decoder, DecoderProtocol newProtocol, uint16_t newAddress)
{
  if(DecoderController::changeDecoderProtocolAddress(decoder, newProtocol, newAddress))
  {
    useLocomotiveAddress(newAddress);
    unuseLocomotiveAddress(decoder.address);
    return true;
  }
  return false;
}

void SelectrixInterface::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(m_kernel)
    m_kernel->decoderChanged(decoder, changes, functionNumber);
}

std::pair<uint32_t, uint32_t> SelectrixInterface::inputAddressMinMax(uint32_t channel) const
{
  assert(
    channel == InputChannel::sx0 ||
    channel == InputChannel::sx1);

  if(Selectrix::toBus(channel) == Selectrix::Bus::SX0)
  {
    return {inputAddressMin, inputAddressMaxSX0};
  }
  return {inputAddressMin, inputAddressMax};
}

bool SelectrixInterface::isInputAddressAvailable(uint32_t channel, uint32_t address) const
{
  if(!InputController::isInputAddressAvailable(channel, address))
  {
    return false;
  }

  if(auto it = m_usedBusAddresses.find({Selectrix::toBus(channel), Selectrix::toBusAddress(address)}); it != m_usedBusAddresses.end())
  {
    return it->second.type == Selectrix::AddressType::Feedback;
  }

  return true;
}

bool SelectrixInterface::changeInputChannelAddress(Input& input, uint32_t newChannel, uint32_t newAddress)
{
  if(InputController::changeInputChannelAddress(input, newChannel, newAddress))
  {
    useFeedbackAddress(newChannel, newAddress);
    unuseFeedbackAddress(input.channel, input.address);
    return true;
  }
  return false;
}

void SelectrixInterface::inputSimulateChange(uint32_t channel, uint32_t address, SimulateInputAction action)
{
  if(m_kernel && inRange(address, inputAddressMinMax(channel)))
  {
    m_kernel->simulateInputChange(Selectrix::toBus(channel), address, action);
  }
}

void SelectrixInterface::onlineChanged(bool /*value*/)
{
  updateEnabled();
}

bool SelectrixInterface::setOnline(bool& value, bool simulation)
{
  if(!m_kernel && value)
  {
    try
    {
      if(simulation)
      {
        m_kernel = Selectrix::Kernel::create<Selectrix::SimulationIOHandler>(id.value(), selectrix->config());
      }
      else
      {
        m_kernel = Selectrix::Kernel::create<Selectrix::SerialIOHandler>(id.value(), selectrix->config(), device.value(), baudrate.value(), selectrix->useRautenhausCommandFormat.value());
      }

      setState(InterfaceState::Initializing);

      m_kernel->setOnStarted(
        [this]()
        {
          setState(InterfaceState::Online);
        });
      m_kernel->setOnError(
        [this]()
        {
          setState(InterfaceState::Error);
          online = false; // communication no longer possible
        });
      m_kernel->setOnTrackPowerChanged(
        [this](bool powerOn)
        {
          if(powerOn && !contains(m_world.state.value(), WorldState::PowerOn))
            m_world.powerOn();
          else if(!powerOn && contains(m_world.state.value(), WorldState::PowerOn))
            m_world.powerOff();
        });
      m_kernel->setDecoderController(this);
      m_kernel->setInputController(this);

      m_kernel->start();

      for(const auto& it : m_usedBusAddresses)
      {
        m_kernel->addAddress(it.first.bus, it.first.address, it.second.type);
      }

      m_selectrixPropertyChanged = selectrix->propertyChanged.connect(
        [this](BaseProperty& /*property*/)
        {
          m_kernel->setConfig(selectrix->config());
        });

      m_kernel->setTrackPower(contains(m_world.state.value(), WorldState::PowerOn));
      Attributes::setEnabled({device, baudrate}, false);
    }
    catch(const LogMessageException& e)
    {
      setState(InterfaceState::Offline);
      Log::log(*this, e.message(), e.args());
      return false;
    }
  }
  else if(m_kernel && !value)
  {
    Attributes::setEnabled({device, baudrate}, true);

    m_selectrixPropertyChanged.disconnect();

    m_kernel->stop();
    m_kernel.reset();

    if(status->state != InterfaceState::Error)
      setState(InterfaceState::Offline);
  }
  return true;
}

void SelectrixInterface::decoderAdded(Decoder& decoder)
{
  useLocomotiveAddress(decoder.address);
}

void SelectrixInterface::decoderRemoved(Decoder& decoder)
{
  unuseLocomotiveAddress(decoder.address);
}

void SelectrixInterface::inputAdded(Input& input)
{
  useFeedbackAddress(input.channel, input.address);
}

void SelectrixInterface::inputRemoved(Input& input)
{
  unuseFeedbackAddress(input.channel, input.address);
}

void SelectrixInterface::addToWorld()
{
  Interface::addToWorld();
  DecoderController::addToWorld();
  InputController::addToWorld(inputListColumns);
}

void SelectrixInterface::loaded()
{
  Interface::loaded();
  updateEnabled();
}

void SelectrixInterface::destroying()
{
  InputController::destroying();
  DecoderController::destroying();
  Interface::destroying();
}

void SelectrixInterface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);

  switch(event)
  {
    case WorldEvent::PowerOff:
      if(m_kernel)
      {
        m_kernel->setTrackPower(false);
      }
      break;

    case WorldEvent::PowerOn:
      if(m_kernel)
      {
        m_kernel->setTrackPower(true);
      }
      break;

    case WorldEvent::EditDisabled:
    case WorldEvent::EditEnabled:
      updateEnabled();
      break;

    default:
      break;
  }
}

void SelectrixInterface::useLocomotiveAddress(uint32_t address)
{
  const BusAddress key{Selectrix::Bus::SX0, static_cast<uint8_t>(address)};

  assert(m_usedBusAddresses.find(key) == m_usedBusAddresses.end());
  m_usedBusAddresses.insert({key, {Selectrix::AddressType::Locomotive, 0}});
  if(m_kernel)
  {
    m_kernel->addAddress(key.bus, key.address, Selectrix::AddressType::Locomotive);
  }
}

void SelectrixInterface::unuseLocomotiveAddress(uint32_t address)
{
  const BusAddress key{Selectrix::Bus::SX0, static_cast<uint8_t>(address)};

  if(auto it = m_usedBusAddresses.find(key); it != m_usedBusAddresses.end()) /*[[likely]]*/
  {
    assert(it->second.type == Selectrix::AddressType::Locomotive);
    m_usedBusAddresses.erase(it);
    if(m_kernel)
    {
      m_kernel->removeAddress(key.bus, key.address);
    }
  }
}

void SelectrixInterface::useFeedbackAddress(uint32_t channel, uint32_t address)
{
  const BusAddress key{Selectrix::toBus(channel), Selectrix::toBusAddress(address)};
  const uint8_t portBit = 1 << Selectrix::toPort(address);

  if(auto it = m_usedBusAddresses.find(key); it != m_usedBusAddresses.end())
  {
    assert(it->second.type == Selectrix::AddressType::Feedback);
    it->second.mask |= portBit;
  }
  else
  {
    m_usedBusAddresses.insert({key, {Selectrix::AddressType::Feedback, portBit}});
    if(m_kernel)
    {
      m_kernel->addAddress(key.bus, key.address, Selectrix::AddressType::Feedback);
    }
  }
}

void SelectrixInterface::unuseFeedbackAddress(uint32_t channel, uint32_t address)
{
  const BusAddress key{Selectrix::toBus(channel), Selectrix::toBusAddress(address)};
  const uint8_t portBit = 1 << Selectrix::toPort(address);

  if(auto it = m_usedBusAddresses.find(key); it != m_usedBusAddresses.end()) /*[[likely]]*/
  {
    assert(it->second.type == Selectrix::AddressType::Feedback);
    it->second.mask &= ~portBit;
    if(it->second.mask == 0x00)
    {
      m_usedBusAddresses.erase(it);
      if(m_kernel)
      {
        m_kernel->removeAddress(key.bus, key.address);
      }
    }
  }
}

void SelectrixInterface::updateEnabled()
{
  selectrix->updateEnabled(contains(m_world.state, WorldState::Edit), online);
}
