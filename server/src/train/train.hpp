/**
 * server/src/train/train.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2023 Reinder Feenstra
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
#include <traintastic/enum/trainmode.hpp>
#include "../core/method.hpp"
#include "../core/objectproperty.hpp"
#include "../core/objectvectorproperty.hpp"
#include "../core/lengthproperty.hpp"
#include "../core/speedproperty.hpp"
#include "../core/weightproperty.hpp"
#include "../enum/direction.hpp"

class TrainVehicleList;
class TrainBlockStatus;
class PoweredRailVehicle;

class Train : public IdObject
{
  friend class TrainVehicleList;

  private:
    enum class SpeedState
    {
      Idle,
      Accelerate,
      Braking,
    };

    std::vector<std::shared_ptr<PoweredRailVehicle>> m_poweredVehicles;

    boost::asio::steady_timer m_speedTimer;
    SpeedState m_speedState = SpeedState::Idle;

    void setSpeed(double kmph);
    void updateSpeed();

    void vehiclesChanged();
    void updateLength();
    void updateWeight();
    void updatePowered();
    void updateSpeedMax();
    void updateEnabled();
    bool setTrainActive(bool val);

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

    //! \brief List of block status the train is in
    //! Index 0 is the block where the head of the train is.
    //! If the train changes direction this list will be reversed.
    ObjectVectorProperty<TrainBlockStatus> blocks;
    Property<std::string> notes;

    Train(World& world, std::string_view _id);
};

#endif
