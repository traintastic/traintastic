/**
 * server/src/hardware/interface/z21interface.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#include "z21interface.hpp"
#include "../decoder/decoderlisttablemodel.hpp"
#include "../protocol/z21/messages.hpp"
#include "../protocol/z21/iohandler/udpclientiohandler.hpp"
#include "../../core/attributes.hpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/category.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../world/world.hpp"

Z21Interface::Z21Interface(const std::weak_ptr<World>& world, std::string_view _id)
  : Interface(world, _id)
  , hostname{this, "hostname", "192.168.1.203", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , port{this, "port", 21105, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , z21{this, "z21", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
  , decoders{this, "decoders", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
  , hardwareType{this, "hardware_type", "", PropertyFlags::ReadOnly | PropertyFlags::NoStore}
  , serialNumber{this, "serial_number", "", PropertyFlags::ReadOnly | PropertyFlags::NoStore}
  , firmwareVersion{this, "firmware_version", "", PropertyFlags::ReadOnly | PropertyFlags::NoStore}
{
  z21.setValueInternal(std::make_shared<Z21::ClientSettings>(*this, z21.name()));
  decoders.setValueInternal(std::make_shared<DecoderList>(*this, decoders.name()));

  Attributes::addDisplayName(hostname, DisplayName::IP::hostname);
  Attributes::addEnabled(hostname, !online);
  m_interfaceItems.insertBefore(hostname, notes);

  Attributes::addDisplayName(port, DisplayName::IP::port);
  Attributes::addEnabled(port, !online);
  m_interfaceItems.insertBefore(port, notes);

  Attributes::addDisplayName(z21, DisplayName::Hardware::z21);
  m_interfaceItems.insertBefore(z21, notes);

  Attributes::addDisplayName(decoders, DisplayName::Hardware::decoders);
  m_interfaceItems.insertBefore(decoders, notes);

  Attributes::addCategory(hardwareType, Category::info);
  m_interfaceItems.insertBefore(hardwareType, notes);

  Attributes::addCategory(serialNumber, Category::info);
  m_interfaceItems.insertBefore(serialNumber, notes);

  Attributes::addCategory(firmwareVersion, Category::info);
  m_interfaceItems.insertBefore(firmwareVersion, notes);
}

bool Z21Interface::addDecoder(Decoder& decoder)
{
  const bool success = DecoderController::addDecoder(decoder);
  if(success)
    decoders->addObject(decoder.shared_ptr<Decoder>());
  return success;
}

bool Z21Interface::removeDecoder(Decoder& decoder)
{
  const bool success = DecoderController::removeDecoder(decoder);
  if(success)
    decoders->removeObject(decoder.shared_ptr<Decoder>());
  return success;
}

void Z21Interface::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(m_kernel)
    m_kernel->decoderChanged(decoder, changes, functionNumber);
}

bool Z21Interface::setOnline(bool& value)
{
  if(!m_kernel && value)
  {
    try
    {
      m_kernel = Z21::ClientKernel::create<Z21::UDPClientIOHandler>(z21->config(), hostname.value(), port.value());

      status.setValueInternal(InterfaceStatus::Initializing);

      m_kernel->setLogId(id.value());
      m_kernel->setOnStarted(
        [this]()
        {
          status.setValueInternal(InterfaceStatus::Online);
        });
      m_kernel->setOnSerialNumberChanged(
        [this](uint32_t newValue)
        {
          serialNumber.setValueInternal(std::to_string(newValue));
        });
      m_kernel->setOnHardwareInfoChanged(
        [this](Z21::HardwareType type, uint8_t versionMajor, uint8_t versionMinor)
        {
          hardwareType.setValueInternal(std::string(Z21::toString(type)));
          Log::log(*this, LogMessage::I2002_HARDWARE_TYPE_X, hardwareType.value());

          if(versionMajor != 0 || versionMinor != 0)
          {
            firmwareVersion.setValueInternal(std::to_string(versionMajor).append(".").append(std::to_string(versionMinor)));
            Log::log(*this, LogMessage::I2003_FIRMWARE_VERSION_X, firmwareVersion.value());
          }
          else
            firmwareVersion.setValueInternal("");
        });
      m_kernel->setOnTrackPowerOnChanged(
        [this](bool powerOn)
        {
          if(auto w = m_world.lock())
          {
            if(powerOn == contains(w->state.value(), WorldState::PowerOn))
              return;

            if(powerOn)
              w->powerOn();
            else
              w->powerOff();
          }
        });
      m_kernel->setOnEmergencyStop(
        [this]()
        {
          if(auto w = m_world.lock(); w && contains(w->state.value(), WorldState::Run))
            w->stop();
        });

      m_kernel->setDecoderController(this);

      m_kernel->start();

      m_z21PropertyChanged = z21->propertyChanged.connect(
        [this](BaseProperty& /*property*/)
        {
          m_kernel->setConfig(z21->config());
        });

      if(auto w = m_world.lock())
      {
        if(contains(w->state.value(), WorldState::PowerOn))
          m_kernel->trackPowerOn();
        else
          m_kernel->trackPowerOff();

        if(!contains(w->state.value(), WorldState::Run))
          m_kernel->emergencyStop();
      }

      Attributes::setEnabled({hostname, port}, false);
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
    Attributes::setEnabled({hostname, port}, true);

    m_z21PropertyChanged.disconnect();

    m_kernel->stop();
    m_kernel.reset();

    status.setValueInternal(InterfaceStatus::Offline);
    hardwareType.setValueInternal("");
    serialNumber.setValueInternal("");
    firmwareVersion.setValueInternal("");
  }
  return true;
}

void Z21Interface::addToWorld()
{
  Interface::addToWorld();

  if(auto world = m_world.lock())
  {
    world->decoderControllers->add(std::dynamic_pointer_cast<DecoderController>(shared_from_this()));
  }
}

void Z21Interface::destroying()
{
  for(const auto& decoder : *decoders)
  {
    assert(decoder->interface.value() == std::dynamic_pointer_cast<DecoderController>(shared_from_this()));
    decoder->interface = nullptr;
  }

  if(auto world = m_world.lock())
  {
    world->decoderControllers->remove(std::dynamic_pointer_cast<DecoderController>(shared_from_this()));
  }

  Interface::destroying();
}

void Z21Interface::worldEvent(WorldState state, WorldEvent event)
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
        m_kernel->trackPowerOn();
        if(!contains(state, WorldState::Run))
          m_kernel->emergencyStop();
        break;

      case WorldEvent::Stop:
        m_kernel->emergencyStop();
        break;

      case WorldEvent::Run:
        if(contains(state, WorldState::PowerOn))
          m_kernel->trackPowerOn();
        break;

      default:
        break;
    }
  }
}

void Z21Interface::idChanged(const std::string& newId)
{
  if(m_kernel)
    m_kernel->setLogId(newId);
}
