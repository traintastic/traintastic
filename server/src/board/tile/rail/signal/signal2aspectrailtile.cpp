/**
 * server/src/board/tile/rail/signal/signal2aspectrailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2021 Reinder Feenstra
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
#include "../../../../core/attributes.hpp"
#include "../../../../utils/makearray.hpp"

Signal2AspectRailTile::Signal2AspectRailTile(const std::weak_ptr<World>& world, std::string_view _id) :
  SignalRailTile(world, _id, TileId::RailSignal2Aspect)
{
  outputMap.setValueInternal(std::make_shared<SignalOutputMap>(*this, outputMap.name(), std::initializer_list<SignalAspect>{SignalAspect::Stop, SignalAspect::Proceed}));

  Attributes::addValues(aspect, makeArray(SignalAspect::Stop, SignalAspect::Proceed, SignalAspect::Unknown));
  m_interfaceItems.add(aspect);
}

void Signal2AspectRailTile::doNextAspect(bool /*reverse*/)
{
  aspect = (aspect == SignalAspect::Stop) ? SignalAspect::Proceed : SignalAspect::Stop;
}
