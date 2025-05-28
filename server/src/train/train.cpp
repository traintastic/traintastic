/**
 * server/src/train/train.cpp
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

#include "train.hpp"
#include "trainerror.hpp"
#include "trainlist.hpp"
#include "trainvehiclelist.hpp"
#include "../world/world.hpp"
#include "trainblockstatus.hpp"
#include "trainlisttablemodel.hpp"
#include "../core/attributes.hpp"
#include "../core/method.tpp"
#include "../core/objectproperty.tpp"
#include "../core/objectvectorproperty.tpp"
#include "../core/eventloop.hpp"
#include "../board/tile/rail/blockrailtile.hpp"
#include "../vehicle/rail/poweredrailvehicle.hpp"
#include "../hardware/decoder/decoder.hpp"
#include "../throttle/throttle.hpp"
#include "../utils/almostzero.hpp"
#include "../utils/displayname.hpp"

CREATE_IMPL(Train)

static inline bool isPowered(const RailVehicle& vehicle)
{
  return dynamic_cast<const PoweredRailVehicle*>(&vehicle);
}

Train::Train(World& world, std::string_view _id) :
  IdObject(world, _id),
  m_speedTimer{EventLoop::ioContext},
  name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly},
  lob{*this, "lob", 0, LengthUnit::MilliMeter, PropertyFlags::ReadWrite | PropertyFlags::Store},
  overrideLength{this, "override_length", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](bool value)
    {
      Attributes::setEnabled(lob, value);
      if(!value)
        updateLength();
    }},
  direction{this, "direction", Direction::Forward, PropertyFlags::ReadWrite | PropertyFlags::StoreState,
    [this](Direction value)
    {
      // update train direction from the block perspective:
      for(const auto& status : *blocks)
        status->direction.setValueInternal(!status->direction.value());
      blocks.reverseInternal(); // index 0 is head of train

      for(const auto& vehicle : m_poweredVehicles)
        vehicle->setDirection(value);
      updateEnabled();
    },
    [](Direction& value)
    {
      // only accept valid direction values, don't accept Unknown direction
      return value == Direction::Forward || value == Direction::Reverse;
    }},
  isStopped{this, "is_stopped", true, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly},
  speed{*this, "speed", 0, SpeedUnit::KiloMeterPerHour, PropertyFlags::ReadOnly | PropertyFlags::NoStore},
  speedMax{*this, "speed_max", 0, SpeedUnit::KiloMeterPerHour, PropertyFlags::ReadWrite | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly},
  throttleSpeed{*this, "throttle_speed", 0, SpeedUnit::KiloMeterPerHour, PropertyFlags::ReadWrite | PropertyFlags::StoreState,
    [this](double value, SpeedUnit unit)
    {
      const double currentSpeed = speed.getValue(unit);

      emergencyStop.setValueInternal(false);

      if(value > currentSpeed) // Accelerate
      {
        if(m_speedState == SpeedState::Accelerate)
          return;

        m_speedTimer.cancel();
        m_speedState = SpeedState::Accelerate;
        updateSpeed();
      }
      else if(value < currentSpeed) // brake
      {
        if(m_speedState == SpeedState::Braking)
          return;

        m_speedTimer.cancel();
        m_speedState = SpeedState::Braking;
        updateSpeed();
      }
    }},
  stop{*this, "stop", MethodFlags::ScriptCallable,
    [this]()
    {
      throttleSpeed.setValue(0);
    }},
  emergencyStop{this, "emergency_stop", true, PropertyFlags::ReadWrite | PropertyFlags::StoreState | PropertyFlags::ScriptReadWrite,
    [this](bool value)
    {
      if(value)
      {
        m_speedState = SpeedState::Idle;
        m_speedTimer.cancel();
        throttleSpeed.setValueInternal(0);
        speed.setValueInternal(0);
        isStopped.setValueInternal(true);
        updateEnabled();
      }

      if(active)
      {
        //Propagate to all vehicles in this Train
        for(const auto& vehicle : m_poweredVehicles)
          vehicle->setEmergencyStop(value);
      }
    }},
  weight{*this, "weight", 0, WeightUnit::Ton, PropertyFlags::ReadWrite | PropertyFlags::Store},
  overrideWeight{this, "override_weight", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](bool value)
    {
      Attributes::setEnabled(weight, value);
      if(!value)
        updateWeight();
    }},
  vehicles{this, "vehicles", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject},
  powered{this, "powered", false, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly},
  active{this, "active", false, PropertyFlags::ReadWrite | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly,
    [this](bool value)
    {
      updateSpeed();
      if(!value && m_throttle)
      {
        m_throttle->release();
      }
    },
    std::bind(&Train::setTrainActive, this, std::placeholders::_1)},
  mode{this, "mode", TrainMode::ManualUnprotected, PropertyFlags::ReadWrite | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly},
  blocks{*this, "blocks", {}, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly},
  notes{this, "notes", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , onBlockAssigned{*this, "on_block_assigned", EventFlags::Scriptable}
  , onBlockReserved{*this, "on_block_reserved", EventFlags::Scriptable}
  , onBlockEntered{*this, "on_block_entered", EventFlags::Scriptable}
  , onBlockLeft{*this, "on_block_left", EventFlags::Scriptable}
  , onBlockRemoved{*this, "on_block_removed", EventFlags::Scriptable}
{
  vehicles.setValueInternal(std::make_shared<TrainVehicleList>(*this, vehicles.name()));

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, false);
  m_interfaceItems.add(name);
  Attributes::addEnabled(lob, overrideLength);
  m_interfaceItems.add(lob);
  m_interfaceItems.add(overrideLength);
  Attributes::addEnabled(direction, false);
  Attributes::addValues(direction, DirectionValues);
  Attributes::addObjectEditor(direction, false);
  m_interfaceItems.add(direction);

  Attributes::addObjectEditor(isStopped, false);
  m_interfaceItems.add(isStopped);

  Attributes::addObjectEditor(speed, false);
  Attributes::addMinMax(speed, 0., 0., SpeedUnit::KiloMeterPerHour);
  m_interfaceItems.add(speed);
  m_interfaceItems.add(speedMax);
  Attributes::addMinMax(throttleSpeed, 0., 0., SpeedUnit::KiloMeterPerHour);
  Attributes::addEnabled(throttleSpeed, false);
  Attributes::addObjectEditor(throttleSpeed, false);
  m_interfaceItems.add(throttleSpeed);

  Attributes::addEnabled(stop, false);
  Attributes::addObjectEditor(stop, false);
  m_interfaceItems.add(stop);

  Attributes::addEnabled(emergencyStop, false);
  Attributes::addObjectEditor(emergencyStop, false);
  m_interfaceItems.add(emergencyStop);

  Attributes::addEnabled(weight, overrideWeight);
  m_interfaceItems.add(weight);
  m_interfaceItems.add(overrideWeight);
  m_interfaceItems.add(vehicles);

  Attributes::addEnabled(active, true);
  m_interfaceItems.add(active);

  Attributes::addValues(mode, trainModeValues);
  m_interfaceItems.add(mode);

  Attributes::addObjectEditor(blocks, false);
  m_interfaceItems.add(blocks);

  Attributes::addObjectEditor(powered, false);
  m_interfaceItems.add(powered);
  Attributes::addDisplayName(notes, DisplayName::Object::notes);
  m_interfaceItems.add(notes);

  m_interfaceItems.add(onBlockAssigned);
  m_interfaceItems.add(onBlockReserved);
  m_interfaceItems.add(onBlockEntered);
  m_interfaceItems.add(onBlockLeft);
  m_interfaceItems.add(onBlockRemoved);

  updateEnabled();
}

void Train::addToWorld()
{
  IdObject::addToWorld();
  m_world.trains->addObject(shared_ptr<Train>());
}

void Train::destroying()
{
  auto self = shared_ptr<Train>();
  for(const auto& vehicle : *vehicles)
  {
    vehicle->trains.removeInternal(self);
  }
  m_world.trains->removeObject(self);
  IdObject::destroying();
}

void Train::loaded()
{
  IdObject::loaded();

  Attributes::setEnabled(lob, overrideLength);
  Attributes::setEnabled(weight, overrideWeight);

  vehiclesChanged();
}

void Train::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);

  updateEnabled();
}

void Train::setSpeed(const double kmph)
{
  for(const auto& vehicle : m_poweredVehicles)
    vehicle->setSpeed(kmph);
  speed.setValueInternal(convertUnit(kmph, SpeedUnit::KiloMeterPerHour, speed.unit()));
  updateEnabled();
}

void Train::updateSpeed()
{
  if(m_speedState == SpeedState::Idle)
    return;

  if(m_speedState == SpeedState::Accelerate && !active)
    return;

  const double targetSpeed = throttleSpeed.getValue(SpeedUnit::MeterPerSecond);
  double currentSpeed = speed.getValue(SpeedUnit::MeterPerSecond);

  double acceleration = 0;
  if(m_speedState == SpeedState::Accelerate)
  {
    //! \todo add realistic acceleration
    acceleration = 1; // m/s^2
  }
  else if(m_speedState == SpeedState::Braking)
  {
    //! \todo add realistic braking
    acceleration = -0.5; // m/s^2
  }
  else
    assert(false);

  currentSpeed += acceleration * 0.1; // x 100ms

  if((m_speedState == SpeedState::Accelerate && currentSpeed >= targetSpeed) ||
      (m_speedState == SpeedState::Braking && currentSpeed <= targetSpeed))
  {
    m_speedState = SpeedState::Idle;
    setSpeed(convertUnit(targetSpeed, SpeedUnit::MeterPerSecond, SpeedUnit::KiloMeterPerHour));
    currentSpeed = targetSpeed;
  }
  else
  {
    using namespace std::literals;
    setSpeed(convertUnit(currentSpeed, SpeedUnit::MeterPerSecond, SpeedUnit::KiloMeterPerHour));
    m_speedTimer.expires_after(100ms);
    m_speedTimer.async_wait(
      [this](const boost::system::error_code& ec)
      {
        if(!ec)
          updateSpeed();
      });
  }

  const bool currentValue = isStopped;
  isStopped.setValueInternal(m_speedState == SpeedState::Idle && almostZero(currentSpeed) && almostZero(targetSpeed));
  if(currentValue != isStopped)
    updateEnabled();
}

void Train::vehiclesChanged()
{
  updateLength();
  updateWeight();
  updatePowered();
  updateSpeedMax();
  updateEnabled();
}

void Train::updateLength()
{
  if(overrideLength)
    return;

  double mm = 0;
  for(const auto& vehicle : *vehicles)
    mm += vehicle->lob.getValue(LengthUnit::MilliMeter);
  lob.setValueInternal(convertUnit(mm, LengthUnit::MilliMeter, lob.unit()));
}

void Train::updateWeight()
{
  if(overrideWeight)
    return;

  double ton = 0;
  for(const auto& vehicle : *vehicles)
    ton += vehicle->totalWeight.getValue(WeightUnit::Ton);
  weight.setValueInternal(convertUnit(ton, WeightUnit::Ton, weight.unit()));
}

void Train::updatePowered()
{
  m_poweredVehicles.clear();
  for(const auto& vehicle : *vehicles)
    if(auto poweredVehicle = std::dynamic_pointer_cast<PoweredRailVehicle>(vehicle))
      m_poweredVehicles.emplace_back(poweredVehicle);
  powered.setValueInternal(!m_poweredVehicles.empty());
}

void Train::updateSpeedMax()
{
  if(!vehicles->empty() && powered)
  {
    const auto itEnd = vehicles->end();
    auto it = vehicles->begin();
    double kmph = (*it)->speedMax.getValue(SpeedUnit::KiloMeterPerHour);
    for(; it != itEnd; ++it)
    {
      const double v = (*it)->speedMax.getValue(SpeedUnit::KiloMeterPerHour);
      if((v > 0 || isPowered(**it)) && v < kmph)
        kmph = v;
    }
    speedMax.setValueInternal(convertUnit(kmph, SpeedUnit::KiloMeterPerHour, speedMax.unit()));
  }
  else
    speedMax.setValueInternal(0);

  speed.setUnit(speedMax.unit());
  Attributes::setMax(speed, speedMax.value(), speedMax.unit());
  throttleSpeed.setUnit(speedMax.unit());
  Attributes::setMax(throttleSpeed, speedMax.value(), speedMax.unit());
}

void Train::updateEnabled()
{
  const bool stopped = isStopped;
  const bool editable = contains(m_world.state, WorldState::Edit);

  Attributes::setEnabled(name, stopped && editable);
  Attributes::setEnabled(direction, stopped && powered);
  Attributes::setEnabled(throttleSpeed, powered);
  Attributes::setEnabled(stop, powered);
  Attributes::setEnabled(emergencyStop, powered);
  Attributes::setEnabled(vehicles->add, stopped);
  Attributes::setEnabled(vehicles->remove, stopped);
  Attributes::setEnabled(vehicles->move, stopped);
  Attributes::setEnabled(vehicles->reverse, stopped);
}

bool Train::setTrainActive(bool val)
{
  auto self = this->shared_ptr<Train>();

  if(val)
  {
    if(vehicles->empty())
    {
      return false; // activating a train without vehicles is useless.
    }

    //To activate a train, ensure all vehicles are stopped and free
    for(const auto& vehicle : *vehicles)
    {
      assert(vehicle->activeTrain.value() != self);
      if(vehicle->activeTrain.value())
      {
        return false; //Not free
      }

      if(auto decoder = vehicle->decoder.value(); decoder && !almostZero(decoder->throttle.value()))
      {
        return false; //Already running
      }
    }

    //Now really activate
    //Register this train as activeTrain
    for(const auto& vehicle : *vehicles)
    {
      vehicle->activeTrain.setValueInternal(self);
    }

    //Sync Emergency Stop state
    const bool stopValue = emergencyStop;
    for(const auto& vehicle : m_poweredVehicles)
      vehicle->setEmergencyStop(stopValue);
  }
  else
  {
    //To deactivate a Train it must be stopped first
    if(!isStopped)
      return false;

    //Deactivate all vehicles
    for(const auto& vehicle : *vehicles)
    {
      assert(vehicle->activeTrain.value() == self);
      vehicle->activeTrain.setValueInternal(nullptr);
    }
  }

  return true;
}

std::string Train::throttleName() const
{
  if(m_throttle)
  {
    return m_throttle->name;
  }
  return {};
}

std::error_code Train::acquire(Throttle& throttle, bool steal)
{
  if(m_throttle)
  {
    if(!steal)
    {
      return make_error_code(TrainError::AlreadyAcquired);
    }
    m_throttle->release();
  }
  if(!active)
  {
    try
    {
      active = true; // TODO: activate();
    }
    catch(...)
    {
    }
    if(!active)
    {
      return make_error_code(TrainError::CanNotActivateTrain);
    }
  }
  assert(!m_throttle);
  m_throttle = throttle.shared_ptr<Throttle>();
  return {};
}

std::error_code Train::release(Throttle& throttle)
{
  if(m_throttle.get() != &throttle)
  {
    return make_error_code(TrainError::InvalidThrottle);
  }
  m_throttle.reset();
  if(isStopped && blocks.empty())
  {
    active = false; // deactive train if it is stopped and not assigned to a block
  }
  return {};
}

std::error_code Train::setSpeed(Throttle& throttle, double value)
{
  if(m_throttle.get() != &throttle)
  {
    return make_error_code(TrainError::InvalidThrottle);
  }
  assert(active);

  value = std::clamp(value, Attributes::getMin(speed), Attributes::getMax(speed));

  setSpeed(convertUnit(value, speed.unit(), SpeedUnit::KiloMeterPerHour));
  throttleSpeed.setValue(convertUnit(value, speed.unit(), throttleSpeed.unit()));
  m_speedTimer.cancel();
  m_speedState = SpeedState::Idle;

  const bool currentValue = isStopped;
  isStopped.setValueInternal(m_speedState == SpeedState::Idle && almostZero(speed.value()) && almostZero(throttleSpeed.value()));
  if(currentValue != isStopped)
  {
    updateEnabled();
  }
  return {};
}

std::error_code Train::setTargetSpeed(Throttle& throttle, double value)
{
  if(m_throttle.get() != &throttle)
  {
    return make_error_code(TrainError::InvalidThrottle);
  }
  assert(active);
  throttleSpeed.setValue(std::clamp(value, Attributes::getMin(throttleSpeed), Attributes::getMax(throttleSpeed)));
  return {};
}

std::error_code Train::setDirection(Throttle& throttle, Direction value)
{
  if(m_throttle.get() != &throttle)
  {
    return make_error_code(TrainError::InvalidThrottle);
  }
  if(direction != value)
  {
    if(!isStopped)
    {
      return make_error_code(TrainError::TrainMustBeStoppedToChangeDirection);
    }
    assert(active);
    direction = value;
  }
  return {};
}

void Train::fireBlockAssigned(const std::shared_ptr<BlockRailTile>& block)
{
  fireEvent(
    onBlockAssigned,
    shared_ptr<Train>(),
    block);
}

void Train::fireBlockReserved(const std::shared_ptr<BlockRailTile>& block, BlockTrainDirection trainDirection)
{
  fireEvent(
    onBlockReserved,
    shared_ptr<Train>(),
    block,
    trainDirection);
}

void Train::fireBlockEntered(const std::shared_ptr<BlockRailTile>& block, BlockTrainDirection trainDirection)
{
  fireEvent(
    onBlockEntered,
    shared_ptr<Train>(),
    block,
    trainDirection);
}

void Train::fireBlockLeft(const std::shared_ptr<BlockRailTile>& block, BlockTrainDirection trainDirection)
{
  fireEvent(
    onBlockLeft,
    shared_ptr<Train>(),
    block,
    trainDirection);
}

void Train::fireBlockRemoved(const std::shared_ptr<BlockRailTile>& block)
{
  fireEvent(
    onBlockRemoved,
    shared_ptr<Train>(),
    block);
}
