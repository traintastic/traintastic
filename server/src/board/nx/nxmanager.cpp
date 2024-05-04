/**
 * server/src/board/nx/nxmanager.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#include "nxmanager.hpp"
#include "../map/blockpath.hpp"
#include "../tile/rail/blockrailtile.hpp"
#include "../tile/rail/nxbuttonrailtile.hpp"
#include "../../core/method.tpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../train/trainblockstatus.hpp"
#include "../../world/getworld.hpp"

NXManager::NXManager(Object& parent_, std::string_view parentPropertyName)
  : SubObject{parent_, parentPropertyName}
  , select{*this, "select",
      [this](const std::shared_ptr<NXButtonRailTile>& from, const std::shared_ptr<NXButtonRailTile>& to)
      {
        if(!from || !to) /*[[unlikely]]*/
        {
          return;
        }
        if(!contains(getWorld(this).state, WorldState::Run))
        {
          return;
        }
        selectPath(*from, *to);
      }}
{
  m_interfaceItems.add(select);
}

void NXManager::pressed(NXButtonRailTile& button)
{
  if(!contains(getWorld(this).state, WorldState::Run))
  {
    return;
  }

  assert(button.block);

  if(!m_pressedButtons.empty())
  {
    for(auto& buttonFirstWeak : m_pressedButtons)
    {
      if(const auto buttonFirst = buttonFirstWeak.lock())
      {
        if(selectPath(*buttonFirst, button))
        {
          released(*buttonFirst);
          return;
        }
      }
    }
  }

  m_pressedButtons.emplace_back(button.shared_ptr<NXButtonRailTile>());
}

void NXManager::released(NXButtonRailTile& button)
{
  const auto it = std::find_if(m_pressedButtons.begin(), m_pressedButtons.end(),
    [&button](const auto& weakPtr)
    {
      return weakPtr.lock().get() == &button;
    });

  if(it != m_pressedButtons.end())
  {
    m_pressedButtons.erase(it);
  }
}

bool NXManager::selectPath(const NXButtonRailTile& from, const NXButtonRailTile& to)
{
  for(auto& path : from.block->paths())
  {
    if(path->nxButtonTo().get() == &to && path->nxButtonFrom().get() == &from)
    {
      LOG_DEBUG("Path found:", path->fromBlock().name.value(), "->", path->toBlock()->name.value());

      if(path->isReserved())
      {
        // If user clicked an already reserved path we release it.
        // TODO: make some logic to prevent releasing a path while train is inside it?
        // Also releasing a path when we already set signal to "Proceed" is dangerous because train might
        // have already started moving towards out path
        return path->release();
      }

      if(from.block->trains.empty())
      {
        continue; // no train in from block
      }

      const auto& status = path->fromSide() == BlockSide::A ? from.block->trains.front() : from.block->trains.back();
      if(!status->train)
      {
        continue; // no train assigned in from block
      }

      if(!path->reserve(status->train.value()))
      {
        continue; // can't reserve path
      }

      return true;
    }
  }
  return false; // no path found
}
