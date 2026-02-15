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

#include "dinamointerface.hpp"
#include "dinamo/dinamosettings.hpp"
#include "../decoder/decoderchangeflags.hpp"
#include "../decoder/list/decoderlist.hpp"
#include "../decoder/list/decoderlisttablemodel.hpp"
#include "../input/input.hpp"
#include "../input/list/inputlist.hpp"
#include "../input/list/inputlistcolumn.hpp"
#include "../output/list/outputlist.hpp"
#include "../output/list/outputlistcolumn.hpp"
#include "../protocol/dinamo/dinamokernel.hpp"
#include "../protocol/dinamo/dinamopolarity.hpp"
#include "../protocol/dinamo/iohandler/dinamoserialiohandler.hpp"
#include "../protocol/dinamo/iohandler/dinamosimulationiohandler.hpp"
#include "../protocol/dinamo/simulator/dinamosimulator.hpp"
#include "../trackdriver/trackdriver.hpp"
#include "../../core/attributes.hpp"
#include "../../core/eventloop.hpp"
#include "../../core/method.tpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../train/train.hpp"
#include "../../train/trainvehiclelist.hpp"
#include "../../utils/displayname.hpp"
#include "../../vehicle/rail/railvehicle.hpp"
#include "../../world/world.hpp"

namespace {

constexpr auto decoderListColumns = DecoderListColumn::Id | DecoderListColumn::Name | DecoderListColumn::Protocol | DecoderListColumn::Address;
constexpr auto inputListColumns = InputListColumn::Address;
constexpr auto outputListColumns = OutputListColumn::Channel | OutputListColumn::Address;

std::shared_ptr<Decoder> getTrainDecoder(const Train& train)
{
  for(const auto& vehicle : *train.vehicles)
  {
    if(vehicle->decoder)
    {
      return vehicle->decoder.value();
    }
  }
  return {};
}

std::vector<std::shared_ptr<Decoder>> getTrainDecoders(const Train& train)
{
  std::vector<std::shared_ptr<Decoder>> decoders;
  for(const auto& vehicle : *train.vehicles)
  {
    if(vehicle->decoder)
    {
      decoders.emplace_back(vehicle->decoder.value());
    }
  }
  return decoders;
}

uint8_t getDecoderSpeed(const Decoder& decoder)
{
  return Decoder::throttleToSpeedStep<uint8_t>(decoder.throttle, decoder.speedSteps);
}

uint32_t getDecoderFunctions(const Decoder& decoder)
{
  uint32_t functions = 0;
  for(const auto& func : *decoder.functions)
  {
    if(func->value)
    {
      functions |= 1u << func->number.value();
    }
  }
  return functions;
}

}

CREATE_IMPL(DinamoInterface)

DinamoInterface::DinamoInterface(World& world, std::string_view _id)
  : Interface(world, _id)
  , DecoderController(*this, decoderListColumns)
  , InputController(static_cast<IdObject&>(*this))
  , OutputController(static_cast<IdObject&>(*this))
  , TrackDriverController(static_cast<IdObject&>(*this))
  , device{this, "device", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , dinamo{this, "dinamo", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "DINAMO";
  dinamo.setValueInternal(std::make_shared<DinamoSettings>(*this, dinamo.name()));

  Attributes::addEnabled(device, !online);
  m_interfaceItems.insertBefore(device, notes);

  m_interfaceItems.insertBefore(dinamo, notes);

  m_interfaceItems.insertBefore(decoders, notes);

  m_interfaceItems.insertBefore(inputs, notes);

  m_interfaceItems.insertBefore(outputs, notes);

  m_dinamoPropertyChanged = dinamo->propertyChanged.connect(
    [this](BaseProperty& /*property*/)
    {
      if(m_kernel)
      {
        m_kernel->setConfig(dinamo->config());
      }
    });

  updateEnabled();
}

DinamoInterface::~DinamoInterface() = default;

std::span<const DecoderProtocol> DinamoInterface::decoderProtocols() const
{
  static constexpr auto protocols = std::array{DecoderProtocol::DCCShort, DecoderProtocol::DCCLong, DecoderProtocol::Analog};
  return protocols;
}

std::span<const uint8_t> DinamoInterface::decoderSpeedSteps(DecoderProtocol protocol) const
{
  static constexpr std::array<uint8_t, 2> dccSpeedSteps{{28, 128}};
  static constexpr std::array<uint8_t, 1> analogSpeedSteps{{63}};

  switch(protocol)
  {
    case DecoderProtocol::DCCShort:
    case DecoderProtocol::DCCLong:
      return dccSpeedSteps;

    case DecoderProtocol::Analog:
      return analogSpeedSteps;

    default: [[unlikely]]
      assert(false);
      return {};
  }
}

void DinamoInterface::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(!m_kernel)
  {
    return;
  }

  if(const auto& train = decoder.vehicle->activeTrain.value())
  {
    const auto trainId = reinterpret_cast<TrainId>(train.get());

    if(auto it = m_trains.find(trainId); it != m_trains.end())
    {
      auto& trainData = it->second;
      const auto drivingBlock = trainData.blocks.back().address;

      if(trainData.type == TrainData::Type::Analog)
      {
        if(&decoder == getTrainDecoder(*train).get()) // only "listen" to first decoder of the train
        {
          if(has(changes, DecoderChangeFlags::EmergencyStop | DecoderChangeFlags::Direction | DecoderChangeFlags::Throttle | DecoderChangeFlags::SpeedSteps))
          {
            const uint8_t speed = contains(m_world.state, WorldState::Run) ? getDecoderSpeed(decoder) : 0;

            if(has(changes, DecoderChangeFlags::Direction))
            {
              auto polarity = (decoder.direction == Direction::Forward) ? Dinamo::Polarity::Positive : Dinamo::Polarity::Negative;
              m_kernel->setBlockAnalogSpeed(drivingBlock, speed, polarity);
            }
            else
            {
              m_kernel->setBlockAnalogSpeed(drivingBlock, speed);
            }
          }
          if(has(changes, DecoderChangeFlags::FunctionValue) && functionNumber == 0)
          {
            m_kernel->setBlockAnalogLight(drivingBlock, decoder.getFunctionValue(0));
          }
        }
      }
      else // DCC
      {
        if(has(changes, DecoderChangeFlags::EmergencyStop | DecoderChangeFlags::Direction | DecoderChangeFlags::Throttle | DecoderChangeFlags::SpeedSteps))
        {
          const bool run = contains(m_world.state, WorldState::Run);
          m_kernel->setBlockDCCSpeedAndDirection(
            drivingBlock,
            decoder.address,
            decoder.protocol == DecoderProtocol::DCCLong,
            !run || decoder.emergencyStop,
            run ? getDecoderSpeed(decoder) : 0,
            decoder.direction);
        }
        if(has(changes, DecoderChangeFlags::FunctionValue) && functionNumber <= 12)
        {
          if(functionNumber <= 4)
          {
            m_kernel->setBlockDCCFunctionF0F4(
              drivingBlock,
              decoder.address,
              decoder.protocol == DecoderProtocol::DCCLong,
              decoder.getFunctionValue(0),
              decoder.getFunctionValue(1),
              decoder.getFunctionValue(2),
              decoder.getFunctionValue(3),
              decoder.getFunctionValue(4));
          }
          else if(functionNumber <= 8)
          {
            m_kernel->setBlockDCCFunctionF5F8(
              drivingBlock,
              decoder.address,
              decoder.protocol == DecoderProtocol::DCCLong,
              decoder.getFunctionValue(5),
              decoder.getFunctionValue(6),
              decoder.getFunctionValue(7),
              decoder.getFunctionValue(8));
          }
          else if(functionNumber <= 12)
          {
            m_kernel->setBlockDCCFunctionF9F12(
              drivingBlock,
              decoder.address,
              decoder.protocol == DecoderProtocol::DCCLong,
              decoder.getFunctionValue(9),
              decoder.getFunctionValue(10),
              decoder.getFunctionValue(11),
              decoder.getFunctionValue(12));
          }
        }
      }
    }
  }
}

std::span<const InputChannel> DinamoInterface::inputChannels() const
{
  static constexpr auto values = std::array{InputChannel::Input};
  return values;
}

std::pair<uint32_t, uint32_t> DinamoInterface::inputAddressMinMax(InputChannel /*channel*/) const
{
  return {Dinamo::Kernel::inputAddressMin, Dinamo::Kernel::inputAddressMax};
}

void DinamoInterface::inputSimulateChange(InputChannel /*channel*/, uint32_t address, SimulateInputAction action)
{
  if(m_simulator) [[likely]]
  {
    switch(action)
    {
      using enum SimulateInputAction;

      case SetFalse:
        m_simulator->inputEvent(static_cast<uint16_t>(address), false);
        break;

      case SetTrue:
        m_simulator->inputEvent(static_cast<uint16_t>(address), true);
        break;

      case Toggle:
        m_simulator->inputEventToggle(static_cast<uint16_t>(address));
        break;
    }
  }
}

std::span<const OutputChannel> DinamoInterface::outputChannels() const
{
  static constexpr auto values = std::array{OutputChannel::OC32};
  return values;
}

std::pair<uint32_t, uint32_t> DinamoInterface::outputAddressMinMax(OutputChannel channel) const
{
  switch(channel)
  {
    case OutputChannel::OC32:
      return {Dinamo::Kernel::outputOC32AddressMin, Dinamo::Kernel::outputOC32AddressMax};

    default: [[unlikely]]
      break;
  }
  return {0, 0};
}

bool DinamoInterface::setOutputValue(OutputChannel channel, uint32_t address, OutputValue value)
{
  if(m_kernel && channel == OutputChannel::OC32)
  {
    m_kernel->setOC32Aspect(address, static_cast<uint8_t>(std::get<int16_t>(value)));
    return true;
  }
  return false;
}

std::pair<uint32_t, uint32_t> DinamoInterface::trackDriverAddressMinMax() const
{
  return {Dinamo::Kernel::blockAddressMin, Dinamo::Kernel::blockAddressMax};
}

void DinamoInterface::trackDriverTrainAdded(uint32_t address, bool invertPolarity, const Train& train, BlockTrainDirection direction)
{
  const auto trainId = reinterpret_cast<TrainId>(&train);
  const auto block = static_cast<uint8_t>(address);

  if(auto it = m_trains.find(trainId); it != m_trains.end()) // link block
  {
    auto& trainData = it->second;
    assert(!trainData.blocks.empty());

    if(trainData.direction != train.direction)
    {
      trainData.direction = train.direction;

      // train direction changed, which means the train direction in all blocks changes as well:
      for(auto& b : trainData.blocks)
      {
        b.direction = !b.direction;
      }

      // if train is in multiple blocks, order must be reversed so front() matches train direction.
      if(trainData.blocks.size() > 1)
      {
        std::reverse(trainData.blocks.begin(), trainData.blocks.end());
        if(m_kernel)
        {
          m_kernel->unlinkBlockUp(trainData.blocks.back().address);
          linkBlocks(trainData.blocks);
        }
      }
    }

    const auto& srcBlock = trainData.blocks.front();
    const auto srcPolarity = srcBlock.invertedPolarity ? ~srcBlock.polarity : srcBlock.polarity;
    const auto polarity = (invertPolarity != (srcBlock.direction != direction)) ? ~srcPolarity : srcPolarity;
    if(m_kernel)
    {
      m_kernel->linkBlock(block, srcBlock.address, polarity != srcPolarity);
    }
    trainData.blocks.emplace_front(TrainData::Block{
      .address = block,
      .invertedPolarity = invertPolarity,
      .direction = direction,
      .polarity = polarity,
    });

    if(dinamo->debugLogTrainBlocks)
    {
      Log::log(*this, LogMessage::D2011_TRAIN_X_BLOCKS_X, train.name.value(), debugBlocks(trainData.blocks));
    }
  }
  else // new train
  {
    const auto type = getType(train);
    auto polarity = (type == TrainData::Type::Analog && train.direction == Direction::Reverse) ? Dinamo::Polarity::Negative : Dinamo::Polarity::Positive;

    const auto [trainDataIt, _] = m_trains.emplace(trainId, TrainData{
      .type = type,
      .direction = train.direction,
      .blocks = std::list<TrainData::Block>{{
        .address = block,
        .invertedPolarity = invertPolarity,
        .direction = direction,
        .polarity = invertPolarity ? ~polarity : polarity,
      }},
      .train = train.weak_ptr_c<Train>(),
    });

    if(m_kernel)
    {
      // only set block mode/polarity and function(s), train speed is zero
      if(type == TrainData::Type::Analog)
      {
        if(auto dec = getTrainDecoder(train)) [[likely]]
        {
          m_kernel->setBlockAnalog(block, dec->getFunctionValue(0), polarity);
        }
      }
      else if(type == TrainData::Type::DCC)
      {
        if(auto decs = getTrainDecoders(train); !decs.empty()) [[likely]]
        {
          m_kernel->setBlockDCC(block, polarity);

          for(const auto& dec : decs)
          {
            m_kernel->setBlockDCCSpeedAndDirection(
              block,
              dec->address,
              dec->protocol == DecoderProtocol::DCCLong,
              true, // estop
              0,
              dec->direction);

            m_kernel->setBlockDCCFunctions(
              block,
              dec->address,
              dec->protocol == DecoderProtocol::DCCLong,
              getDecoderFunctions(*dec));
          }
        }
      }
      else [[unlikely]]
      {
        assert(false);
      }
    }

    if(dinamo->debugLogTrainBlocks)
    {
      Log::log(*this, LogMessage::D2011_TRAIN_X_BLOCKS_X, train.name.value(), debugBlocks(trainDataIt->second.blocks));
    }
  }
}

void DinamoInterface::trackDriverTrainFlipped(uint32_t address, const Train& train, BlockTrainDirection direction)
{
  const auto trainId = reinterpret_cast<TrainId>(&train);

  if(auto it = m_trains.find(trainId); it != m_trains.end()) [[likely]]
  {
    auto& trainData = it->second;
    if(trainData.blocks.size() == 1 && trainData.blocks.front().address == address) [[likely]]
    {
      trainData.blocks.front().direction = direction;
    }
  }
}

void DinamoInterface::trackDriverTrainRemoved(uint32_t address, const Train& train)
{
  const auto trainId = reinterpret_cast<TrainId>(&train);
  const auto block = static_cast<uint8_t>(address);

  if(auto it = m_trains.find(trainId); it != m_trains.end()) [[likely]]
  {
    auto& trainData = it->second;
    assert(!trainData.blocks.empty());

    if(trainData.blocks.size() == 1 && trainData.blocks.front().address == block) // train removed
    {
      trainData.blocks.clear();
      if(m_kernel)
      {
        m_kernel->resetBlock(block);
      }
    }
    else if(trainData.blocks.back().address == block) // train left block
    {
      trainData.blocks.pop_back();
      if(m_kernel)
      {
        m_kernel->unlinkBlockUp(trainData.blocks.back().address);
      }
    }
    else if(trainData.blocks.front().address == block) // train released block in front of the train
    {
      trainData.blocks.pop_front();
      if(m_kernel)
      {
        m_kernel->unlinkBlockDown(block);
      }
    }
    else if(auto blockItr = std::find_if(trainData.blocks.begin(), trainData.blocks.end(),
      [block](const auto& item)
      {
        return item.address == block;
      }); blockItr != trainData.blocks.end()) // somewhere in between -> rebuild chain, this is uncommon
    {
      trainData.blocks.erase(blockItr);
      if(m_kernel)
      {
        m_kernel->unlinkBlockUp(trainData.blocks.front().address);
        linkBlocks(trainData.blocks);
      }
    }
    else
    {
      assert(false); // unknown block, this should never happen
    }

    if(dinamo->debugLogTrainBlocks)
    {
      Log::log(*this, LogMessage::D2011_TRAIN_X_BLOCKS_X, train.name.value(), debugBlocks(trainData.blocks));
    }

    if(trainData.blocks.empty())
    {
      m_trains.erase(it);
    }
  }
}

void DinamoInterface::addToWorld()
{
  Interface::addToWorld();
  DecoderController::addToWorld();
  InputController::addToWorld(inputListColumns);
  OutputController::addToWorld(outputListColumns);
  TrackDriverController::addToWorld();
}

void DinamoInterface::loaded()
{
  Interface::loaded();

  updateEnabled();
}

void DinamoInterface::destroying()
{
  m_dinamoPropertyChanged.disconnect();
  TrackDriverController::destroying();
  OutputController::destroying();
  InputController::destroying();
  DecoderController::destroying();
  Interface::destroying();
}

void DinamoInterface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);

  switch(event)
  {
    case WorldEvent::EditEnabled:
    case WorldEvent::EditDisabled:
      updateEnabled();
      break;

    case WorldEvent::PowerOff:
      if(m_kernel)
      {
        m_kernel->setFault();
        stopTrains();
      }
      break;

    case WorldEvent::PowerOn:
      if(m_kernel)
      {
        m_kernel->resetFault();
      }
      break;

    case WorldEvent::Stop:
      if(m_kernel)
      {
        stopTrains();
      }
      break;

    case WorldEvent::Run:
      if(m_kernel)
      {
        m_kernel->resetFault();
        runTrains();
      }
      break;

    default:
      break;
  }
}

bool DinamoInterface::setOnline(bool& value, bool simulation)
{
  if(!m_kernel && value)
  {
    try
    {
      if(simulation)
      {
        m_simulator = std::make_unique<Dinamo::Simulator>();
        m_kernel = Dinamo::Kernel::create<Dinamo::SimulationIOHandler>(id.value(), dinamo->config(), std::ref(*m_simulator));
      }
      else
      {
        m_kernel = Dinamo::Kernel::create<Dinamo::SerialIOHandler>(id.value(), dinamo->config(), device.value());
      }

      setState(InterfaceState::Initializing);

      m_kernel->setOnStarted(
        [this]()
        {
          if(contains(m_world.state, WorldState::PowerOn))
          {
            m_kernel->resetFault();
          }
          if(!inputs->empty())
          {
            std::vector<uint16_t> inputAddresses;
            for(const auto& input : *inputs)
            {
              inputAddresses.emplace_back(static_cast<uint16_t>(input->address.value()));
            }
            m_kernel->requestInputState(std::move(inputAddresses));
          }
          setState(InterfaceState::Online);
          if(contains(m_world.state, WorldState::Run))
          {
            runTrains();
          }
        });
      m_kernel->setOnError(
        [this]()
        {
          setState(InterfaceState::Error);
          online = false; // communication no longer possible
        });
      m_kernel->onFault =
        [this]()
        {
          Log::log(*this, LogMessage::E2031_DINAMO_IN_FAULT_STATE);
          m_world.powerOff();
        };
      m_kernel->onInputChanged =
        [this](uint16_t address, bool inputValue)
        {
          updateInputValue(InputChannel::Input, address, toTriState(inputValue));
        };

      m_kernel->start();
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
    m_kernel->stop();
    EventLoop::deleteLater(m_kernel.release());
    EventLoop::deleteLater(m_simulator.release());

    if(status->state != InterfaceState::Error)
    {
      setState(InterfaceState::Offline);
    }
  }
  return true;
}

void DinamoInterface::onlineChanged(bool /*value*/)
{
  updateEnabled();
}

void DinamoInterface::updateEnabled()
{
  const bool editable = contains(m_world.state, WorldState::Edit);

  Attributes::setEnabled(device, editable && !online);
}

void DinamoInterface::linkBlocks(const std::list<TrainData::Block>& blocks)
{
  assert(m_kernel);
  auto src = blocks.begin();
  auto dst = std::next(src);
  for(; dst != blocks.end(); src++, dst++)
  {
    m_kernel->linkBlock(dst->address, src->address, dst->polarity != src->polarity); // rebuild chain
  }
}

void DinamoInterface::stopTrains()
{
  assert(m_kernel);
  for(const auto& [trainid, trainData] : m_trains)
  {
    const auto drivingBlock = trainData.blocks.back().address;

    if(trainData.type == TrainData::Type::Analog)
    {
      m_kernel->setBlockAnalogSpeed(drivingBlock, 0);
    }
    else if(trainData.type == TrainData::Type::DCC)
    {
      if(auto train = trainData.train.lock()) [[likely]]
      {
        for(const auto& decoder : getTrainDecoders(*train))
        {
          m_kernel->setBlockDCCSpeedAndDirection(
            drivingBlock,
            decoder->address,
            decoder->protocol.value() == DecoderProtocol::DCCLong,
            true,
            0,
            decoder->direction);
        }
      }
    }
    else [[unlikely]]
    {
      assert(false);
    }
  }
}

void DinamoInterface::runTrains()
{
  assert(m_kernel);
  for(const auto& [trainid, trainData] : m_trains)
  {
    const auto drivingBlock = trainData.blocks.back().address;

    if(auto train = trainData.train.lock()) [[likely]]
    {
      if(trainData.type == TrainData::Type::Analog)
      {
        if(const auto decoder = getTrainDecoder(*train)) [[likely]]
        {
          m_kernel->setBlockAnalogSpeed(
            drivingBlock,
            getDecoderSpeed(*decoder));
        }
      }
      else if(trainData.type == TrainData::Type::DCC)
      {
        for(const auto& decoder : getTrainDecoders(*train))
        {
          assert(decoder);
          m_kernel->setBlockDCCSpeedAndDirection(
            drivingBlock,
            decoder->address,
            decoder->protocol.value() == DecoderProtocol::DCCLong,
            decoder->emergencyStop,
            getDecoderSpeed(*decoder),
            decoder->direction);
        }
      }
      else [[unlikely]]
      {
        assert(false);
      }
    }
  }
}

DinamoInterface::TrainData::Type DinamoInterface::getType(const Train& train)
{
  for(const auto& vehicle : *train.vehicles)
  {
    if(const auto& decoder = vehicle->decoder.value())
    {
      return (decoder->protocol == DecoderProtocol::Analog) ? TrainData::Type::Analog : TrainData::Type::DCC;
    }
  }
  return TrainData::Type::Invalid;
}

std::string DinamoInterface::debugBlocks(const std::list<TrainData::Block>& blocks)
{
  std::string s;
  for(const auto& block : blocks)
  {
    if(!s.empty())
    {
      s.append(" - ");
    }
    s.append(std::format("{}{}", block.address, block.polarity == Dinamo::Polarity::Positive ? "p" : "n"));
  }
  return s;
}
