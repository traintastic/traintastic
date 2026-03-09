/**
 * server/src/board/tile/rail/signal/signalrailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2025 Reinder Feenstra
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

#include "signalrailtile.hpp"
#include "../../../map/abstractsignalpath.hpp"
#include "../../../map/blockpath.hpp"
#include "../../../../core/attributes.hpp"
#include "../../../../core/method.tpp"
#include "../../../../core/objectproperty.tpp"
#include "../../../../world/getworld.hpp"
#include "../../../../utils/category.hpp"
#include "../../../../utils/displayname.hpp"
#include "../blockrailtile.hpp"
#include "../../../../train/trainblockstatus.hpp"
#include "../../../../train/train.hpp"
#include "../../../../log/log.hpp"

std::optional<OutputActionValue> SignalRailTile::getDefaultActionValue(SignalAspect signalAspect, OutputType outputType, size_t outputIndex)
{
  // FIXME: implement more defaults
  switch(outputType)
  {
    case OutputType::Pair:
      if(signalAspect == SignalAspect::Stop && outputIndex == 0)
      {
        return PairOutputAction::First;
      }
      else if(signalAspect == SignalAspect::ProceedReducedSpeed && outputIndex == 1)
      {
        return PairOutputAction::Second;
      }
      else if(signalAspect == SignalAspect::Proceed && outputIndex == 0)
      {
        return PairOutputAction::Second;
      }
      break;

    case OutputType::Aspect:
      if(outputIndex == 0)
      {
        // There is no official/defacto standard yet, until there is one use:
        // https://www.z21.eu/de/produkte/z21-signal-decoder/signaltypen
        // also used by YaMoRC YD8116.

        if(signalAspect == SignalAspect::Stop)
        {
          return static_cast<int16_t>(0);
        }
        if(signalAspect == SignalAspect::ProceedReducedSpeed)
        {
          return static_cast<int16_t>(1);
        }
        if(signalAspect == SignalAspect::Proceed)
        {
          return static_cast<int16_t>(16);
        }
      }
      break;

    default:
      break;
  }
  return {};
}

SignalRailTile::SignalRailTile(World& world, std::string_view _id, TileId tileId_) :
  StraightRailTile(world, _id, tileId_),
  m_node{*this, 2},
  m_retryCount(0),
  name{this, "name", std::string(_id), PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly},
  requireReservation{this, "require_reservation", AutoYesNo::Auto, PropertyFlags::ReadWrite | PropertyFlags::Store},
  aspect{this, "aspect", SignalAspect::Unknown, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly},
  outputMap{this, "output_map", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject | PropertyFlags::NoScript},
  setAspect{*this, "set_aspect", MethodFlags::ScriptCallable, [this](SignalAspect value) { return doSetAspect(value); }}
  , onAspectChanged{*this, "on_aspect_changed", EventFlags::Scriptable}
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);

  Attributes::addCategory(requireReservation, Category::options);
  Attributes::addDisplayName(requireReservation, "board_tile.rail.signal:require_reservation");
  Attributes::addEnabled(requireReservation, editable);
  Attributes::addValues(requireReservation, autoYesNoValues);
  m_interfaceItems.add(requireReservation);

  Attributes::addObjectEditor(aspect, false);
  // aspect is added by sub class

  Attributes::addDisplayName(outputMap, DisplayName::BoardTile::outputMap);
  m_interfaceItems.add(outputMap);

  Attributes::addObjectEditor(setAspect, false);
  // setAspect is added by sub class

  m_interfaceItems.add(onAspectChanged);
}

SignalRailTile::~SignalRailTile() = default; // default here, so we can use a forward declaration of SignalPath in the header.

bool SignalRailTile::hasReservedPath() const noexcept
{
  return !m_blockPath.expired();
}

std::shared_ptr<BlockPath> SignalRailTile::reservedPath() const noexcept
{
  return m_blockPath.lock();
}

bool SignalRailTile::reserve(const std::shared_ptr<BlockPath>& blockPath, bool dryRun)
{
  // no conditions yet...

  if(!dryRun)
  {
    m_blockPath = blockPath;
    RailTile::reserve();
    evaluate();
  }
  return true;
}

void SignalRailTile::destroying()
{
  outputMap->parentObject.setValueInternal(nullptr);
  StraightRailTile::destroying();
}

void SignalRailTile::addToWorld()
{
  outputMap->parentObject.setValueInternal(shared_from_this());
  StraightRailTile::addToWorld();
}

void SignalRailTile::worldEvent(WorldState state, WorldEvent event)
{
  StraightRailTile::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);
  const bool editableAndStopped = editable && !contains(state, WorldState::Run);

  Attributes::setEnabled(name, editable);
  Attributes::setEnabled(requireReservation, editableAndStopped);

  if(event == WorldEvent::Run)
  {
    evaluate();
  }
}

void SignalRailTile::boardModified()
{
  if(m_signalPath)
  {
    m_signalPath->evaluate();
  }
  StraightRailTile::boardModified();
}

bool SignalRailTile::doSetAspect(SignalAspect value, bool skipAction)
{
  const auto* values = setAspect.tryGetValuesAttribute(AttributeName::Values);
  assert(values);
  if(!values->contains(static_cast<int64_t>(value)))
    return false;
  if(aspect != value)
  {
    if(!skipAction)
      (*outputMap)[value]->execute();
    aspect.setValueInternal(value);
    aspectChanged(*this, value);
    fireEvent(onAspectChanged, shared_ptr<SignalRailTile>(), value);
  }
  return true;
}

void SignalRailTile::evaluate()
{
  if(m_signalPath) /*[[likely]]*/
  {
    m_signalPath->evaluate();
  }
  else
  {
    setAspect(SignalAspect::Stop);
  }
}

void SignalRailTile::connectOutputMap()
{
    outputMap->onOutputStateMatchFound.connect([this](SignalAspect value)
      {
        bool changed = (value != aspect);
        if(!doSetAspect(value, true) || !changed)
          return; // No change

        if(!m_signalPath || !hasReservedPath())
          return; // Not locked

        // If we are in a signal path, re-evaluate our aspect
        // This corrects accidental modifications of aspect done
        // by the user with an handset or command station.
        Log::log(id, LogMessage::W3004_LOCKED_SIGNAL_CHANGED);

        if(m_world.correctOutputPosWhenLocked)
        {
          auto now = std::chrono::steady_clock::now();
          if((now - m_lastRetryStart) >= RETRY_DURATION)
          {
            // Reset retry count
            m_lastRetryStart = now;
            m_retryCount = 0;
          }

          if(m_retryCount < MAX_RETRYCOUNT)
          {
            // Try to reset output to reseved state
            m_retryCount++;
            evaluate();

            Log::log(id, LogMessage::N3004_SIGNAL_RESET_TO_RESERVED_ASPECT);
            return;
          }
        }

        // We reached maximum retry count
        // We cannot lock this signal. Take action.
        switch (m_world.extOutputChangeAction.value())
        {
        default:
        case ExternalOutputChangeAction::DoNothing:
        {
          // Do nothing
          break;
        }
        case ExternalOutputChangeAction::EmergencyStopTrain:
        {
          if(auto blockPath = reservedPath())
          {
            std::vector<std::shared_ptr<Train>> alreadyStoppedTrains;

            for(auto it : blockPath->fromBlock().trains)
            {
              it->train.value()->emergencyStop.setValue(true);
              alreadyStoppedTrains.push_back(it->train.value());
              Log::log(it->train->id, LogMessage::E3004_TRAIN_STOPPED_ON_SIGNAL_X_CHANGED, id.value());
            }

            auto toBlock = blockPath->toBlock();
            if(toBlock)
            {
              for(auto it : blockPath->toBlock()->trains)
              {
                if(std::find(alreadyStoppedTrains.cbegin(), alreadyStoppedTrains.cend(), it->train.value()) != alreadyStoppedTrains.cend())
                  continue; // Do not stop train twice

                it->train.value()->emergencyStop.setValue(true);
                Log::log(it->train->id, LogMessage::E3004_TRAIN_STOPPED_ON_SIGNAL_X_CHANGED, id.value());
              }
            }
          }
          break;
        }
        case ExternalOutputChangeAction::EmergencyStopWorld:
        {
          m_world.stop();
          Log::log(m_world, LogMessage::E3008_WORLD_STOPPED_ON_SIGNAL_X_CHANGED, id.value());
          break;
        }
        case ExternalOutputChangeAction::PowerOffWorld:
        {
          m_world.powerOff();
          Log::log(m_world, LogMessage::E3010_WORLD_POWER_OFF_ON_SIGNAL_X_CHANGED, id.value());
          break;
        }
        }
      });

    //TODO: disconnect somewhere?
}
