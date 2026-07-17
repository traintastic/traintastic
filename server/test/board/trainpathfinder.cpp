#include <catch2/catch_test_macros.hpp>
#include "../../src/core/eventloop.hpp"
#include "../../src/world/world.hpp"
#include "../../src/core/method.tpp"
#include "../../src/core/objectproperty.tpp"
#include "../../src/board/board.hpp"
#include "../../src/board/boardlist.hpp"
#include "../../src/board/map/blockpath.hpp"
#include "../../src/board/pathfinder/trainpathfinder.hpp"
#include "../../src/board/tile/rail/blockrailtile.hpp"
#include "../../src/board/tile/rail/curve90railtile.hpp"
#include "../../src/board/tile/rail/straightrailtile.hpp"
#include "../../src/hardware/decoder/decoder.hpp"
#include "../../src/vehicle/rail/railvehiclelist.hpp"
#include "../../src/vehicle/rail/locomotive.hpp"
#include "../../src/train/trainlist.hpp"
#include "../../src/train/train.hpp"
#include "../../src/train/trainvehiclelist.hpp"
#include "../../src/train/trainblockstatus.hpp"

TEST_CASE("Board/TrainPathFinder: Direct block to block, variant 1", "[board][train-path-finder]")
{
  EventLoop::reset();

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  // Board:
  // +-----+    +-----+
  // |A 1 B|----|A 2 B|
  // +-----+    +-----+
  std::weak_ptr<Board> boardWeak = world->boards->create();

  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg90, BlockRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(1, 0, TileRotate::Deg90, StraightRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(2, 0, TileRotate::Deg90, BlockRailTile::classId, false));

  std::weak_ptr<BlockRailTile> block1 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 0}));
  REQUIRE_FALSE(block1.expired());
  std::weak_ptr<BlockRailTile> block2 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({2, 0}));
  REQUIRE_FALSE(block2.expired());

  // Set blocks free:
  REQUIRE(block1.lock()->state == BlockState::Unknown);
  REQUIRE(block1.lock()->setStateFree());
  REQUIRE(block1.lock()->state == BlockState::Free);
  REQUIRE(block2.lock()->state == BlockState::Unknown);
  REQUIRE(block2.lock()->setStateFree());
  REQUIRE(block2.lock()->state == BlockState::Free);

  // Create a train:
  std::weak_ptr<RailVehicle> locomotive1 = world->railVehicles->create(Locomotive::classId);
  std::weak_ptr<Train> train1 = world->trains->create();
  REQUIRE(train1.lock()->vehicles->length == 0);
  train1.lock()->vehicles->add(locomotive1.lock());
  REQUIRE(train1.lock()->vehicles->length == 1);

  // Assign train to block 1:
  block1.lock()->assignTrain(train1.lock());
  REQUIRE(block1.lock()->state == BlockState::Reserved);
  REQUIRE(block1.lock()->trains.size() == 1);
  REQUIRE(block1.lock()->trains.front()->direction.value() == BlockTrainDirection::TowardsB);

  world->run(); // this will build the board network
  world->stop();

  REQUIRE(block1.lock()->paths().size() == 1);
  REQUIRE(block1.lock()->paths().front()->fromSide() == BlockSide::B);
  REQUIRE(block1.lock()->paths().front()->toBlock() == block2.lock());
  REQUIRE(block1.lock()->paths().front()->toSide() == BlockSide::A);

  REQUIRE(block2.lock()->paths().size() == 1);
  REQUIRE(block2.lock()->paths().front()->fromSide() == BlockSide::A);
  REQUIRE(block2.lock()->paths().front()->toBlock() == block1.lock());
  REQUIRE(block2.lock()->paths().front()->toSide() == BlockSide::B);

  // Reserve the path:
  REQUIRE(world->trainPathFinder->reserve(block1.lock(), BlockTrainDirection::TowardsB, block2.lock(), BlockTrainDirection::TowardsB));

  REQUIRE(block2.lock()->trains.size() == 1);
  REQUIRE(block2.lock()->trains.front()->direction.value() == BlockTrainDirection::TowardsB);

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(block1.expired());
  REQUIRE(block2.expired());
  REQUIRE(locomotive1.expired());
  REQUIRE(train1.expired());
}

TEST_CASE("Board/TrainPathFinder: Direct block to block, variant 2", "[board][train-path-finder]")
{
  EventLoop::reset();

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  // Board:
  //        +---+
  //        | B |
  //        | 2 |
  //        | A |
  //        +---+
  // +-----+  |
  // |A 1 B|--/
  // +-----+
  std::weak_ptr<Board> boardWeak = world->boards->create();

  REQUIRE(boardWeak.lock()->addTile(0, 1, TileRotate::Deg90, BlockRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(1, 1, TileRotate::Deg90, Curve90RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(1, 0, TileRotate::Deg0, BlockRailTile::classId, false));

  std::weak_ptr<BlockRailTile> block1 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 1}));
  REQUIRE_FALSE(block1.expired());
  std::weak_ptr<BlockRailTile> block2 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({1, 0}));
  REQUIRE_FALSE(block2.expired());

  // Set blocks free:
  REQUIRE(block1.lock()->state == BlockState::Unknown);
  REQUIRE(block1.lock()->setStateFree());
  REQUIRE(block1.lock()->state == BlockState::Free);
  REQUIRE(block2.lock()->state == BlockState::Unknown);
  REQUIRE(block2.lock()->setStateFree());
  REQUIRE(block2.lock()->state == BlockState::Free);

  // Create a train:
  std::weak_ptr<RailVehicle> locomotive1 = world->railVehicles->create(Locomotive::classId);
  std::weak_ptr<Train> train1 = world->trains->create();
  REQUIRE(train1.lock()->vehicles->length == 0);
  train1.lock()->vehicles->add(locomotive1.lock());
  REQUIRE(train1.lock()->vehicles->length == 1);

  // Assign train to block 1:
  block1.lock()->assignTrain(train1.lock());
  REQUIRE(block1.lock()->state == BlockState::Reserved);
  REQUIRE(block1.lock()->trains.size() == 1);
  REQUIRE(block1.lock()->trains.front()->direction.value() == BlockTrainDirection::TowardsB);

  world->run(); // this will build the board network
  world->stop();

  REQUIRE(block1.lock()->paths().size() == 1);
  REQUIRE(block1.lock()->paths().front()->fromSide() == BlockSide::B);
  REQUIRE(block1.lock()->paths().front()->toBlock() == block2.lock());
  REQUIRE(block1.lock()->paths().front()->toSide() == BlockSide::A);

  REQUIRE(block2.lock()->paths().size() == 1);
  REQUIRE(block2.lock()->paths().front()->fromSide() == BlockSide::A);
  REQUIRE(block2.lock()->paths().front()->toBlock() == block1.lock());
  REQUIRE(block2.lock()->paths().front()->toSide() == BlockSide::B);

  // Reserve the path:
  REQUIRE(world->trainPathFinder->reserve(block1.lock(), BlockTrainDirection::TowardsB, block2.lock(), BlockTrainDirection::TowardsB));

  REQUIRE(block2.lock()->trains.size() == 1);
  REQUIRE(block2.lock()->trains.front()->direction.value() == BlockTrainDirection::TowardsB);

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(block1.expired());
  REQUIRE(block2.expired());
  REQUIRE(locomotive1.expired());
  REQUIRE(train1.expired());
}

TEST_CASE("Board/TrainPathFinder: Direct block to block, variant 3", "[board][train-path-finder]")
{
  EventLoop::reset();

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  // Board:
  // +-----+
  // |A 1 B|--\                                     (line may not end with a backslash)
  // +-----+  |
  //        +---+
  //        | B |
  //        | 2 |
  //        | A |
  //        +---+
  std::weak_ptr<Board> boardWeak = world->boards->create();

  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg90, BlockRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(1, 0, TileRotate::Deg0, Curve90RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(1, 1, TileRotate::Deg0, BlockRailTile::classId, false));

  std::weak_ptr<BlockRailTile> block1 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 0}));
  REQUIRE_FALSE(block1.expired());
  std::weak_ptr<BlockRailTile> block2 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({1, 1}));
  REQUIRE_FALSE(block2.expired());

  // Set blocks free:
  REQUIRE(block1.lock()->state == BlockState::Unknown);
  REQUIRE(block1.lock()->setStateFree());
  REQUIRE(block1.lock()->state == BlockState::Free);
  REQUIRE(block2.lock()->state == BlockState::Unknown);
  REQUIRE(block2.lock()->setStateFree());
  REQUIRE(block2.lock()->state == BlockState::Free);

  // Create a train:
  std::weak_ptr<RailVehicle> locomotive1 = world->railVehicles->create(Locomotive::classId);
  std::weak_ptr<Train> train1 = world->trains->create();
  REQUIRE(train1.lock()->vehicles->length == 0);
  train1.lock()->vehicles->add(locomotive1.lock());
  REQUIRE(train1.lock()->vehicles->length == 1);

  // Assign train to block 1:
  block1.lock()->assignTrain(train1.lock());
  REQUIRE(block1.lock()->state == BlockState::Reserved);
  REQUIRE(block1.lock()->trains.size() == 1);
  REQUIRE(block1.lock()->trains.front()->direction.value() == BlockTrainDirection::TowardsB);

  world->run(); // this will build the board network
  world->stop();

  REQUIRE(block1.lock()->paths().size() == 1);
  REQUIRE(block1.lock()->paths().front()->fromSide() == BlockSide::B);
  REQUIRE(block1.lock()->paths().front()->toBlock() == block2.lock());
  REQUIRE(block1.lock()->paths().front()->toSide() == BlockSide::B);

  REQUIRE(block2.lock()->paths().size() == 1);
  REQUIRE(block2.lock()->paths().front()->fromSide() == BlockSide::B);
  REQUIRE(block2.lock()->paths().front()->toBlock() == block1.lock());
  REQUIRE(block2.lock()->paths().front()->toSide() == BlockSide::B);

  // Reserve the path:
  REQUIRE(world->trainPathFinder->reserve(block1.lock(), BlockTrainDirection::TowardsB, block2.lock(), BlockTrainDirection::TowardsA));

  REQUIRE(block2.lock()->trains.size() == 1);
  REQUIRE(block2.lock()->trains.front()->direction.value() == BlockTrainDirection::TowardsA);

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(block1.expired());
  REQUIRE(block2.expired());
  REQUIRE(locomotive1.expired());
  REQUIRE(train1.expired());
}

TEST_CASE("Board/TrainPathFinder: Direct block to block, variant 4", "[board][train-path-finder]")
{
  EventLoop::reset();

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  // Board:
  // +-----+
  // |A 1 B|--\                                     (line may not end with a backslash)
  // +-----+  |
  // +-----+  |
  // |A 2 B|--/
  // +-----+
  std::weak_ptr<Board> boardWeak = world->boards->create();

  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg90, BlockRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(1, 0, TileRotate::Deg0, Curve90RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(1, 1, TileRotate::Deg90, Curve90RailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(0, 1, TileRotate::Deg90, BlockRailTile::classId, false));

  std::weak_ptr<BlockRailTile> block1 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 0}));
  REQUIRE_FALSE(block1.expired());
  std::weak_ptr<BlockRailTile> block2 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 1}));
  REQUIRE_FALSE(block2.expired());

  // Set blocks free:
  REQUIRE(block1.lock()->state == BlockState::Unknown);
  REQUIRE(block1.lock()->setStateFree());
  REQUIRE(block1.lock()->state == BlockState::Free);
  REQUIRE(block2.lock()->state == BlockState::Unknown);
  REQUIRE(block2.lock()->setStateFree());
  REQUIRE(block2.lock()->state == BlockState::Free);

  // Create a train:
  std::weak_ptr<RailVehicle> locomotive1 = world->railVehicles->create(Locomotive::classId);
  std::weak_ptr<Train> train1 = world->trains->create();
  REQUIRE(train1.lock()->vehicles->length == 0);
  train1.lock()->vehicles->add(locomotive1.lock());
  REQUIRE(train1.lock()->vehicles->length == 1);

  // Assign train to block 1:
  block1.lock()->assignTrain(train1.lock());
  REQUIRE(block1.lock()->state == BlockState::Reserved);
  REQUIRE(block1.lock()->trains.size() == 1);
  REQUIRE(block1.lock()->trains.front()->direction.value() == BlockTrainDirection::TowardsB);

  world->run(); // this will build the board network
  world->stop();

  REQUIRE(block1.lock()->paths().size() == 1);
  REQUIRE(block1.lock()->paths().front()->fromSide() == BlockSide::B);
  REQUIRE(block1.lock()->paths().front()->toBlock() == block2.lock());
  REQUIRE(block1.lock()->paths().front()->toSide() == BlockSide::B);

  REQUIRE(block2.lock()->paths().size() == 1);
  REQUIRE(block2.lock()->paths().front()->fromSide() == BlockSide::B);
  REQUIRE(block2.lock()->paths().front()->toBlock() == block1.lock());
  REQUIRE(block2.lock()->paths().front()->toSide() == BlockSide::B);

  // Reserve the path:
  REQUIRE(world->trainPathFinder->reserve(block1.lock(), BlockTrainDirection::TowardsB, block2.lock(), BlockTrainDirection::TowardsA));

  REQUIRE(block2.lock()->trains.size() == 1);
  REQUIRE(block2.lock()->trains.front()->direction.value() == BlockTrainDirection::TowardsA);

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(block1.expired());
  REQUIRE(block2.expired());
  REQUIRE(locomotive1.expired());
  REQUIRE(train1.expired());
}

TEST_CASE("Board/TrainPathFinder: Direct block to block, variant 5", "[board][train-path-finder]")
{
  EventLoop::reset();

  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;

  // Board:
  // +-----+    +-----+
  // |A 2 B|----|A 1 B|
  // +-----+    +-----+
  std::weak_ptr<Board> boardWeak = world->boards->create();

  REQUIRE(boardWeak.lock()->addTile(2, 0, TileRotate::Deg90, BlockRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(1, 0, TileRotate::Deg90, StraightRailTile::classId, false));
  REQUIRE(boardWeak.lock()->addTile(0, 0, TileRotate::Deg90, BlockRailTile::classId, false));

  std::weak_ptr<BlockRailTile> block1 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({2, 0}));
  REQUIRE_FALSE(block1.expired());
  std::weak_ptr<BlockRailTile> block2 = std::dynamic_pointer_cast<BlockRailTile>(boardWeak.lock()->getTile({0, 0}));
  REQUIRE_FALSE(block2.expired());

  // Set blocks free:
  REQUIRE(block1.lock()->state == BlockState::Unknown);
  REQUIRE(block1.lock()->setStateFree());
  REQUIRE(block1.lock()->state == BlockState::Free);
  REQUIRE(block2.lock()->state == BlockState::Unknown);
  REQUIRE(block2.lock()->setStateFree());
  REQUIRE(block2.lock()->state == BlockState::Free);

  // Create a train:
  std::weak_ptr<RailVehicle> locomotive1 = world->railVehicles->create(Locomotive::classId);
  std::weak_ptr<Train> train1 = world->trains->create();
  REQUIRE(train1.lock()->vehicles->length == 0);
  train1.lock()->vehicles->add(locomotive1.lock());
  REQUIRE(train1.lock()->vehicles->length == 1);

  // Assign train to block 1:
  block1.lock()->assignTrain(train1.lock());
  REQUIRE(block1.lock()->state == BlockState::Reserved);
  REQUIRE(block1.lock()->trains.size() == 1);
  REQUIRE(block1.lock()->trains.front()->direction.value() == BlockTrainDirection::TowardsB);

  block1.lock()->flipTrain();
  REQUIRE(block1.lock()->state == BlockState::Reserved);
  REQUIRE(block1.lock()->trains.size() == 1);
  REQUIRE(block1.lock()->trains.front()->direction.value() == BlockTrainDirection::TowardsA);

  world->run(); // this will build the board network
  world->stop();

  REQUIRE(block1.lock()->paths().size() == 1);
  REQUIRE(block1.lock()->paths().front()->fromSide() == BlockSide::A);
  REQUIRE(block1.lock()->paths().front()->toBlock() == block2.lock());
  REQUIRE(block1.lock()->paths().front()->toSide() == BlockSide::B);

  REQUIRE(block2.lock()->paths().size() == 1);
  REQUIRE(block2.lock()->paths().front()->fromSide() == BlockSide::B);
  REQUIRE(block2.lock()->paths().front()->toBlock() == block1.lock());
  REQUIRE(block2.lock()->paths().front()->toSide() == BlockSide::A);

  // Reserve the path:
  REQUIRE(world->trainPathFinder->reserve(block1.lock(), BlockTrainDirection::TowardsA, block2.lock(), BlockTrainDirection::TowardsA));

  REQUIRE(block2.lock()->trains.size() == 1);
  REQUIRE(block2.lock()->trains.front()->direction.value() == BlockTrainDirection::TowardsA);

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(boardWeak.expired());
  REQUIRE(block1.expired());
  REQUIRE(block2.expired());
  REQUIRE(locomotive1.expired());
  REQUIRE(train1.expired());
}
