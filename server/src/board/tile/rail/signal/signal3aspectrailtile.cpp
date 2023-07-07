/**
 * server/src/board/tile/rail/signal/signal3aspectrailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2022 Reinder Feenstra
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

#include "signal3aspectrailtile.hpp"
#include "../../../map/signalpath.hpp"
#include "../../../../core/attributes.hpp"
#include "../../../../core/method.tpp"
#include "../../../../core/objectproperty.tpp"

static const std::array<SignalAspect, 4> aspectValues = {SignalAspect::Stop, SignalAspect::ProceedReducedSpeed, SignalAspect::Proceed, SignalAspect::Unknown};
static const std::array<SignalAspect, 3> setAspectValues = {SignalAspect::Stop, SignalAspect::ProceedReducedSpeed, SignalAspect::Proceed};

Signal3AspectRailTile::Signal3AspectRailTile(World& world, std::string_view _id) :
  SignalRailTile(world, _id, TileId::RailSignal3Aspect)
{
  outputMap.setValueInternal(std::make_shared<SignalOutputMap>(*this, outputMap.name(), std::initializer_list<SignalAspect>{SignalAspect::Stop, SignalAspect::ProceedReducedSpeed, SignalAspect::Proceed}));

  Attributes::addValues(aspect, aspectValues);
  m_interfaceItems.add(aspect);

  Attributes::addValues(setAspect, setAspectValues);
  m_interfaceItems.add(setAspect);
}

void Signal3AspectRailTile::boardModified()
{
  m_signalPath = std::make_unique<SignalPath>(m_node, 2,
    [this](const std::vector<BlockState>& states)
    {
      if(!states.empty() && states[0] == BlockState::Free)
      {
        if(states.size() >= 2 && states[1] == BlockState::Free)
          setAspect(SignalAspect::Proceed);
        else
          setAspect(SignalAspect::ProceedReducedSpeed);
      }
      else
        setAspect(SignalAspect::Stop);
    });
}
