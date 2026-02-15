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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_DINAMOINTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_DINAMOINTERFACE_HPP

#include "interface.hpp"
#include "../decoder/decodercontroller.hpp"
#include "../input/inputcontroller.hpp"
#include "../output/outputcontroller.hpp"
#include "../trackdriver/trackdrivercontroller.hpp"
#include "../../core/serialdeviceproperty.hpp"
#include "../../core/objectproperty.hpp"

enum class Direction : uint8_t;
class DinamoSettings;

namespace Dinamo {
class Kernel;
class Simulator;
enum class Polarity : int8_t;
}

/**
 * @brief VPEB DINAMO hardware interface
 */
class DinamoInterface final
  : public Interface
  , public DecoderController
  , public InputController
  , public OutputController
  , public TrackDriverController
{
  CLASS_ID("interface.dinamo")
  DEFAULT_ID("dinamo")
  CREATE_DEF(DinamoInterface)

public:
  SerialDeviceProperty device;
  ObjectProperty<DinamoSettings> dinamo;

  DinamoInterface(World& world, std::string_view _id);
  ~DinamoInterface() final;

  // DecoderController:
  std::span<const DecoderProtocol> decoderProtocols() const final;
  std::span<const uint8_t> decoderSpeedSteps(DecoderProtocol protocol) const final;
  void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) final;

  // InputController:
  std::span<const InputChannel> inputChannels() const final;
  std::pair<uint32_t, uint32_t> inputAddressMinMax(InputChannel channel) const final;
  void inputSimulateChange(InputChannel /*channel*/, uint32_t /*address*/, SimulateInputAction /*action*/) final;

  // OutputController:
  std::span<const OutputChannel> outputChannels() const final;
  std::pair<uint32_t, uint32_t> outputAddressMinMax(OutputChannel channel) const final;
  [[nodiscard]] bool setOutputValue(OutputChannel channel, uint32_t address, OutputValue value) final;

  // TrackDriverController:
  std::pair<uint32_t, uint32_t> trackDriverAddressMinMax() const final;
  void trackDriverTrainAdded(uint32_t address, bool invertPolarity, const Train& train, BlockTrainDirection direction) final;
  void trackDriverTrainFlipped(uint32_t address, const Train& train, BlockTrainDirection direction) final;
  void trackDriverTrainRemoved(uint32_t address, const Train& train) final;

protected:
  void addToWorld() final;
  void loaded() final;
  void destroying() final;
  void worldEvent(WorldState state, WorldEvent event) final;

  bool setOnline(bool& value, bool simulation) final;
  void onlineChanged(bool value) final;

private:
  using TrainId = uintptr_t;
  struct TrainData
  {
    enum class Type
    {
      Invalid = 0,
      Analog,
      DCC,
    };
    struct Block
    {
      uint8_t address;
      bool invertedPolarity;
      BlockTrainDirection direction;
      Dinamo::Polarity polarity;
    };

    Type type;
    Direction direction;
    std::list<Block> blocks;
    std::weak_ptr<const Train> train;
  };

  std::unique_ptr<Dinamo::Kernel> m_kernel;
  std::unique_ptr<Dinamo::Simulator> m_simulator;
  boost::signals2::connection m_dinamoPropertyChanged;
  std::unordered_map<TrainId, TrainData> m_trains;

  //void updateBlock(uint8_t address, const Decoder& decoder)

  void updateEnabled();

  void linkBlocks(const std::list<TrainData::Block>& blocks);

  void stopTrains();
  void runTrains();

  static TrainData::Type getType(const Train& train);
  static std::string debugBlocks(const std::list<TrainData::Block>& blocks);
};

#endif
