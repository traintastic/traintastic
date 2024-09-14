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

#include <cmath>
#include "train.hpp"
#include "trainblockstatus.hpp"
#include "trainlist.hpp"
#include "trainlisttablemodel.hpp"
#include "trainspeedtable.hpp"
#include "trainvehiclelist.hpp"
#include "trainvehiclelistitem.hpp"
#include "../world/world.hpp"
#include "../core/attributes.hpp"
#include "../core/errorcode.hpp"
#include "../core/method.tpp"
#include "../core/objectproperty.tpp"
#include "../core/objectvectorproperty.tpp"
#include "../core/eventloop.hpp"
#include "../board/tile/rail/blockrailtile.hpp"
#include "../vehicle/rail/poweredrailvehicle.hpp"
#include "../vehicle/rail/vehiclespeedcurve.hpp"
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
  m_delayedSpeedApplyTimer{EventLoop::ioContext},
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

      propagateDirection(value);
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
      double modelSpeed = convertUnit(value, unit, SpeedUnit::MeterPerSecond);
      modelSpeed /= m_world.scaleRatio;

      SpeedPoint speedPoint;
      speedPoint.speedMetersPerSecond = modelSpeed;

      // Table match will be calculated later
      speedPoint.tableIdx = TrainSpeedTable::NULL_TABLE_ENTRY;

      emergencyStop.setValueInternal(false);

      setThrottleSpeed(speedPoint);
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

        // Reset speed point to zero
        stopDelayedSpeedApply();
        lastSetSpeedPoint = throttleSpeedPoint = SpeedPoint();

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
      propagateDirection(direction); //Sync all vehicles direction
      updateSpeed(); // TODO: no-op?
      if(!value && m_throttle)
      {
        m_throttle->release();
      }
    },
    std::bind(&Train::setTrainActive, this, std::placeholders::_1)},
  mode{this, "mode", TrainMode::ManualUnprotected, PropertyFlags::ReadWrite | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly},
  hasThrottle{this, "has_throttle", false, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly},
  throttleName{this, "throttle_name", "", PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly},
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

  Attributes::addObjectEditor(hasThrottle, false);
  m_interfaceItems.add(hasThrottle);

  Attributes::addObjectEditor(throttleName, false);
  m_interfaceItems.add(throttleName);

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
  for(const auto& item : *vehicles)
  {
    item->vehicle->trains.removeInternal(self);
  }
  m_world.trains->removeObject(self);
  IdObject::destroying();
}

void Train::loaded()
{
  IdObject::loaded();

  Attributes::setEnabled(lob, overrideLength);
  Attributes::setEnabled(weight, overrideWeight);

  if(active)
  {
    // Try activating all vehicles
    bool allVehiclesFree = true;

    for(const auto& item : *vehicles)
    {
      if(item->vehicle->activeTrain.value())
      {
        allVehiclesFree = false;
        break;
      }
    }

    if(allVehiclesFree && !vehicles->empty())
    {
      for(const auto& item : *vehicles)
      {
        item->vehicle->activeTrain.setValueInternal(shared_ptr<Train>());
      }
    }
    else
    {
      // Maybe there was a file corruption, deactivate Train
      active.setValueInternal(false);
    }
  }

  // Update vehicle list state
  vehiclesChanged();

  if(active)
  {
    if(!emergencyStop)
    {
      // If one vehicle is in Emergency stop, sync all Train
      bool atLeastOneEmergencyStop = false;

      for(const auto& vehicle : m_poweredVehicles)
      {
        if(vehicle->decoder && vehicle->decoder->emergencyStop)
        {
          atLeastOneEmergencyStop = true;
          break;
        }
      }
      if(atLeastOneEmergencyStop)
        emergencyStop.setValueInternal(atLeastOneEmergencyStop);
    }

    // Manually sync emergency stop
    for(const auto& vehicle : m_poweredVehicles)
    {
      vehicle->setEmergencyStop(emergencyStop);
    }
  }
}

void Train::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);

  updateEnabled();
}

void Train::setSpeed(const SpeedPoint& speedPoint)
{
  lastSetSpeedPoint = speedPoint;

  if(m_speedTable)
  {
    const auto& entry = m_speedTable->getEntryAt(speedPoint.tableIdx);

    uint32_t locoIdx = 0;
    for(const auto& vehicle : m_poweredVehicles)
    {
      if(vehicle == m_delayedApplyLoco)
      {
        locoIdx++;
        continue; // This loco will be set later
      }

      // TODO: support arbitrary speed step max
      driveLocomotive(vehicle,
                      entry.getStepForLoco(locoIdx));

      locoIdx++;
    }
  }

  // Update real speed property
  const double realSpeedMS = lastSetSpeedPoint.speedMetersPerSecond * m_world.scaleRatio.value();
  speed.setValueInternal(convertUnit(realSpeedMS,
                                     SpeedUnit::MeterPerSecond,
                                     speed.unit()));

  updateEnabled();
}

void Train::setThrottleSpeed(const SpeedPoint& targetSpeed)
{
  const SpeedPoint oldThrottle = throttleSpeedPoint;
  throttleSpeedPoint = targetSpeed;

  if(targetSpeed.tableIdx == TrainSpeedTable::NULL_TABLE_ENTRY)
  {
    if(!almostZero(targetSpeed.speedMetersPerSecond))
    {
      auto match = m_speedTable->getClosestMatch(targetSpeed.speedMetersPerSecond);
      throttleSpeedPoint.speedMetersPerSecond = match.tableEntry.avgSpeed;
      throttleSpeedPoint.tableIdx = match.tableIdx;

      // Do smart rounding
      if(targetSpeed.speedMetersPerSecond > oldThrottle.speedMetersPerSecond)
      {
          // User wants to go faster
          if(throttleSpeedPoint.tableIdx == oldThrottle.tableIdx &&
             throttleSpeedPoint.tableIdx < maxSpeedPoint.tableIdx)
          {
              // But we got rounded back to old value
              // So give it a +1
              throttleSpeedPoint.tableIdx++;
              const auto& next = m_speedTable->getEntryAt(throttleSpeedPoint.tableIdx);
              throttleSpeedPoint.speedMetersPerSecond = next.avgSpeed;
          }
      }
      else if(targetSpeed.speedMetersPerSecond < oldThrottle.speedMetersPerSecond)
      {
          // User wants to go slower
          if(throttleSpeedPoint.tableIdx == oldThrottle.tableIdx &&
             throttleSpeedPoint.tableIdx > 0)
          {
              // But we got rounded back to old value
              // So give it a -1
              throttleSpeedPoint.tableIdx--;
              const auto& prev = m_speedTable->getEntryAt(throttleSpeedPoint.tableIdx);
              throttleSpeedPoint.speedMetersPerSecond = prev.avgSpeed;
          }
      }
    }
  }

  if(throttleSpeedPoint.tableIdx > maxSpeedPoint.tableIdx)
    throttleSpeedPoint = maxSpeedPoint;

  // Update real speed property
  const double realSpeedMS = throttleSpeedPoint.speedMetersPerSecond * m_world.scaleRatio.value();
  throttleSpeed.setValueInternal(convertUnit(realSpeedMS,
                                             SpeedUnit::MeterPerSecond,
                                             throttleSpeed.unit()));

  auto elapsed = std::chrono::steady_clock::now() - m_speedTimerStart;
  auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);

  if(throttleSpeedPoint.tableIdx > lastSetSpeedPoint.tableIdx)
  {
    if(m_speedState == SpeedState::Accelerating)
    {
      // Keep accelerating
    }
    else
    {
      double currentSpeed = lastSetSpeedPoint.speedMetersPerSecond;
      uint8_t nextTableIdx = lastSetSpeedPoint.tableIdx + 1;

      if(m_speedState == SpeedState::Braking)
      {
        // We start from under last set speed
        double deltaSpeed = (m_brakingRate / m_world.scaleRatio) * double(millis.count()) / 1000.0;
        currentSpeed += deltaSpeed; // Negative delta
        nextTableIdx--;
      }

      scheduleAccelerationFrom(currentSpeed,
                               nextTableIdx,
                               SpeedState::Accelerating);
    }
  }
  else if(throttleSpeedPoint.tableIdx < lastSetSpeedPoint.tableIdx)
  {
    if(m_speedState == SpeedState::Braking)
    {
      // Keep braking
    }
    else
    {
      double currentSpeed = lastSetSpeedPoint.speedMetersPerSecond;
      uint8_t prevTableIdx = lastSetSpeedPoint.tableIdx - 1;

      if(m_speedState == SpeedState::Accelerating)
      {
        // We start from above last set speed
        double deltaSpeed = (m_accelerationRate / m_world.scaleRatio) * double(millis.count()) / 1000.0;
        currentSpeed += deltaSpeed;
        prevTableIdx++;
      }

      scheduleAccelerationFrom(currentSpeed,
                               prevTableIdx,
                               SpeedState::Braking);
    }
  }
  else if(m_speedState != SpeedState::Idle)
  {
    // Cancel current acceleration
    m_speedState = SpeedState::Idle;
    m_speedTimer.cancel();

    // Force setting speed
    setSpeed(throttleSpeedPoint);
  }
}

void Train::scheduleAccelerationFrom(double currentSpeed, uint8_t newTableIdx, SpeedState state)
{
  // Calculate seconds to previous table index
  double newSpeed = m_speedTable->getEntryAt(newTableIdx).avgSpeed;

  const double deltaSpeed = newSpeed - currentSpeed;
  double accelRate = m_accelerationRate;
  if(state == SpeedState::Braking)
  {
    accelRate = m_brakingRate;
  }
  accelRate /= m_world.scaleRatio;

  const double deltaSeconds = deltaSpeed / accelRate;
  const uint32_t millis = std::ceil(deltaSeconds * 1000.0);

  m_speedState = state;

  m_speedTimerStart = std::chrono::steady_clock::now();
  m_speedTimer.expires_after(std::chrono::milliseconds(millis));
  m_speedTimer.async_wait(
    [this](const boost::system::error_code& ec)
    {
      if(!ec)
        updateSpeed();
    });
}

void Train::updateSpeed()
{
  if(m_speedState == SpeedState::Idle)
    return;

  if(m_speedState == SpeedState::Accelerating && !active)
    return;

  // TODO: needed?
  m_speedTimer.cancel();

  uint8_t newTableIdx = lastSetSpeedPoint.tableIdx;
  if(m_speedState == SpeedState::Accelerating)
  {
    newTableIdx++;
  }
  else if(m_speedState == SpeedState::Braking)
  {
    newTableIdx--;
  }
  else
    assert(false);

  const auto& entry = m_speedTable->getEntryAt(newTableIdx);
  SpeedPoint newSpeed;
  newSpeed.speedMetersPerSecond = entry.avgSpeed;
  newSpeed.tableIdx = newTableIdx;

  setSpeed(newSpeed);

  if(throttleSpeedPoint.tableIdx > lastSetSpeedPoint.tableIdx)
  {
    // Keep accelerating
    double currentSpeed = lastSetSpeedPoint.speedMetersPerSecond;
    uint8_t nextTableIdx = lastSetSpeedPoint.tableIdx + 1;

    scheduleAccelerationFrom(currentSpeed,
                             nextTableIdx,
                             SpeedState::Accelerating);
  }
  else if(throttleSpeedPoint.tableIdx < lastSetSpeedPoint.tableIdx)
  {
    // Keep braking
    double currentSpeed = lastSetSpeedPoint.speedMetersPerSecond;
    uint8_t prevTableIdx = lastSetSpeedPoint.tableIdx - 1;

    scheduleAccelerationFrom(currentSpeed,
                             prevTableIdx,
                             SpeedState::Braking);
  }
  else
  {
    // We reached target speed
    m_speedState = SpeedState::Idle;
  }

  const bool currentValue = isStopped;
  isStopped.setValueInternal(m_speedState == SpeedState::Idle &&
                             lastSetSpeedPoint.tableIdx == TrainSpeedTable::NULL_TABLE_ENTRY &&
                             throttleSpeedPoint.tableIdx == TrainSpeedTable::NULL_TABLE_ENTRY);
  if(currentValue != isStopped)
    updateEnabled();
}

void Train::updateSpeedTable()
{
  // Recalculate speed table
  m_speedTableNeedsRecalculation = false;

  if(m_poweredVehicles.empty())
  {
    m_speedTable.reset();
    return;
  }

  std::vector<VehicleSpeedCurve> speedCurves;
  speedCurves.reserve(m_poweredVehicles.size());
  for(const auto& vehicle : m_poweredVehicles)
  {
    if(!vehicle->m_speedCurve)
    {
      // All vehicles must have a speed curve loaded
      m_speedTable.reset();
      return;
    }

    speedCurves.push_back(*vehicle->m_speedCurve);
  }

  if(!m_speedTable)
      m_speedTable.reset(new TrainSpeedTable);

  *m_speedTable = TrainSpeedTable::buildTable(speedCurves);

  // Update max speed TODO: update also speedMax property
  const auto& lastEntry = m_speedTable->getEntryAt(m_speedTable->count() - 1);
  maxSpeedPoint.speedMetersPerSecond = lastEntry.avgSpeed;
  maxSpeedPoint.tableIdx = m_speedTable->count() - 1;

  if(m_speedTable->count() == 1)
  {
    // No match was found, only null entry
    m_speedTable.reset();
  }
}

void Train::scheduleSpeedTableUpdate()
{
  if(!active)
  {
    // Delay recalculation to when active
    m_speedTable.reset();
    m_speedTableNeedsRecalculation = true;
    return;
  }

  updateSpeedTable();
}

void Train::startDelayedSpeedApply(const std::shared_ptr<PoweredRailVehicle> &vehicle)
{
  using namespace std::chrono_literals;
  if(m_delayedApplyLoco && m_delayedApplyLoco != vehicle)
  {
    // Source loco changed, apply now
    applyDelayedSpeed();
  }

  m_delayedSpeedApplyTimer.cancel();

  m_delayedApplyLoco = vehicle;
  m_delayedSpeedApplyTimer.expires_after(700ms);
  m_delayedSpeedApplyTimer.async_wait(
    [this](const boost::system::error_code& ec)
    {
      if(!ec)
        applyDelayedSpeed();
    });
}

void Train::stopDelayedSpeedApply()
{
  m_delayedSpeedApplyTimer.cancel();
  m_delayedApplyLoco.reset();
}

void Train::applyDelayedSpeed()
{
    auto vehicle = m_delayedApplyLoco;

    stopDelayedSpeedApply();

    if(!active || !vehicle || !vehicle->decoder)
      return;

    auto it = std::find(m_poweredVehicles.begin(),
                        m_poweredVehicles.end(),
                        vehicle);
    if(it == m_poweredVehicles.end())
      return;

    const uint32_t locoIdx = std::distance(m_poweredVehicles.begin(), it);

    const auto& entry = m_speedTable->getEntryAt(lastSetSpeedPoint.tableIdx);
    uint8_t step = entry.getStepForLoco(locoIdx);

    driveLocomotive(vehicle, step);
}

void Train::driveLocomotive(const std::shared_ptr<PoweredRailVehicle> &vehicle,
                            uint8_t step)
{
  // TODO: support arbitrary speed step max
  float throttle = Decoder::speedStepToThrottle(step, uint8_t(126));
  vehicle->lastTrainSpeedStep = throttle;
  if(vehicle->decoder)
    vehicle->decoder->throttle = throttle;
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
  for(const auto& item : *vehicles)
    mm += item->vehicle->lob.getValue(LengthUnit::MilliMeter);
  lob.setValueInternal(convertUnit(mm, LengthUnit::MilliMeter, lob.unit()));
}

void Train::updateWeight()
{
  if(overrideWeight)
    return;

  double ton = 0;
  for(const auto& item : *vehicles)
    ton += item->vehicle->totalWeight.getValue(WeightUnit::Ton);
  weight.setValueInternal(convertUnit(ton, WeightUnit::Ton, weight.unit()));
}

void Train::updatePowered()
{
  m_poweredVehicles.clear();
  for(const auto& item : *vehicles)
    if(auto poweredVehicle = std::dynamic_pointer_cast<PoweredRailVehicle>(item->vehicle.value()))
      m_poweredVehicles.emplace_back(poweredVehicle);
  powered.setValueInternal(!m_poweredVehicles.empty());

  scheduleSpeedTableUpdate();
}

void Train::updateSpeedMax()
{
  if(!vehicles->empty() && powered)
  {
    const auto itEnd = vehicles->end();
    auto it = vehicles->begin();
    double kmph = (*it)->vehicle->speedMax.getValue(SpeedUnit::KiloMeterPerHour);
    for(; it != itEnd; ++it)
    {
      const double v = (*it)->vehicle->speedMax.getValue(SpeedUnit::KiloMeterPerHour);
      if((v > 0 || isPowered(*(*it)->vehicle)) && v < kmph)
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

  for(const auto& item : *vehicles)
  {
    Attributes::setEnabled(item->vehicle, stopped);
    Attributes::setEnabled(item->invertDirection, stopped);
  }
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
    for(const auto& item : *vehicles)
    {
      assert(item->vehicle->activeTrain.value() != self);
      if(item->vehicle->activeTrain.value())
      {
        return false; //Not free
      }

      if(auto decoder = item->vehicle->decoder.value(); decoder && !almostZero(decoder->throttle.value()))
      {
        return false; //Already running
      }
    }

    //Now really activate
    //Register this train as activeTrain
    for(const auto& item : *vehicles)
    {
      item->vehicle->activeTrain.setValueInternal(self);
    }

    //Sync Emergency Stop state
    const bool stopValue = emergencyStop;
    for(const auto& vehicle : m_poweredVehicles)
      vehicle->setEmergencyStop(stopValue);

    if(!m_speedTable || m_speedTableNeedsRecalculation)
      updateSpeedTable();
  }
  else
  {
    //To deactivate a Train it must be stopped first
    if(!isStopped)
      return false;

    stopDelayedSpeedApply();

    //Deactivate all vehicles
    for(const auto& item : *vehicles)
    {
      assert(item->vehicle->activeTrain.value() == self);
      item->vehicle->activeTrain.setValueInternal(nullptr);
    }
  }

  return true;
}

std::error_code Train::acquire(Throttle& throttle, bool steal)
{
  if(m_throttle)
  {
    if(!steal)
    {
      return make_error_code(ErrorCode::AlreadyAcquired);
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
      return make_error_code(ErrorCode::CanNotActivateTrain);
    }
  }
  assert(!m_throttle);
  m_throttle = throttle.shared_ptr<Throttle>();
  hasThrottle.setValueInternal(true);
  throttleName.setValueInternal(m_throttle->name);
  return {};
}

std::error_code Train::release(Throttle& throttle)
{
  if(m_throttle.get() != &throttle)
  {
    return make_error_code(ErrorCode::InvalidThrottle);
  }
  m_throttle.reset();
  hasThrottle.setValueInternal(false);
  throttleName.setValueInternal("");
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
    return make_error_code(ErrorCode::InvalidThrottle);
  }
  assert(active);

  value = std::clamp(value, Attributes::getMin(speed), Attributes::getMax(speed));

  // Get speed point
  auto match = m_speedTable->getClosestMatch(value);

  SpeedPoint speedPoint;
  speedPoint.speedMetersPerSecond = match.tableEntry.avgSpeed;
  speedPoint.tableIdx = match.tableIdx;

  // Cancel current acceleration
  m_speedState = SpeedState::Idle;
  m_speedTimer.cancel();

  // Force setting speed
  setSpeed(throttleSpeedPoint);

  // Update real speed property
  const double realSpeedMS = throttleSpeedPoint.speedMetersPerSecond * m_world.scaleRatio.value();
  throttleSpeed.setValueInternal(convertUnit(realSpeedMS,
                                             SpeedUnit::MeterPerSecond,
                                             throttleSpeed.unit()));

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
    return make_error_code(ErrorCode::InvalidThrottle);
  }
  assert(active);
  throttleSpeed.setValue(std::clamp(value, Attributes::getMin(throttleSpeed), Attributes::getMax(throttleSpeed)));
  return {};
}

std::error_code Train::setDirection(Throttle& throttle, Direction value)
{
  if(m_throttle.get() != &throttle)
  {
    return make_error_code(ErrorCode::InvalidThrottle);
  }
  if(direction != value)
  {
    if(!isStopped)
    {
      return make_error_code(ErrorCode::TrainMustBeStoppedToChangeDirection);
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

void Train::propagateDirection(Direction newDirection)
{
  if(!active)
    return;

  const Direction oppositeDirection = newDirection == Direction::Forward ? Direction::Reverse : Direction::Forward;
  for(const auto& item : *vehicles)
  {
    Direction dir = newDirection;
    if(item->invertDirection)
      dir = oppositeDirection;

    auto poweredVehicle = std::dynamic_pointer_cast<PoweredRailVehicle>(item->vehicle.value());
    if(poweredVehicle)
    {
      poweredVehicle->lastTrainSetDirection = dir;
      poweredVehicle->setDirection(dir);
    }
  }
}

void Train::handleDecoderDirection(const std::shared_ptr<PoweredRailVehicle>& vehicle, Direction newDirection)
{
  //! \todo assert vehicle contained in train?
  if(!active || newDirection == Direction::Unknown)
    return;

  //Check if vehicle is inverted
  bool isInverted = false;
  for(const auto& item : *vehicles)
  {
    if(item->vehicle.value() == vehicle)
    {
      if(item->invertDirection)
        isInverted = true;
      break;
    }
  }

  if(isInverted)
    newDirection = newDirection == Direction::Forward ? Direction::Reverse : Direction::Forward;

  if(direction == newDirection)
    return; //No change

  direction = newDirection;
}

void Train::handleDecoderThrottle(const std::shared_ptr<PoweredRailVehicle> &vehicle, float newThrottle)
{
  if(!active || !powered || !vehicle->decoder)
    return;

  // TODO: handle case when no speed table is present
  // Do it legacy way by setting all locomotives to same throttle?

  auto it = std::find(m_poweredVehicles.begin(),
                      m_poweredVehicles.end(),
                      vehicle);
  if(it == m_poweredVehicles.end())
    return;

  const uint32_t locoIdx = std::distance(m_poweredVehicles.begin(), it);

  uint8_t step = Decoder::throttleToSpeedStep(newThrottle, uint8_t(126));
  uint8_t oldStep = Decoder::throttleToSpeedStep(vehicle->lastTrainSpeedStep, uint8_t(126));

  const TrainSpeedTable::Entry& maxSpeedEntry = m_speedTable->getEntryAt(maxSpeedPoint.tableIdx);
  uint8_t maxLocoStep = maxSpeedEntry.getStepForLoco(locoIdx);
  if(step > maxLocoStep)
  {
    // Locomotive exceeded Train max speed, revert immediately

    // TODO: is it better to revert to lastSetSpeedPoint?
    driveLocomotive(vehicle, oldStep);

    // Set all locomotives to max train speed
    setThrottleSpeed(maxSpeedPoint);

    return;
  }

  auto match = m_speedTable->getClosestMatch(locoIdx, step);

  SpeedPoint speedPoint;
  speedPoint.speedMetersPerSecond = match.tableEntry.avgSpeed;
  speedPoint.tableIdx = match.tableIdx;

  bool needsDelay = false;
  const uint8_t newStep = match.tableEntry.getStepForLoco(locoIdx);

  if(newStep != step)
  {
    // Requested step was adjusted
    // This could prevent setting speed going up one by one

    bool invertsTrend = (oldStep < step && step > newStep) ||
                        (oldStep > step && step < newStep);

    if(lastSetSpeedPoint.tableIdx == match.tableIdx)
    {
      // We would get rounded back to previous state
      if(std::abs(oldStep - step) > 3)
      {
        // It takes more than +3 steps to reach next tableIdx
        // We use 3 as threshold to ignore spurious rotary knob changes
        // We help user reach next tableIdx before delay timeout triggers
        // In fact it's hard to go more than 4 steps up before delaied
        // step apply triggers.
        uint8_t newTableIdx = lastSetSpeedPoint.tableIdx;
        if(step > oldStep)
          newTableIdx++;
        else
          newTableIdx--;

        // Update speed point
        speedPoint.tableIdx = newTableIdx;
        speedPoint.speedMetersPerSecond = m_speedTable->getEntryAt(newTableIdx).avgSpeed;
      }
      else if(invertsTrend)
      {
        // We would override user requested speed
        // with original speed, give user some time
        // to set higher/lower step
        needsDelay = true;
      }
    }
    else if(abs(lastSetSpeedPoint.tableIdx - match.tableIdx) == 1)
    {
      // User managed to get to next/prev tableIdx but still got
      // adjusted. Give some extra time if trend got inverted
      if(invertsTrend)
      {
        needsDelay = true;
      }
    }
  }

  if(needsDelay)
  {
    // Keep step set by user
    newThrottle = Decoder::speedStepToThrottle(newStep, uint8_t(126));
    vehicle->lastTrainSpeedStep = newThrottle;

    startDelayedSpeedApply(vehicle);
  }
  else
  {
    // We will accelerate/brake up to requested step
    // But first reset to last set step
    // This is needed so all locomotives start accelerating/braking
    // At same speed, otherwise this locomotive would accelerate faster
    // or brake faster because it's the first one to receive commands
    // and would be out of sync with the others.
    const auto& entry = m_speedTable->getEntryAt(lastSetSpeedPoint.tableIdx);
    step = entry.getStepForLoco(locoIdx);
    driveLocomotive(vehicle, step);

    // Apply previous delay if present
    applyDelayedSpeed();
  }

  setThrottleSpeed(speedPoint);
}
