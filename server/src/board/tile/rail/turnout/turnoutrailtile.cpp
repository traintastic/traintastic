/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2020-2026 Reinder Feenstra
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

#include "turnoutrailtile.hpp"
#include "../../../../core/objectproperty.tpp"
#include "../../../../core/attributes.hpp"
#include "../../../../core/method.tpp"
#include "../../../../world/world.hpp"
#include "../../../../utils/displayname.hpp"
#include "../../../map/blockpath.hpp"
#include "../blockrailtile.hpp"
#include "../../../../train/trainblockstatus.hpp"
#include "../../../../train/train.hpp"
#include "../../../../log/log.hpp"

TurnoutRailTile::TurnoutRailTile(World& world, std::string_view _id, TileId tileId_, size_t connectors) :
  RailTile(world, _id, tileId_),
  m_node{*this, connectors},
  name{this, "name", std::string(_id), PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly},
  position{this, "position", TurnoutPosition::Unknown, PropertyFlags::ReadWrite | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly},
  outputMap{this, "output_map", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject | PropertyFlags::NoScript},
  feedbackMap{this, "feedback_map", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject | PropertyFlags::NoScript},
  setPosition{*this, "set_position", MethodFlags::ScriptCallable, [this](TurnoutPosition value)
    {
      TurnoutPosition reservedPosition = getReservedPosition();
      if(reservedPosition != TurnoutPosition::Unknown && reservedPosition != value)
        return false; // Turnout is locked by reservation path
      return doSetPosition(value);
    }}
{
  assert(isRailTurnout(tileId_));

  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);

  Attributes::addDisplayName(position, DisplayName::BoardTile::Turnout::position);
  Attributes::addObjectEditor(position, false);
  // position is added by sub class

  Attributes::addDisplayName(outputMap, DisplayName::BoardTile::outputMap);
  m_interfaceItems.add(outputMap);

  Attributes::addDisplayName(feedbackMap, DisplayName::BoardTile::feedbackMap);
  m_interfaceItems.add(feedbackMap);

  Attributes::addObjectEditor(setPosition, false);
  // setPosition is added by sub class
}

bool TurnoutRailTile::reserve(const std::shared_ptr<BlockPath> &blockPath, TurnoutPosition turnoutPosition, bool dryRun)
{
  if(!isValidPosition(turnoutPosition))
  {
    return false;
  }

  const TurnoutPosition reservedPos = getReservedPosition();
  if(reservedPos != TurnoutPosition::Unknown && reservedPos != turnoutPosition)
  {
    // TODO: what if 2 path reserve same turnout for same position?
    // Upon release one path it will make turnout free while it's still reserved by second path

    // Turnout is already reserved for another position
    return false;
  }

  if(!dryRun)
  {
    if(!doSetPosition(turnoutPosition)) /*[[unlikely]]*/
    {
      return false;
    }

    m_reservedPath = blockPath;

    RailTile::setReservedState(static_cast<uint8_t>(turnoutPosition));
  }
  return true;
}

bool TurnoutRailTile::release(bool dryRun)
{
  //! \todo check occupancy sensor, once supported

  if(!dryRun)
  {
    m_reservedPath.reset();

    RailTile::release();
  }
  return true;
}

void TurnoutRailTile::destroying()
{
  outputMap->parentObject.setValueInternal(nullptr);
  feedbackMap->parentObject.setValueInternal(nullptr);
  RailTile::destroying();
}

void TurnoutRailTile::addToWorld()
{
  feedbackMap->parentObject.setValueInternal(shared_from_this());
  outputMap->parentObject.setValueInternal(shared_from_this());
  RailTile::addToWorld();
}

TurnoutPosition TurnoutRailTile::getReservedPosition() const
{
  return static_cast<TurnoutPosition>(RailTile::reservedState());
}

void TurnoutRailTile::worldEvent(WorldState state, WorldEvent event)
{
  RailTile::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(name, editable);
}

bool TurnoutRailTile::isValidPosition(TurnoutPosition value)
{
  const auto* values = setPosition.tryGetValuesAttribute(AttributeName::Values);
  assert(values);
  return values->contains(static_cast<int64_t>(value));
}

bool TurnoutRailTile::doSetPosition(TurnoutPosition value, bool skipAction)
{
  if(!isValidPosition(value))
  {
    return false;
  }
  if(!skipAction)
    (*outputMap)[value]->execute();
  if(!hasFeedback())
  {
    position.setValueInternal(value);
    positionChanged(*this, value);
  }
  return true;
}

bool TurnoutRailTile::hasFeedback() const
{
  return feedbackMap->interface;
}

void TurnoutRailTile::connectOutputMap()
{
  // FIXME: use similar callback as FeedbackMap
  outputMap->onOutputStateMatchFound.connect([this](TurnoutPosition pos)
    {
      updatePosition(Source::OutputStateMatch, pos);
    });
}

void TurnoutRailTile::updatePosition(Source source, TurnoutPosition pos)
{
  if(hasFeedback() && source != Source::FeedbackMatch)
  {
    return; // Ignore
  }
  if(pos == position)
  {
    return; // No change
  }

  position.setValueInternal(pos);
  positionChanged(*this, pos);

  TurnoutPosition reservedPosition = getReservedPosition();
  if(reservedPosition == TurnoutPosition::Unknown || reservedPosition == position.value())
    return; // Not locked

  // If turnout is inside a reserved path, force it to reserved position
  // This corrects accidental modifications of position done
  // by the user with an handset or command station.
  Log::log(id, LogMessage::W3003_LOCKED_TURNOUT_CHANGED);

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
      doSetPosition(reservedPosition, false);

      Log::log(id, LogMessage::N3003_TURNOUT_RESET_TO_RESERVED_POSITION);
      return;
    }
  }

  // We reached maximum retry count
  // We cannot lock this turnout. Take action.
  switch (m_world.extOutputChangeAction.value())
  {
    default:
    case ExternalOutputChangeAction::DoNothing:
      break; // Do nothing

    case ExternalOutputChangeAction::EmergencyStopTrain:
    {
      if(auto blockPath = m_reservedPath.lock())
      {
        std::vector<std::shared_ptr<Train>> alreadyStoppedTrains;

        for(auto it : blockPath->fromBlock().trains)
        {
          it->train.value()->emergencyStop.setValue(true);
          alreadyStoppedTrains.push_back(it->train.value());
          Log::log(it->train->id, LogMessage::E3003_TRAIN_STOPPED_ON_TURNOUT_X_CHANGED, id.value());
        }

        auto toBlock = blockPath->toBlock();
        if(toBlock)
        {
          for(auto it : blockPath->toBlock()->trains)
          {
            if(std::find(alreadyStoppedTrains.cbegin(), alreadyStoppedTrains.cend(), it->train.value()) != alreadyStoppedTrains.cend())
              continue; // Do not stop train twice

            it->train.value()->emergencyStop.setValue(true);
            Log::log(it->train->id, LogMessage::E3003_TRAIN_STOPPED_ON_TURNOUT_X_CHANGED, id.value());
          }
        }
      }
      break;
    }
    case ExternalOutputChangeAction::EmergencyStopWorld:
    {
      m_world.stop();
      Log::log(m_world, LogMessage::E3007_WORLD_STOPPED_ON_TURNOUT_X_CHANGED, id.value());
      break;
    }
    case ExternalOutputChangeAction::PowerOffWorld:
    {
      m_world.powerOff();
      Log::log(m_world, LogMessage::E3009_WORLD_POWER_OFF_ON_TURNOUT_X_CHANGED, id.value());
      break;
    }
  }
}
