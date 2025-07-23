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
#include "interfaces.hpp"
#include "../src/core/eventloop.hpp"
#include "../src/core/method.tpp"
#include "../src/core/objectproperty.tpp"
#include "../src/world/world.hpp"
#include "../src/hardware/interface/interfacelist.hpp"
#include "../src/hardware/decoder/list/decoderlist.hpp"
#include "../src/hardware/input/list/inputlist.hpp"
#include "../src/hardware/output/list/outputlist.hpp"
#include "../src/vehicle/rail/locomotive.hpp"
#include "../src/vehicle/rail/railvehiclelist.hpp"
#include "../src/train/train.hpp"
#include "../src/train/trainlist.hpp"
#include "../src/train/trainvehiclelist.hpp"
#include "../src/simulator/interfacesimulatorsettings.hpp"
#include "../../shared/src/traintastic/simulator/simulator.hpp"

TEMPLATE_TEST_CASE("Decoder - forward, reverse", "[decoder]",
  DCCEXInterface,
  LocoNetInterface,
  MarklinCANInterface,
  XpressNetInterface,
  Z21Interface
  )
{
  using namespace std::chrono_literals;

  static const nlohmann::json layout
  {
    {"trackplan", {
      {
        {"id", "start"},
        {"type", "straight"},
        {"length", 50},
        {"sensor_address", 1}
      },
      {
        {"type", "straight"},
        {"length", 50},
        {"sensor_address", 2}
      }
    }},
    {"trains", {
      {
        {"address", 3},
        {"track_id", "start"},
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

  std::weak_ptr<Decoder> decoderWeak = interfaceWeak.lock()->decoders->create();
  decoderWeak.lock()->address = 3;
  REQUIRE(decoderWeak.lock()->address.value() == 3);

  std::weak_ptr<Locomotive> locomotiveWeak = std::dynamic_pointer_cast<Locomotive>(world->railVehicles->create(Locomotive::classId));
  locomotiveWeak.lock()->decoder = decoderWeak.lock();
  locomotiveWeak.lock()->speedMax.setUnit(SpeedUnit::KiloMeterPerHour);
  locomotiveWeak.lock()->speedMax.setValue(50.0);

  std::weak_ptr<Train> trainWeak = world->trains->create();
  trainWeak.lock()->vehicles->add(locomotiveWeak.lock());
  trainWeak.lock()->throttleSpeed.setUnit(SpeedUnit::KiloMeterPerHour);
  trainWeak.lock()->active = true;
  REQUIRE(trainWeak.lock()->active.value());

  world->simulation = true;
  REQUIRE(world->simulation.value());

  EventLoop::call(
    [&]()
    {
      world->online();
      world->run();
    });
  EventLoop::runFor(250ms);
  {
    REQUIRE(simulator->stateData().powerOn);
    REQUIRE(interfaceWeak.lock()->online.value());
  }

  // Forward:
  EventLoop::call(
    [&]()
    {
      trainWeak.lock()->emergencyStop = false;
      trainWeak.lock()->direction = Direction::Forward;
      trainWeak.lock()->throttleSpeed.setValue(50.0);
    });
  EventLoop::runFor(1s);
  {
    const auto stateData = simulator->stateData();
    REQUIRE(stateData.powerOn);
    REQUIRE(stateData.trains[0].speed > 0.0f);
    REQUIRE_FALSE(stateData.trains[0].reverse);
  }
  EventLoop::runFor(3s);
  {
    const auto stateData = simulator->stateData();
    REQUIRE(stateData.powerOn);
    REQUIRE(stateData.trains[0].speed == 0.0f);
    REQUIRE_FALSE(stateData.sensors[0].value);
    REQUIRE(stateData.sensors[1].value);
  }

  // FIXME: train is stopped, but Traintastic doesn't know yet...

  // Reverse:
  EventLoop::call(
    [&]()
    {
      trainWeak.lock()->emergencyStop = true;
      trainWeak.lock()->emergencyStop = false;
      trainWeak.lock()->direction = Direction::Reverse;
      trainWeak.lock()->throttleSpeed.setValue(50.0);
    });
  EventLoop::runFor(1s);
  {
    const auto stateData = simulator->stateData();
    REQUIRE(stateData.powerOn);
    REQUIRE(stateData.trains[0].speed > 0.0f);
    REQUIRE(stateData.trains[0].reverse);
  }
  EventLoop::runFor(3s);
  {
    const auto stateData = simulator->stateData();
    REQUIRE(stateData.powerOn);
    REQUIRE(stateData.trains[0].speed == 0.0f);
    REQUIRE(stateData.sensors[0].value);
    REQUIRE_FALSE(stateData.sensors[1].value);
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
  REQUIRE(decoderWeak.expired());
  REQUIRE(locomotiveWeak.expired());
  REQUIRE(trainWeak.expired());

  EventLoop::stop();
}
