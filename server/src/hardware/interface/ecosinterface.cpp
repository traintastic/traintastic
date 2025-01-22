/**
 * server/src/hardware/interface/ecosinterface.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2024 Reinder Feenstra
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

#include "ecosinterface.hpp"
#include "../decoder/list/decoderlist.hpp"
#include "../decoder/list/decoderlisttablemodel.hpp"
#include "../input/list/inputlist.hpp"
#include "../output/list/outputlist.hpp"
#include "../protocol/ecos/kernel.hpp"
#include "../protocol/ecos/settings.hpp"
#include "../protocol/ecos/messages.hpp"
#include "../protocol/ecos/iohandler/tcpiohandler.hpp"
#include "../protocol/ecos/iohandler/simulationiohandler.hpp"
#include "../protocol/ecos/object/switch.hpp"
#include "../../core/attributes.hpp"
#include "../../core/eventloop.hpp"
#include "../../core/method.tpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../utils/makearray.hpp"
#include "../../world/world.hpp"
#include "../../world/worldloader.hpp"
#include "../../world/worldsaver.hpp"

constexpr auto decoderListColumns = DecoderListColumn::Id | DecoderListColumn::Name | DecoderListColumn::Protocol | DecoderListColumn::Address;
constexpr auto inputListColumns = InputListColumn::Id | InputListColumn::Name | InputListColumn::Channel | InputListColumn::Address;
constexpr auto outputListColumns = OutputListColumn::Channel | OutputListColumn::Address;

CREATE_IMPL(ECoSInterface)

ECoSInterface::ECoSInterface(World& world, std::string_view _id)
  : Interface(world, _id)
  , DecoderController(*this, decoderListColumns)
  , InputController(static_cast<IdObject&>(*this))
  , OutputController(static_cast<IdObject&>(*this))
  , hostname{this, "hostname", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , ecos{this, "ecos", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "ECoS";
  ecos.setValueInternal(std::make_shared<ECoS::Settings>(*this, ecos.name()));

  Attributes::addDisplayName(hostname, DisplayName::IP::hostname);
  Attributes::addEnabled(hostname, !online);
  m_interfaceItems.insertBefore(hostname, notes);

  m_interfaceItems.insertBefore(ecos, notes);

  m_interfaceItems.insertBefore(decoders, notes);

  m_interfaceItems.insertBefore(inputs, notes);

  m_interfaceItems.insertBefore(outputs, notes);
}

tcb::span<const DecoderProtocol> ECoSInterface::decoderProtocols() const
{
  static constexpr std::array<DecoderProtocol, 4> protocols{DecoderProtocol::DCCShort, DecoderProtocol::DCCLong, DecoderProtocol::Motorola, DecoderProtocol::Selectrix};
  return tcb::span<const DecoderProtocol>{protocols.data(), protocols.size()};
}

void ECoSInterface::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(m_kernel)
    m_kernel->decoderChanged(decoder, changes, functionNumber);
}

const std::vector<uint32_t> *ECoSInterface::inputChannels() const
{
  return &ECoS::Kernel::inputChannels;
}

const std::vector<std::string_view> *ECoSInterface::inputChannelNames() const
{
  return &ECoS::Kernel::inputChannelNames;
}

std::pair<uint32_t, uint32_t> ECoSInterface::inputAddressMinMax(uint32_t channel) const
{
  using namespace ECoS;

  switch(channel)
  {
    case Kernel::InputChannel::s88:
      return {Kernel::s88AddressMin, Kernel::s88AddressMax};

    case Kernel::InputChannel::ecosDetector:
      return {Kernel::ecosDetectorAddressMin, Kernel::ecosDetectorAddressMax};
  }

  assert(false);
  return {0, 0};
}

void ECoSInterface::inputSimulateChange(uint32_t channel, uint32_t address, SimulateInputAction action)
{
  if(m_kernel && inRange(address, inputAddressMinMax(channel)))
    m_kernel->simulateInputChange(channel, address, action);
}

tcb::span<const OutputChannel> ECoSInterface::outputChannels() const
{
  static const auto values = makeArray(OutputChannel::AccessoryDCC, OutputChannel::AccessoryMotorola, OutputChannel::ECoSObject);
  return values;
}

std::pair<tcb::span<const uint16_t>, tcb::span<const std::string>> ECoSInterface::getOutputECoSObjects(OutputChannel channel) const
{
  if(channel == OutputChannel::ECoSObject) /*[[likely]]*/
  {
    return {m_outputECoSObjectIds, m_outputECoSObjectNames};
  }
  return OutputController::getOutputECoSObjects(channel);
}

bool ECoSInterface::isOutputId(OutputChannel channel, uint32_t outputId) const
{
  if(channel == OutputChannel::ECoSObject)
  {
    return inRange<uint32_t>(outputId, ECoS::ObjectId::switchMin, ECoS::ObjectId::switchMax);
  }
  return OutputController::isOutputId(channel, outputId);
}

bool ECoSInterface::setOutputValue(OutputChannel channel, uint32_t outputId, OutputValue value)
{
  return
    m_kernel &&
    isOutputId(channel, outputId) &&
    m_kernel->setOutput(channel, outputId, value);
}

bool ECoSInterface::setOnline(bool& value, bool simulation)
{
  if(!m_kernel && value)
  {
    try
    {
      if(simulation)
        m_kernel = ECoS::Kernel::create<ECoS::SimulationIOHandler>(id.value(), ecos->config(), m_simulation);
      else
        m_kernel = ECoS::Kernel::create<ECoS::TCPIOHandler>(id.value(), ecos->config(), hostname.value());

      setState(InterfaceState::Initializing);

      m_kernel->setOnStarted(
        [this]()
        {
          setState(InterfaceState::Online);

          if(contains(m_world.state.value(), WorldState::Run))
          {
            m_kernel->go();
          }
          else
          {
            m_kernel->emergencyStop();
          }
        });
      m_kernel->setOnError(
        [this]()
        {
          setState(InterfaceState::Error);
          online = false; // communication no longer possible
        });
      m_kernel->setOnEmergencyStop(
        [this]()
        {
          if(contains(m_world.state.value(), WorldState::PowerOn | WorldState::Run))
            m_world.powerOff();
        });
      m_kernel->setOnGo(
        [this]()
        {
          if(!contains(m_world.state.value(), WorldState::Run))
            m_world.run();
        });
      m_kernel->setOnObjectChanged(
        [this](std::size_t typeHash, uint16_t objectId, const std::string& objectName)
        {
          if(typeHash == typeid(ECoS::Switch).hash_code())
          {
            if(auto it = std::find(m_outputECoSObjectIds.begin(), m_outputECoSObjectIds.end(), objectId); it != m_outputECoSObjectIds.end())
            {
              const std::size_t index = std::distance(m_outputECoSObjectIds.begin(), it);
              m_outputECoSObjectNames[index] = objectName;
            }
            else
            {
              m_outputECoSObjectIds.emplace_back(objectId);
              m_outputECoSObjectNames.emplace_back(objectName);
            }
            assert(m_outputECoSObjectIds.size() == m_outputECoSObjectNames.size());
            outputECoSObjectsChanged();
          }
        });
      m_kernel->setOnObjectRemoved(
        [this](uint16_t objectId)
        {
          assert(objectId == 0);
          if(auto it = std::find(m_outputECoSObjectIds.begin(), m_outputECoSObjectIds.end(), objectId); it != m_outputECoSObjectIds.end())
          {
            const std::size_t index = std::distance(m_outputECoSObjectIds.begin(), it);
            m_outputECoSObjectIds.erase(it);
            m_outputECoSObjectNames.erase(std::next(m_outputECoSObjectNames.begin(), index));
            assert(m_outputECoSObjectIds.size() == m_outputECoSObjectNames.size());
            outputECoSObjectsChanged();
          }
        });
      m_kernel->setDecoderController(this);
      m_kernel->setInputController(this);
      m_kernel->setOutputController(this);
      m_kernel->start();

      m_ecosPropertyChanged = ecos->propertyChanged.connect(
        [this](BaseProperty& /*property*/)
        {
          m_kernel->setConfig(ecos->config());
        });

      // Reset output object list:
      m_outputECoSObjectIds.assign({0});
      m_outputECoSObjectNames.assign({{}});

      Attributes::setEnabled(hostname, false);
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
    Attributes::setEnabled(hostname, true);

    m_ecosPropertyChanged.disconnect();

    m_kernel->stop(simulation ? nullptr : &m_simulation);
    EventLoop::deleteLater(m_kernel.release());

    setState(InterfaceState::Offline);
  }
  return true;
}

void ECoSInterface::addToWorld()
{
  Interface::addToWorld();
  DecoderController::addToWorld();
  InputController::addToWorld(inputListColumns);
  OutputController::addToWorld(outputListColumns);
}

void ECoSInterface::destroying()
{
  OutputController::destroying();
  InputController::destroying();
  DecoderController::destroying();
  Interface::destroying();
}

void ECoSInterface::load(WorldLoader& loader, const nlohmann::json& data)
{
  Interface::load(loader, data);

  // load simulation data:
  {
    using namespace nlohmann;

    json simulation;
    if(loader.readFile(simulationDataFilename(), simulation))
    {
      using namespace ECoS;

      // ECoS:
      if(json object = simulation.value("ecos", json::object()); !object.empty())
      {
        m_simulation.ecos.commandStationType = object.value("command_station_type", "");
        m_simulation.ecos.applicationVersion = object.value("application_version", "");
        m_simulation.ecos.applicationVersionSuffix = object.value("application_version_suffix", "");
        m_simulation.ecos.hardwareVersion = object.value("hardware_version", "");
        m_simulation.ecos.protocolVersion = object.value("protocol_version", "");
        m_simulation.ecos.railcom = object.value("railcom", false);
        m_simulation.ecos.railcomPlus = object.value("railcom_plus", false);
      }

      if(json locomotives = simulation.value("locomotives", json::array()); !locomotives.empty())
      {
        for(const json& object : locomotives)
        {
          const uint16_t objectId = object.value("id", 0U);
          LocomotiveProtocol protocol;
          const uint16_t address = object.value("address", 0U);
          if(objectId != 0 && fromString(object.value("protocol", ""), protocol) && address != 0)
            m_simulation.locomotives.emplace_back(Simulation::Locomotive{{objectId}, protocol, address});
        }
      }

      if(json switches = simulation.value("switches", json::array()); !switches.empty())
      {
        for(const json& object : switches)
        {
          const uint16_t objectId = object.value("id", 0U);
          const uint16_t address = object.value("address", 0U);
          if(objectId != 0 && address != 0)
          {
            m_simulation.switches.emplace_back(
              Simulation::Switch{
                {objectId},
                object.value("name1", ""),
                object.value("name2", ""),
                object.value("name3", ""),
                address,
                object.value("addrext", ""),
                object.value("type", ""),
                object.value("symbol", -1),
                object.value("protocol", ""),
                object.value<uint8_t>("state", 0U),
                object.value("mode", ""),
                object.value<uint16_t>("duration", 0U),
                object.value<uint8_t>("variant", 0U)
                });
          }
        }
      }

      if(json s88 = simulation.value("s88", json::array()); !s88.empty())
      {
        for(const json& object : s88)
        {
          const uint16_t objectId = object.value("id", 0U);
          const uint8_t ports = object.value("ports", 0U);
          if(objectId != 0 && (ports == 8 || ports == 16))
            m_simulation.s88.emplace_back(Simulation::S88{{objectId}, ports});
          else
            break;
        }
      }
    }
  }
}

void ECoSInterface::save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const
{
  Interface::save(saver, data, state);

  using nlohmann::json;

  // save data for simulation:
  json simulation = json::object();

  // ECoS:
  {
    simulation["ecos"] = {
      {"command_station_type", m_simulation.ecos.commandStationType},
      {"application_version", m_simulation.ecos.applicationVersion},
      {"application_version_suffix", m_simulation.ecos.applicationVersionSuffix},
      {"hardware_version", m_simulation.ecos.hardwareVersion},
      {"protocol_version", m_simulation.ecos.protocolVersion},
      {"railcom", m_simulation.ecos.railcom},
      {"railcom_plus", m_simulation.ecos.railcomPlus},
      };
  }

  if(!m_simulation.locomotives.empty())
  {
    json objects = json::array();
    for(const auto& locomotive : m_simulation.locomotives)
      objects.emplace_back(json::object({{"id", locomotive.id}, {"protocol", toString(locomotive.protocol)}, {"address", locomotive.address}}));
    simulation["locomotives"] = objects;
  }

  if(!m_simulation.switches.empty())
  {
    json objects = json::array();
    for(const auto& sw : m_simulation.switches)
    {
      objects.emplace_back(
        json::object({
          {"id", sw.id},
          {"name1", sw.name1},
          {"name2", sw.name2},
          {"name3", sw.name3},
          {"address", sw.address},
          {"addrext", sw.addrext},
          {"type", sw.type},
          {"symbol", sw.symbol},
          {"protocol", sw.protocol},
          {"state", sw.state},
          {"mode", sw.mode},
          {"duration", sw.duration},
          {"variant", sw.variant}
          }));
    }
    simulation["switches"] = objects;
  }

  if(!m_simulation.s88.empty())
  {
    json objects = json::array();
    for(const auto& s88 : m_simulation.s88)
      objects.emplace_back(json::object({{"id", s88.id}, {"ports", s88.ports}}));
    simulation["s88"] = objects;
  }

  if(!simulation.empty())
  {
    saver.writeFile(simulationDataFilename(), simulation.dump(2));
  }
}

void ECoSInterface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);

  if(m_kernel)
  {
    switch(event)
    {
      case WorldEvent::PowerOff:
      case WorldEvent::Stop:
        m_kernel->emergencyStop();
        break;

      case WorldEvent::PowerOn:
      case WorldEvent::Run:
        if(contains(state, WorldState::PowerOn | WorldState::Run))
          m_kernel->go();
        break;

      default:
        break;
    }
  }
}

std::filesystem::path ECoSInterface::simulationDataFilename() const
{
  return (std::filesystem::path("simulation") / id.value()) += ".json";
}
