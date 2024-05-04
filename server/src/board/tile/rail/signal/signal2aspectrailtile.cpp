/**
 * server/src/board/tile/rail/signal/signal2aspectrailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2024 Reinder Feenstra
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

#include "signal2aspectrailtile.hpp"
#include "../../../map/abstractsignalpath.hpp"
#include "../../../map/blockpath.hpp"
#include "../../../../core/attributes.hpp"
#include "../../../../core/method.tpp"
#include "../../../../core/objectproperty.tpp"
#include "../../../../hardware/output/outputcontroller.hpp"

static const std::array<SignalAspect, 3> aspectValues = {SignalAspect::Stop, SignalAspect::Proceed, SignalAspect::Unknown};
static const std::array<SignalAspect, 2> setAspectValues = {SignalAspect::Stop, SignalAspect::Proceed};

namespace
{
  class SignalPath : public AbstractSignalPath
  {
    protected:
      SignalAspect determineAspect() const final
      {
        std::array<BlockState, 1> states;
        getBlockStates(states);

        if(!requireReservation() && states[0] == BlockState::Free)
        {
          return SignalAspect::Proceed;
        }
        if(states[0] == BlockState::Reserved)
        {
          const auto path = signal().reservedPath();
          if(path && path->toBlock() == getBlock(0))
          {
            return SignalAspect::Proceed;
          }
        }
        return SignalAspect::Stop;
      }

    public:
      SignalPath(Signal2AspectRailTile& signal)
        : AbstractSignalPath(signal, 1)
      {
      }
  };
}

Signal2AspectRailTile::Signal2AspectRailTile(World& world, std::string_view _id) :
  SignalRailTile(world, _id, TileId::RailSignal2Aspect)
{
  outputMap.setValueInternal(std::make_shared<SignalOutputMap>(*this, outputMap.name(), std::initializer_list<SignalAspect>{SignalAspect::Stop, SignalAspect::Proceed}, getDefaultActionValue));

  Attributes::addValues(aspect, aspectValues);
  m_interfaceItems.add(aspect);

  Attributes::addValues(setAspect, setAspectValues);
  m_interfaceItems.add(setAspect);

  connectOutputMap();
}

void Signal2AspectRailTile::boardModified()
{
  m_signalPath = std::make_unique<SignalPath>(*this);
  SignalRailTile::boardModified();
}
