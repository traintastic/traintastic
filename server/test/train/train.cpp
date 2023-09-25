#include <catch2/catch.hpp>
#include "../../src/core/objectproperty.tpp"
#include "../../src/core/method.tpp"
#include "../../src/world/world.hpp"
#include "../../src/train/trainlist.hpp"
#include "../../src/train/train.hpp"

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
