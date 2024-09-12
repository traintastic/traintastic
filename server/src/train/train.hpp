/**
 * server/src/train/train.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_TRAIN_TRAIN_HPP
#define TRAINTASTIC_SERVER_TRAIN_TRAIN_HPP

#include "../core/idobject.hpp"
#include <boost/asio/steady_timer.hpp>
#include <traintastic/enum/blocktraindirection.hpp>
#include <traintastic/enum/trainmode.hpp>
#include "../core/event.hpp"
#include "../core/method.hpp"
#include "../core/objectproperty.hpp"
#include "../core/objectvectorproperty.hpp"
#include "../core/lengthproperty.hpp"
#include "../core/speedproperty.hpp"
#include "../core/weightproperty.hpp"
#include "../enum/direction.hpp"

class TrainVehicleList;
class TrainBlockStatus;
class BlockRailTile;
class PoweredRailVehicle;
class Throttle;
class TrainSpeedTable;

class Train : public IdObject
{
  friend class TrainVehicleList;
  friend class PoweredRailVehicle;
  friend class TrainTracking;

  private:
    enum class SpeedState
    {
      Idle,
      Accelerating,
      Braking,
    };

    struct SpeedPoint
    {
        double speedMetersPerSecond;
        uint8_t tableIdx = 0;
    };

    SpeedPoint throttleSpeedPoint;
    SpeedPoint lastSetSpeedPoint;
    SpeedPoint maxSpeedPoint;

    std::vector<std::shared_ptr<PoweredRailVehicle>> m_poweredVehicles;
    std::unique_ptr<TrainSpeedTable> m_speedTable;
    bool m_speedTableNeedsRecalculation = false;

    std::chrono::steady_clock::time_point m_speedTimerStart;
    boost::asio::steady_timer m_speedTimer;
    SpeedState m_speedState = SpeedState::Idle;
    std::shared_ptr<Throttle> m_throttle;

    boost::asio::steady_timer m_delayedSpeedApplyTimer;
    std::shared_ptr<PoweredRailVehicle> m_delayedApplyLoco;

    //! \todo add realistic acceleration
    //! \note m/s^2 in physical model scale
    double m_accelerationRate = 1.5 / 87.0; // m/s^2

    //! \todo add realistic braking
    double m_brakingRate = -1.0 / 87.0; // m/s^2

    void setSpeed(const SpeedPoint &speedPoint);
    void setThrottleSpeed(const SpeedPoint &targetSpeed);

    void scheduleAccelerationFrom(double currentSpeed,
                                  uint8_t newTableIdx,
                                  SpeedState state);
    void updateSpeed();
    void updateSpeedTable();
    void scheduleSpeedTableUpdate();

    void startDelayedSpeedApply(const std::shared_ptr<PoweredRailVehicle> &vehicle);
    void stopDelayedSpeedApply();
    void applyDelayedSpeed();

    void driveLocomotive(const std::shared_ptr<PoweredRailVehicle> &vehicle,
                         uint8_t step);

    void vehiclesChanged();
    void updateLength();
    void updateWeight();
    void updatePowered();
    void updateSpeedMax();
    void updateEnabled();
    bool setTrainActive(bool val);
    void propagateDirection(Direction newDirection);
    void handleDecoderDirection(const std::shared_ptr<PoweredRailVehicle>& vehicle, Direction newDirection);
    void handleDecoderThrottle(const std::shared_ptr<PoweredRailVehicle>& vehicle, float newThrottle);

    void fireBlockReserved(const std::shared_ptr<BlockRailTile>& block, BlockTrainDirection trainDirection);
    void fireBlockEntered(const std::shared_ptr<BlockRailTile>& block, BlockTrainDirection trainDirection);
    void fireBlockLeft(const std::shared_ptr<BlockRailTile>& block, BlockTrainDirection trainDirection);

protected:
    void addToWorld() override;
    void destroying() override;
    void loaded() override;
    void worldEvent(WorldState state, WorldEvent event) override;

  public:
    CLASS_ID("train")
    CREATE_DEF(Train)

    Property<std::string> name;
    LengthProperty lob;
    Property<bool> overrideLength;
    Property<Direction> direction;
    Property<bool> isStopped;
    SpeedProperty speed;
    SpeedProperty speedMax;
    SpeedProperty throttleSpeed;
    Method<void()> stop;
    Property<bool> emergencyStop;
    WeightProperty weight;
    Property<bool> overrideWeight;
    ObjectProperty<TrainVehicleList> vehicles;
    Property<bool> powered;
    Property<bool> active;
    Property<TrainMode> mode;
    Property<bool> hasThrottle;
    Property<std::string> throttleName;

    //! \brief List of block status the train is in
    //! Index 0 is the block where the head of the train is.
    //! If the train changes direction this list will be reversed.
    ObjectVectorProperty<TrainBlockStatus> blocks;
    Property<std::string> notes;
    Event<const std::shared_ptr<Train>&, const std::shared_ptr<BlockRailTile>&> onBlockAssigned;
    Event<const std::shared_ptr<Train>&, const std::shared_ptr<BlockRailTile>&, BlockTrainDirection> onBlockReserved;
    Event<const std::shared_ptr<Train>&, const std::shared_ptr<BlockRailTile>&, BlockTrainDirection> onBlockEntered;
    Event<const std::shared_ptr<Train>&, const std::shared_ptr<BlockRailTile>&, BlockTrainDirection> onBlockLeft;
    Event<const std::shared_ptr<Train>&, const std::shared_ptr<BlockRailTile>&> onBlockRemoved;

    Train(World& world, std::string_view _id);

    std::error_code acquire(Throttle& throttle, bool steal = false);
    std::error_code release(Throttle& throttle);
    std::error_code setSpeed(Throttle& throttle, double value);
    std::error_code setTargetSpeed(Throttle& throttle, double value);
    std::error_code setDirection(Throttle& throttle, Direction value);

    void fireBlockAssigned(const std::shared_ptr<BlockRailTile>& block);
    void fireBlockRemoved(const std::shared_ptr<BlockRailTile>& block);
};

#endif
