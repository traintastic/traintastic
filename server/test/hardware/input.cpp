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

#include <catch2/catch_template_test_macros.hpp>
#include <bitset>
#include "interfaces.hpp"
#include "../src/core/eventloop.hpp"
#include "../src/core/method.tpp"
#include "../src/core/objectproperty.tpp"
#include "../src/world/world.hpp"
#include "../src/hardware/interface/interfacelist.hpp"
#include "../src/hardware/decoder/list/decoderlist.hpp"
#include "../src/hardware/input/input.hpp"
#include "../src/hardware/input/list/inputlist.hpp"
#include "../src/hardware/output/list/outputlist.hpp"
#include "../src/hardware/protocol/z21/clientkernel.hpp"
#include "../src/simulator/interfacesimulatorsettings.hpp"
#include "../../shared/src/traintastic/simulator/simulator.hpp"

TEMPLATE_TEST_CASE("Input", "[input]",
  DCCEXInterface,
  ECoSInterface,
  LocoNetInterface,
  XpressNetInterface,
  Z21Interface
  )
{
  using namespace std::chrono_literals;

  constexpr auto simulatorSensorChannels =
    []() -> std::array<uint16_t, 4>
    {
      if constexpr(std::is_same_v<TestType, Z21Interface>)
      {
        return {{0, 0, 1, 1}}; // 0 = RBus, 1 = LocoNet
      }
      return {{0, 0, 0, 0}};
    }();

  static const nlohmann::json layout
  {
    {"trackplan", {
      {
        {"type", "straight"},
        {"length", 200},
        {"sensor_channel", simulatorSensorChannels[0]},
        {"sensor_address", 1}
      },
      {
        {"type", "straight"},
        {"length", 200},
        {"sensor_channel", simulatorSensorChannels[1]},
        {"sensor_address", 2}
      },
      {
        {"type", "straight"},
        {"length", 200},
        {"sensor_channel", simulatorSensorChannels[2]},
        {"sensor_address", 3}
      },
      {
        {"type", "straight"},
        {"length", 200},
        {"sensor_channel", simulatorSensorChannels[3]},
        {"sensor_address", 4}
      },
    }},
    {"trains", {
      {
        //{"address", 3},
        //{"track_id", "top"},
        {"vehicles", {
          {
            {"length", 40.0}
          }}
        }
      }
    }}
  };

  // Setup simulator:
  auto simulator = std::make_shared<Simulator>(layout);
  simulator->enableServer(true, 0); // set port to 0 to let the OS choose a free port
  simulator->start();
  std::this_thread::sleep_for(100ms); // give it some time to start

  // Setup world:
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());

  std::weak_ptr<TestType> interfaceWeak = std::dynamic_pointer_cast<TestType>(world->interfaces->create(TestType::classId));
  REQUIRE_FALSE(interfaceWeak.expired());
  interfaceWeak.lock()->simulator->useSimulator = true;
  interfaceWeak.lock()->simulator->port = simulator->serverPort();

  using InputState = std::bitset<4>;
  InputState inputStateKnown(0);
  InputState inputState(0);
  std::vector<InputState> inputStates;
  inputStates.reserve(16);
  inputStates.push_back(inputState);

  auto inputValueChangedHandler =
    [&](bool value, const std::shared_ptr<Input>& input)
    {
      const auto index = input->address.value() - 1;
      REQUIRE((!inputStateKnown[index] || inputState[index] != value));
      inputState[index] = value;
      inputStateKnown[index] = true;
      inputStates.push_back(inputState);
    };

  std::weak_ptr<Input> input1 = interfaceWeak.lock()->inputs->create();
  input1.lock()->address = 1;
  REQUIRE(input1.lock()->address == 1);
  input1.lock()->onValueChanged.connect(inputValueChangedHandler);
  if constexpr(std::is_same_v<TestType, Z21Interface>)
  {
    input1.lock()->channel = Z21::ClientKernel::InputChannel::rbus;
    REQUIRE(input1.lock()->channel == Z21::ClientKernel::InputChannel::rbus);
    REQUIRE(input1.lock()->address == 1);
  }

  std::weak_ptr<Input> input2 = interfaceWeak.lock()->inputs->create();
  input2.lock()->address = 2;
  REQUIRE(input2.lock()->address == 2);
  input2.lock()->onValueChanged.connect(inputValueChangedHandler);
  if constexpr(std::is_same_v<TestType, Z21Interface>)
  {
    input2.lock()->channel = Z21::ClientKernel::InputChannel::rbus;
    REQUIRE(input2.lock()->channel == Z21::ClientKernel::InputChannel::rbus);
    REQUIRE(input2.lock()->address == 2);
  }

  std::weak_ptr<Input> input3 = interfaceWeak.lock()->inputs->create();
  input3.lock()->address = 3;
  REQUIRE(input3.lock()->address == 3);
  input3.lock()->onValueChanged.connect(inputValueChangedHandler);
  if constexpr(std::is_same_v<TestType, Z21Interface>)
  {
    input3.lock()->channel = Z21::ClientKernel::InputChannel::loconet;
    REQUIRE(input3.lock()->channel == Z21::ClientKernel::InputChannel::loconet);
    REQUIRE(input3.lock()->address == 3);
  }

  std::weak_ptr<Input> input4 = interfaceWeak.lock()->inputs->create();
  input4.lock()->address = 4;
  REQUIRE(input4.lock()->address == 4);
  input4.lock()->onValueChanged.connect(inputValueChangedHandler);
  if constexpr(std::is_same_v<TestType, Z21Interface>)
  {
    input4.lock()->channel = Z21::ClientKernel::InputChannel::loconet;
    REQUIRE(input4.lock()->channel == Z21::ClientKernel::InputChannel::loconet);
    REQUIRE(input4.lock()->address == 4);
  }

  REQUIRE(interfaceWeak.lock()->inputs->length.value() == 4);

  world->simulation = true;
  REQUIRE(world->simulation.value());

  EventLoop::call(
    [&]()
    {
      world->online();
      world->run();
      simulator->setTrainDirection(0, false);
      simulator->setTrainSpeed(0, simulator->staticData.trains[0].speedMax);
    });

  EventLoop::runFor(5s);

  if constexpr(std::is_same_v<TestType, ECoSInterface>)
  {
    REQUIRE(inputStates.size() == 12);
    REQUIRE(inputStates[0].to_ulong() == 0b0000);
    REQUIRE(inputStates[1].to_ulong() == 0b0000); // ECoS S88 multiple feedback values in one message,
    REQUIRE(inputStates[2].to_ulong() == 0b0000); // this causes the first change to trigger a change on all.
    REQUIRE(inputStates[3].to_ulong() == 0b0000); // In this test case 4 event.
    REQUIRE(inputStates[4].to_ulong() == 0b0000); //
    REQUIRE(inputStates[5].to_ulong() == 0b0001);
    REQUIRE(inputStates[6].to_ulong() == 0b0011);
    REQUIRE(inputStates[7].to_ulong() == 0b0010);
    REQUIRE(inputStates[8].to_ulong() == 0b0110);
    REQUIRE(inputStates[9].to_ulong() == 0b0100);
    REQUIRE(inputStates[10].to_ulong() == 0b1100);
    REQUIRE(inputStates[11].to_ulong() == 0b1000);
  }
  else if constexpr(std::is_same_v<TestType, XpressNetInterface>)
  {
    REQUIRE(inputStates.size() == 11);
    REQUIRE(inputStates[0].to_ulong() == 0b0000);
    REQUIRE(inputStates[1].to_ulong() == 0b0001); // XpressNet has multiple feedback values in one message,
    REQUIRE(inputStates[2].to_ulong() == 0b0001); // this causes the first change to trigger a change on all.
    REQUIRE(inputStates[3].to_ulong() == 0b0001); // In this test case 4 event.
    REQUIRE(inputStates[4].to_ulong() == 0b0001); //
    REQUIRE(inputStates[5].to_ulong() == 0b0011);
    REQUIRE(inputStates[6].to_ulong() == 0b0010);
    REQUIRE(inputStates[7].to_ulong() == 0b0110);
    REQUIRE(inputStates[8].to_ulong() == 0b0100);
    REQUIRE(inputStates[9].to_ulong() == 0b1100);
    REQUIRE(inputStates[10].to_ulong() == 0b1000);
  }
  else if constexpr(std::is_same_v<TestType, Z21Interface>)
  {
    REQUIRE(inputStates.size() == 9);
    REQUIRE(inputStates[0].to_ulong() == 0b0000);
    REQUIRE(inputStates[1].to_ulong() == 0b0001); // Z21 R-bus has multiple feedback values in one message,
    REQUIRE(inputStates[2].to_ulong() == 0b0001); // this causes the first change to trigger a change on all.
    REQUIRE(inputStates[3].to_ulong() == 0b0011);
    REQUIRE(inputStates[4].to_ulong() == 0b0010);
    REQUIRE(inputStates[5].to_ulong() == 0b0110);
    REQUIRE(inputStates[6].to_ulong() == 0b0100);
    REQUIRE(inputStates[7].to_ulong() == 0b1100);
    REQUIRE(inputStates[8].to_ulong() == 0b1000);
  }
  else
  {
    REQUIRE(inputStates.size() == 8);
    REQUIRE(inputStates[0].to_ulong() == 0b0000);
    REQUIRE(inputStates[1].to_ulong() == 0b0001);
    REQUIRE(inputStates[2].to_ulong() == 0b0011);
    REQUIRE(inputStates[3].to_ulong() == 0b0010);
    REQUIRE(inputStates[4].to_ulong() == 0b0110);
    REQUIRE(inputStates[5].to_ulong() == 0b0100);
    REQUIRE(inputStates[6].to_ulong() == 0b1100);
    REQUIRE(inputStates[7].to_ulong() == 0b1000);
  }

  EventLoop::call(
    [&]()
    {
      world->offline();
    });

  EventLoop::runFor(500ms);

  simulator->stop();
  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(interfaceWeak.expired());
  REQUIRE(input1.expired());
  REQUIRE(input2.expired());
  REQUIRE(input3.expired());
  REQUIRE(input4.expired());
}
