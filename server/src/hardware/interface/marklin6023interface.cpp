/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Kamil Kasprzak
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

#include "marklin6023interface.hpp"
#include "../protocol/marklin6023interface/iohandler/serialiohandler.hpp"
#include "../protocol/marklin6023interface/iohandler/simulationiohandler.hpp"
#include "../output/list/outputlist.hpp"
#include "../input/list/inputlist.hpp"
#include "../decoder/list/decoderlist.hpp"
#include "../decoder/list/decoderlisttablemodel.hpp"
#include "../decoder/decoderchangeflags.hpp"
#include "../decoder/decoder.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../utils/makearray.hpp"
#include "../../world/world.hpp"
#include "../../core/attributes.hpp"
#include "../../core/objectproperty.tpp"
#include "../../core/eventloop.hpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"

// List columns
constexpr auto inputListColumns  = InputListColumn::Address;
constexpr auto outputListColumns = OutputListColumn::Channel | OutputListColumn::Address;
constexpr auto decoderListColumns =
  DecoderListColumn::Id | DecoderListColumn::Name | DecoderListColumn::Address;

// Baudrate choices as a span-compatible array
static constexpr std::array<uint32_t, 6> kBaudrateValues{
  1200, 2400, 4800, 9600, 19200, 38400
};

CREATE_IMPL(Marklin6023Interface)

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

Marklin6023Interface::Marklin6023Interface(World& world, std::string_view objId)
  : Interface{world, objId}
  , DecoderController{*this, decoderListColumns}
  , InputController{static_cast<IdObject&>(*this)}
  , OutputController{static_cast<IdObject&>(*this)}
  , serialPort{this, "serialPort", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , baudrate{this, "baudrate", 9600, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , settings{this, "settings", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject}
{
  name = "Märklin 6023/6223";

  settings.setValueInternal(
    std::make_shared<Marklin6023::Settings>(*this, settings.name()));

  Attributes::addDisplayName(serialPort, DisplayName::Serial::device);
  Attributes::addEnabled(serialPort, !online);
  m_interfaceItems.insertBefore(serialPort, notes);

  Attributes::addDisplayName(baudrate, DisplayName::Serial::baudrate);
  Attributes::addEnabled(baudrate, !online);
  Attributes::addValues(baudrate, std::span<const uint32_t>{kBaudrateValues});
  m_interfaceItems.insertBefore(baudrate, notes);

  m_interfaceItems.insertBefore(settings, notes);
  m_interfaceItems.insertBefore(inputs,   notes);
  m_interfaceItems.insertBefore(outputs,  notes);
  m_interfaceItems.insertBefore(decoders, notes);
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void Marklin6023Interface::addToWorld()
{
  Interface::addToWorld();
  DecoderController::addToWorld();
  InputController::addToWorld(inputListColumns);
  OutputController::addToWorld(outputListColumns);
}

void Marklin6023Interface::loaded()
{
  Interface::loaded();
  updateEnabled();
}

void Marklin6023Interface::destroying()
{
  OutputController::destroying();
  InputController::destroying();
  DecoderController::destroying();
  Interface::destroying();
}

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

void Marklin6023Interface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);
  updateEnabled();

  if(!m_kernel)
    return;

  switch(event)
  {
    case WorldEvent::Stop: m_kernel->sendGlobalStop(); break;
    case WorldEvent::Run:  m_kernel->sendGlobalGo();  break;
    default: break;
  }
}

void Marklin6023Interface::onlineChanged(bool /*value*/)
{
  updateEnabled();
}

void Marklin6023Interface::updateEnabled()
{
  Attributes::setEnabled(serialPort, !online);
  Attributes::setEnabled(baudrate,   !online);
  settings->updateEnabled(online);
}

// ---------------------------------------------------------------------------
// Connection
// ---------------------------------------------------------------------------

bool Marklin6023Interface::setOnline(bool& value, bool simulation)
{
  if(value)
  {
    setState(InterfaceState::Initializing);

    const auto cfg = settings->config();

    try
    {
      m_kernel = std::make_unique<Marklin6023::Kernel>(id.value(), cfg);

      m_kernel->s88Callback = [this](uint32_t address, bool state)
      {
        onS88Input(address, state);
      };

      if(simulation)
        m_kernel->setIOHandler(
          std::make_unique<Marklin6023::SimulationIOHandler>(
            *m_kernel, m_kernel->strand()));
      else
        m_kernel->setIOHandler(
          std::make_unique<Marklin6023::SerialIOHandler>(
            *m_kernel, m_kernel->ioContext(), m_kernel->strand(),
            serialPort.value(), baudrate.value()));

      m_kernel->start();
    }
    catch(const LogMessageException& e)
    {
      Log::log(id.value(), e.message(), e.args());
      m_kernel.reset();
      value = false;
      setState(InterfaceState::Offline);
      return false;
    }

    setState(InterfaceState::Online);
  }
  else
  {
    if(m_kernel)
    {
      m_kernel->stop();
      m_kernel.reset();
    }
    setState(InterfaceState::Offline);
  }

  updateEnabled();
  return true;
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------

std::span<const InputChannel> Marklin6023Interface::inputChannels() const
{
  static const auto values = makeArray(InputChannel::S88);
  return values;
}

std::pair<uint32_t, uint32_t>
Marklin6023Interface::inputAddressMinMax(InputChannel channel) const
{
  if(channel == InputChannel::S88)
    return {1, settings->s88amount.value() * 16};
  return {0, 0};
}

void Marklin6023Interface::inputSimulateChange(
  InputChannel channel, uint32_t address, SimulateInputAction action)
{
  if(channel != InputChannel::S88)
    return;

  switch(action)
  {
    case SimulateInputAction::SetFalse: onS88Input(address, false); break;
    case SimulateInputAction::SetTrue:  onS88Input(address, true);  break;
    case SimulateInputAction::Toggle:   onS88Input(address, true);  break;
  }
}

void Marklin6023Interface::onS88Input(uint32_t address, bool state)
{
  updateInputValue(InputChannel::S88, address,
                   state ? TriState::True : TriState::False);
}

// ---------------------------------------------------------------------------
// Output
// ---------------------------------------------------------------------------

std::span<const OutputChannel> Marklin6023Interface::outputChannels() const
{
  static const auto values = makeArray(
    OutputChannel::Accessory,
    OutputChannel::Turnout,
    OutputChannel::Output);
  return values;
}

std::pair<uint32_t, uint32_t>
Marklin6023Interface::outputAddressMinMax(OutputChannel channel) const
{
  switch(channel)
  {
    case OutputChannel::Accessory:
    case OutputChannel::Turnout:
    case OutputChannel::Output:
      return {1, 256};
    default:
      return OutputController::outputAddressMinMax(channel);
  }
}

bool Marklin6023Interface::setOutputValue(
  OutputChannel channel, uint32_t address, OutputValue value)
{
  if(!m_kernel)
    return false;

  switch(channel)
  {
    case OutputChannel::Accessory:
    case OutputChannel::Turnout:
    case OutputChannel::Output:
    {
      const auto [min, max] = outputAddressMinMax(channel);
      if(!inRange(address, min, max)) [[unlikely]]
        return false;

      const bool result = m_kernel->setAccessory(address, value);
      if(result)
        updateOutputValue(channel, address, value);
      return result;
    }
    default:
      return false;
  }
}

// ---------------------------------------------------------------------------
// Decoder
// ---------------------------------------------------------------------------

std::span<const DecoderProtocol> Marklin6023Interface::decoderProtocols() const
{
  static constexpr std::array<DecoderProtocol, 1> p{DecoderProtocol::Motorola};
  return p;
}

std::pair<uint16_t, uint16_t>
Marklin6023Interface::decoderAddressMinMax(DecoderProtocol /*protocol*/) const
{
  return {10, 40};
}

std::span<const uint8_t> Marklin6023Interface::decoderSpeedSteps(DecoderProtocol) const
{
  static constexpr std::array<uint8_t, 1> steps{14};
  return steps;
}

void Marklin6023Interface::decoderChanged(
  const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(!m_kernel)
    return;

  const uint8_t address = static_cast<uint8_t>(decoder.address);
  const bool    f0      = decoder.getFunctionValue(0);
  const uint8_t speed   = decoder.emergencyStop
    ? 0 : Decoder::throttleToSpeedStep<uint8_t>(decoder.throttle, 14);

  // Emergency stop overrides all other changes.
  if(has(changes, DecoderChangeFlags::EmergencyStop) && decoder.emergencyStop)
  {
    m_kernel->setLocoEmergencyStop(address, f0);
    return;
  }

  // Handle remaining flags independently — multiple may be set simultaneously.
  if(has(changes, DecoderChangeFlags::Direction))
    m_kernel->setLocoDirection(address, f0);

  if(has(changes, DecoderChangeFlags::Throttle))
    m_kernel->setLocoSpeed(address, speed, f0);

  // 6023/6223: only F0 is supported
  if(has(changes, DecoderChangeFlags::FunctionValue) && functionNumber == 0)
    m_kernel->setLocoFunction(address, speed, f0);
}
