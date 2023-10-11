#include <catch2/catch.hpp>
#include "../../src/core/objectproperty.tpp"
#include "../../src/core/method.tpp"
#include "../../src/world/world.hpp"
#include "../../src/vehicle/rail/railvehiclelist.hpp"
#include "../../src/vehicle/rail/locomotive.hpp"
#include "../../src/train/trainlist.hpp"
#include "../../src/train/train.hpp"
#include "../../src/train/trainvehiclelist.hpp"
#include "../../src/hardware/decoder/decoder.hpp"
#include "../../src/log/logmessageexception.hpp"

TEST_CASE("Activate empty train", "[train]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());
  REQUIRE(worldWeak.lock()->trains->length == 0);

  std::weak_ptr<Train> trainWeak = world->trains->create();
  REQUIRE_FALSE(trainWeak.expired());
  REQUIRE(worldWeak.lock()->trains->length == 1);

  REQUIRE_FALSE(trainWeak.lock()->active.value());
  CHECK_THROWS_AS(trainWeak.lock()->active = true, invalid_value_error);
  REQUIRE_FALSE(trainWeak.lock()->active.value());
}

TEST_CASE("Delete active train", "[train]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());
  REQUIRE(world->trains->length == 0);

  std::weak_ptr<RailVehicle> locomotiveWeak = world->railVehicles->create(Locomotive::classId);
  REQUIRE_FALSE(locomotiveWeak.expired());
  REQUIRE(world->railVehicles->length == 1);

  std::weak_ptr<Train> trainWeak = world->trains->create();
  REQUIRE_FALSE(trainWeak.expired());
  REQUIRE(world->trains->length == 1);

  REQUIRE(trainWeak.lock()->vehicles->length == 0);
  trainWeak.lock()->vehicles->add(locomotiveWeak.lock());
  REQUIRE(trainWeak.lock()->vehicles->length == 1);

  REQUIRE_FALSE(trainWeak.lock()->active.value());
  trainWeak.lock()->active = true;
  REQUIRE(trainWeak.lock()->active.value());

  CHECK_THROWS_AS(world->trains->delete_(trainWeak.lock()), LogMessageException);
  REQUIRE(world->trains->length == 1);

  world.reset();
  REQUIRE(locomotiveWeak.expired());
  REQUIRE(trainWeak.expired());
  REQUIRE(worldWeak.expired());
}

TEST_CASE("Delete rail vehicle in active train", "[train]")
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());
  REQUIRE(world->trains->length == 0);

  std::weak_ptr<RailVehicle> locomotiveWeak = world->railVehicles->create(Locomotive::classId);
  REQUIRE_FALSE(locomotiveWeak.expired());
  REQUIRE(world->railVehicles->length == 1);

  std::weak_ptr<Train> trainWeak = world->trains->create();
  REQUIRE_FALSE(trainWeak.expired());
  REQUIRE(world->trains->length == 1);

  REQUIRE(trainWeak.lock()->vehicles->length == 0);
  trainWeak.lock()->vehicles->add(locomotiveWeak.lock());
  REQUIRE(trainWeak.lock()->vehicles->length == 1);

  REQUIRE_FALSE(trainWeak.lock()->active.value());
  trainWeak.lock()->active = true;
  REQUIRE(trainWeak.lock()->active.value());

  CHECK_THROWS_AS(world->railVehicles->delete_(locomotiveWeak.lock()), LogMessageException);
  REQUIRE(world->railVehicles->length == 1);

  world.reset();
  REQUIRE(locomotiveWeak.expired());
  REQUIRE(trainWeak.expired());
  REQUIRE(worldWeak.expired());
}
