/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "trainroutelist.hpp"
#include "trainroute.hpp"
#include "trainroutelisttablemodel.hpp"
#include "../core/attributes.hpp"
#include "../core/method.tpp"
#ifdef ENABLE_LOG_DEBUG
  #include "../log/log.hpp"
#endif
#include "../log/logmessageexception.hpp"
#include "../utils/displayname.hpp"
#include "../world/getworld.hpp"
#include "../world/world.hpp"

TrainRouteList::TrainRouteList(Object& _parent, std::string_view parentPropertyName)
  : ObjectList<TrainRoute>(_parent, parentPropertyName)
  , create{*this, "create",
      [this]()
      {
        auto& world = getWorld(parent());
        return TrainRoute::create(world, world.getUniqueId("train_route"));
      }}
  , delete_{*this, "delete",
      [this](const std::shared_ptr<TrainRoute>& route)
      {
        // FIXME: what to do if in use??
        deleteMethodHandler(route);
      }}
{
  Attributes::addDisplayName(create, DisplayName::List::create);
  m_interfaceItems.add(create);

  Attributes::addDisplayName(delete_, DisplayName::List::delete_);
  m_interfaceItems.add(delete_);
}

void TrainRouteList::resolve()
{
#ifdef ENABLE_LOG_DEBUG
    const auto start = std::chrono::steady_clock::now();
#endif

  for(auto& route : m_items)
  {
    route->resolve();
  }

#ifdef ENABLE_LOG_DEBUG
  const auto duration = std::chrono::steady_clock::now() - start;
  Log::debug("Train route resolve took", std::chrono::duration_cast<std::chrono::microseconds>(duration).count() / 1e3 , "ms");
#endif
}

TableModelPtr TrainRouteList::getModel()
{
  return std::make_shared<TrainRouteListTableModel>(*this);
}

bool TrainRouteList::isListedProperty(std::string_view name)
{
  return TrainRouteListTableModel::isListedProperty(name);
}
