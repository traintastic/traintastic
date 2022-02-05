/**
 * server/src/hardware/interface/xpressnetinterface.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

#include "xpressnetinterface.hpp"
#include "../input/list/inputlisttablemodel.hpp"
#include "../output/list/outputlisttablemodel.hpp"
#include "../protocol/xpressnet/messages.hpp"
#include "../protocol/xpressnet/iohandler/serialiohandler.hpp"
#include "../protocol/xpressnet/iohandler/liusbiohandler.hpp"
#include "../protocol/xpressnet/iohandler/rosofts88xpressnetliiohandler.hpp"
#include "../protocol/xpressnet/iohandler/tcpiohandler.hpp"
#include "../../core/attributes.hpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../world/world.hpp"

constexpr auto outputListColumns = OutputListColumn::Id | OutputListColumn::Name | OutputListColumn::Address;

XpressNetInterface::XpressNetInterface(World& world, std::string_view _id)
  : Interface(world, _id)
  , type{this, "type", XpressNetInterfaceType::Serial, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](XpressNetInterfaceType /*value*/)
      {
        updateVisible();
      }}
  , serialInterfaceType{this, "interface", XpressNetSerialInterfaceType::LenzLI100, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](XpressNetSerialInterfaceType value)
      {
        switch(value)
        {
          case XpressNetSerialInterfaceType::LenzLI100:
          case XpressNetSerialInterfaceType::RoSoftS88XPressNetLI:
            baudrate.setValueInternal(9600);
            flowControl.setValueInternal(SerialFlowControl::Hardware);
            break;

          case XpressNetSerialInterfaceType::LenzLI100F:
          case XpressNetSerialInterfaceType::LenzLI101F:
            baudrate.setValueInternal(19200);
            flowControl.setValueInternal(SerialFlowControl::Hardware);
            break;

          case XpressNetSerialInterfaceType::LenzLIUSB:
            baudrate.setValueInternal(57600);
            flowControl.setValueInternal(SerialFlowControl::None);
            break;

          case XpressNetSerialInterfaceType::DigikeijsDR5000:
            baudrate.setValueInternal(115200);
            flowControl.setValueInternal(SerialFlowControl::None);
            break;
        }
        updateVisible();
      }}
  , device{this, "device", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , baudrate{this, "baudrate", 19200, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , flowControl{this, "flow_control", SerialFlowControl::None, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , hostname{this, "hostname", "192.168.1.203", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , port{this, "port", 5550, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , s88StartAddress{this, "s88_start_address", XpressNet::RoSoftS88XpressNetLI::S88StartAddress::startAddressDefault, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , s88ModuleCount{this, "s88_module_count", XpressNet::RoSoftS88XpressNetLI::S88ModuleCount::moduleCountDefault, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , xpressnet{this, "xpressnet", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
  , decoders{this, "decoders", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
  , inputs{this, "inputs", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
  , outputs{this, "outputs", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
{
  xpressnet.setValueInternal(std::make_shared<XpressNet::Settings>(*this, xpressnet.name()));
  decoders.setValueInternal(std::make_shared<DecoderList>(*this, decoders.name()));
  inputs.setValueInternal(std::make_shared<InputList>(*this, inputs.name()));
  outputs.setValueInternal(std::make_shared<OutputList>(*this, outputs.name(), outputListColumns));

  Attributes::addDisplayName(type, DisplayName::Interface::type);
  Attributes::addEnabled(type, !online);
  Attributes::addValues(type, xpressNetInterfaceTypeValues);
  m_interfaceItems.insertBefore(type, notes);

  Attributes::addValues(serialInterfaceType, XpressNetSerialInterfaceTypeValues);
  Attributes::addEnabled(serialInterfaceType, !online);
  Attributes::addVisible(serialInterfaceType, false);
  m_interfaceItems.insertBefore(serialInterfaceType, notes);

  Attributes::addDisplayName(device, DisplayName::Serial::device);
  Attributes::addEnabled(device, !online);
  Attributes::addVisible(device, false);
  m_interfaceItems.insertBefore(device, notes);

  Attributes::addDisplayName(baudrate, DisplayName::Serial::baudrate);
  Attributes::addEnabled(baudrate, !online);
  Attributes::addVisible(baudrate, false);
  m_interfaceItems.insertBefore(baudrate, notes);

  Attributes::addDisplayName(flowControl, DisplayName::Serial::flowControl);
  Attributes::addEnabled(flowControl, !online);
  Attributes::addValues(flowControl, SerialFlowControlValues);
  Attributes::addVisible(flowControl, false);
  m_interfaceItems.insertBefore(flowControl, notes);

  Attributes::addDisplayName(hostname, DisplayName::IP::hostname);
  Attributes::addEnabled(hostname, !online);
  Attributes::addVisible(hostname, false);
  m_interfaceItems.insertBefore(hostname, notes);

  Attributes::addDisplayName(port, DisplayName::IP::port);
  Attributes::addEnabled(port, !online);
  Attributes::addVisible(port, false);
  m_interfaceItems.insertBefore(port, notes);

  Attributes::addMinMax(s88StartAddress, XpressNet::RoSoftS88XpressNetLI::S88StartAddress::startAddressMin, XpressNet::RoSoftS88XpressNetLI::S88StartAddress::startAddressMax);
  Attributes::addEnabled(s88StartAddress, !online);
  Attributes::addVisible(s88StartAddress, false);
  m_interfaceItems.insertBefore(s88StartAddress, notes);

  Attributes::addMinMax(s88ModuleCount, XpressNet::RoSoftS88XpressNetLI::S88ModuleCount::moduleCountMin, XpressNet::RoSoftS88XpressNetLI::S88ModuleCount::moduleCountMax);
  Attributes::addEnabled(s88ModuleCount, !online);
  Attributes::addVisible(s88ModuleCount, false);
  m_interfaceItems.insertBefore(s88ModuleCount, notes);

  Attributes::addDisplayName(xpressnet, DisplayName::Hardware::xpressnet);
  m_interfaceItems.insertBefore(xpressnet, notes);

  Attributes::addDisplayName(decoders, DisplayName::Hardware::decoders);
  m_interfaceItems.insertBefore(decoders, notes);

  Attributes::addDisplayName(inputs, DisplayName::Hardware::inputs);
  m_interfaceItems.insertBefore(inputs, notes);

  Attributes::addDisplayName(outputs, DisplayName::Hardware::outputs);
  m_interfaceItems.insertBefore(outputs, notes);

  updateVisible();
}

bool XpressNetInterface::addDecoder(Decoder& decoder)
{
  const bool success = DecoderController::addDecoder(decoder);
  if(success)
    decoders->addObject(decoder.shared_ptr<Decoder>());
  return success;
}

bool XpressNetInterface::removeDecoder(Decoder& decoder)
{
  const bool success = DecoderController::removeDecoder(decoder);
  if(success)
    decoders->removeObject(decoder.shared_ptr<Decoder>());
  return success;
}

void XpressNetInterface::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(m_kernel)
    m_kernel->decoderChanged(decoder, changes, functionNumber);
}

bool XpressNetInterface::addInput(Input& input)
{
  const bool success = InputController::addInput(input);
  if(success)
    inputs->addObject(input.shared_ptr<Input>());
  return success;
}

bool XpressNetInterface::removeInput(Input& input)
{
  const bool success = InputController::removeInput(input);
  if(success)
    inputs->removeObject(input.shared_ptr<Input>());
  return success;
}

bool XpressNetInterface::addOutput(Output& output)
{
  const bool success = OutputController::addOutput(output);
  if(success)
    outputs->addObject(output.shared_ptr<Output>());
  return success;
}

bool XpressNetInterface::removeOutput(Output& output)
{
  const bool success = OutputController::removeOutput(output);
  if(success)
    outputs->removeObject(output.shared_ptr<Output>());
  return success;
}

bool XpressNetInterface::setOutputValue(uint32_t channel, uint32_t address, bool value)
{
  assert(isOutputChannel(channel));
  return
    m_kernel &&
    inRange(address, outputAddressMinMax(channel)) &&
    m_kernel->setOutput(static_cast<uint16_t>(address), value);
}

bool XpressNetInterface::setOnline(bool& value)
{
  if(!m_kernel && value)
  {
    try
    {
      switch(type)
      {
        case XpressNetInterfaceType::Serial:
          switch(serialInterfaceType)
          {
            case XpressNetSerialInterfaceType::LenzLI100:
            case XpressNetSerialInterfaceType::LenzLI100F:
            case XpressNetSerialInterfaceType::LenzLI101F:
              m_kernel = XpressNet::Kernel::create<XpressNet::SerialIOHandler>(xpressnet->config(), device.value(), baudrate.value(), flowControl.value());
              break;

            case XpressNetSerialInterfaceType::RoSoftS88XPressNetLI:
              m_kernel = XpressNet::Kernel::create<XpressNet::RoSoftS88XPressNetLIIOHandler>(xpressnet->config(), device.value(), baudrate.value(), flowControl.value(), s88StartAddress.value(), s88ModuleCount.value());
              break;

            case XpressNetSerialInterfaceType::LenzLIUSB:
            case XpressNetSerialInterfaceType::DigikeijsDR5000:
              m_kernel = XpressNet::Kernel::create<XpressNet::LIUSBIOHandler>(xpressnet->config(), device.value(), baudrate.value(), flowControl.value());
              break;
          }
          break;

        case XpressNetInterfaceType::Network:
          m_kernel = XpressNet::Kernel::create<XpressNet::TCPIOHandler>(xpressnet->config(), hostname.value(), port.value());
          break;
      }

      if(!m_kernel)
      {
        assert(false);
        return false;
      }

      status.setValueInternal(InterfaceStatus::Initializing);

      m_kernel->setLogId(id.value());
      m_kernel->setOnStarted(
        [this]()
        {
          status.setValueInternal(InterfaceStatus::Online);
        });
      m_kernel->setOnNormalOperationResumed(
        [this]()
        {
          if(!contains(m_world.state.value(), WorldState::PowerOn))
            m_world.powerOn();
          if(!contains(m_world.state.value(), WorldState::Run))
            m_world.run();
        });
      m_kernel->setOnTrackPowerOff(
        [this]()
        {
          if(contains(m_world.state.value(), WorldState::PowerOn))
            m_world.powerOff();
          if(contains(m_world.state.value(), WorldState::Run))
            m_world.stop();
        });
      m_kernel->setOnEmergencyStop(
        [this]()
        {
          if(contains(m_world.state.value(), WorldState::Run))
            m_world.stop();
        });

      m_kernel->setDecoderController(this);
      m_kernel->setInputController(this);
      m_kernel->setOutputController(this);
      m_kernel->start();

      m_xpressnetPropertyChanged = xpressnet->propertyChanged.connect(
        [this](BaseProperty& /*property*/)
        {
          m_kernel->setConfig(xpressnet->config());
        });

      if(!contains(m_world.state.value(), WorldState::PowerOn))
        m_kernel->trackPowerOff();
      else if(!contains(m_world.state.value(), WorldState::Run))
        m_kernel->emergencyStop();
      else
        m_kernel->normalOperationsResumed();

      Attributes::setEnabled({type, serialInterfaceType, device, baudrate, flowControl, hostname, port, s88StartAddress, s88ModuleCount}, false);
    }
    catch(const LogMessageException& e)
    {
      status.setValueInternal(InterfaceStatus::Offline);
      Log::log(*this, e.message(), e.args());
      return false;
    }
  }
  else if(m_kernel && !value)
  {
    Attributes::setEnabled({type, serialInterfaceType, device, baudrate, flowControl, hostname, port, s88StartAddress, s88ModuleCount}, true);

    m_xpressnetPropertyChanged.disconnect();

    m_kernel->stop();
    m_kernel.reset();

    status.setValueInternal(InterfaceStatus::Offline);
  }
  return true;
}

void XpressNetInterface::addToWorld()
{
  Interface::addToWorld();

  m_world.decoderControllers->add(std::dynamic_pointer_cast<DecoderController>(shared_from_this()));
  m_world.inputControllers->add(std::dynamic_pointer_cast<InputController>(shared_from_this()));
  m_world.outputControllers->add(std::dynamic_pointer_cast<OutputController>(shared_from_this()));
}

void XpressNetInterface::loaded()
{
  Interface::loaded();

  updateVisible();
}

void XpressNetInterface::destroying()
{
  for(const auto& decoder : *decoders)
  {
    assert(decoder->interface.value() == std::dynamic_pointer_cast<DecoderController>(shared_from_this()));
    decoder->interface = nullptr;
  }

  for(const auto& input : *inputs)
  {
    assert(input->interface.value() == std::dynamic_pointer_cast<InputController>(shared_from_this()));
    input->interface = nullptr;
  }

  for(const auto& output : *outputs)
  {
    assert(output->interface.value() == std::dynamic_pointer_cast<OutputController>(shared_from_this()));
    output->interface = nullptr;
  }

  m_world.decoderControllers->remove(std::dynamic_pointer_cast<DecoderController>(shared_from_this()));
  m_world.inputControllers->remove(std::dynamic_pointer_cast<InputController>(shared_from_this()));
  m_world.outputControllers->remove(std::dynamic_pointer_cast<OutputController>(shared_from_this()));

  Interface::destroying();
}

void XpressNetInterface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);

  if(m_kernel)
  {
    switch(event)
    {
      case WorldEvent::PowerOff:
        m_kernel->trackPowerOff();
        break;

      case WorldEvent::PowerOn:
        m_kernel->normalOperationsResumed();
        if(!contains(state, WorldState::Run))
          m_kernel->emergencyStop();
        break;

      case WorldEvent::Stop:
        m_kernel->emergencyStop();
        break;

      case WorldEvent::Run:
        if(contains(state, WorldState::PowerOn))
          m_kernel->normalOperationsResumed();
        break;

      default:
        break;
    }
  }
}

void XpressNetInterface::idChanged(const std::string& newId)
{
  if(m_kernel)
    m_kernel->setLogId(newId);
}

void XpressNetInterface::updateVisible()
{
  const bool isSerial = (type == XpressNetInterfaceType::Serial);
  Attributes::setVisible(serialInterfaceType, isSerial);
  Attributes::setVisible(device, isSerial);
  Attributes::setVisible(baudrate, isSerial);
  Attributes::setVisible(flowControl, isSerial);

  const bool isNetwork = (type == XpressNetInterfaceType::Network);
  Attributes::setVisible(hostname, isNetwork);
  Attributes::setVisible(port, isNetwork);

  const bool isRoSoftS88XPressNetLI = isSerial && (serialInterfaceType == XpressNetSerialInterfaceType::RoSoftS88XPressNetLI);
  Attributes::setVisible(s88StartAddress, isRoSoftS88XPressNetLI);
  Attributes::setVisible(s88ModuleCount, isRoSoftS88XPressNetLI);
}
