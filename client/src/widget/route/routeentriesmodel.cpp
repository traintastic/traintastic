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

#include "routeentriesmodel.hpp"
#include <traintastic/enum/trainrouteentrysource.hpp>
#include "../../network/connection.hpp"
#include "../../network/error.hpp"
#include "../../network/object.tpp"
#include "../../network/objectproperty.hpp"
#include "../../network/objectvectorproperty.hpp"

RouteEntriesModel::RouteEntriesModel(ObjectPtr object, QObject* parent)
  : QAbstractListModel(parent)
  , m_object{std::move(object)}
  , m_entriesResolved{m_object->getObjectVectorProperty("entries_resolved")}
  , m_entriesResolvedRequestId{Connection::invalidRequestId}
{
  if(m_entriesResolved) [[likely]]
  {
    connect(m_entriesResolved, &ObjectVectorProperty::valueChanged, this, &RouteEntriesModel::entriesResolvedChanged);
    entriesResolvedChanged();
  }
}

RouteEntriesModel::~RouteEntriesModel()
{
  if(m_entriesResolvedRequestId != Connection::invalidRequestId)
  {
    m_object->connection()->cancelRequest(m_entriesResolvedRequestId);
  }
}

int RouteEntriesModel::rowCount(const QModelIndex& /*parent*/) const
{
  return static_cast<int>(m_items.size());
}

QVariant RouteEntriesModel::data(const QModelIndex& index, int role) const
{
  if(index.row() >= 0 && index.row() < static_cast<int>(m_items.size()))
  {
    auto& item = *m_items[index.row()];

    switch(role)
    {
      case Qt::DisplayRole:
        if(auto* block = item.getObjectProperty("block")) [[likely]]
        {
          return block->objectId();
        }
        break;

      case iconRole:
      {
        auto icon = Icon::None;
        if(m_items.size() >= 2)
        {
          if(index.row() == 0)
          {
            icon = Icon::First;
          }
          else if(index.row() == static_cast<int>(m_items.size()) - 1)
          {
            icon = Icon::Last;
          }
          else if(auto* block = item.getObjectProperty("block"); block && !block->objectId().isEmpty())
          {
            if(item.getPropertyValueInt("wait_time_max", 0) > 0)
            {
              icon = Icon::LineWithDot;
            }
            else
            {
              icon = Icon::Line;
            }
          }
        }
        return static_cast<uint>(icon);
      }
      case sourceRole:
        return static_cast<uint>(item.getPropertyValueEnum<TrainRouteEntrySource>("source", static_cast<TrainRouteEntrySource>(0)));

      case handleRole:
        return item.handle();
    }
  }
  return {};
}

ObjectPtr RouteEntriesModel::getObject(int row) const
{
  if(row >= 0 && row < static_cast<int>(m_items.size()))
  {
    return m_items[row];
  }
  return {};
}

void RouteEntriesModel::entriesResolvedChanged()
{
  if(m_entriesResolvedRequestId != Connection::invalidRequestId)
  {
    m_object->connection()->cancelRequest(m_entriesResolvedRequestId);
    m_entriesResolvedRequestId = Connection::invalidRequestId;
  }

  if(!m_entriesResolved->empty())
  {
    m_entriesResolvedRequestId = m_entriesResolved->getObjects(
      [this](const std::vector<ObjectPtr>& objects, std::optional<const Error> /*err*/)
      {
        beginResetModel();
        m_items = objects;
        endResetModel();
      });
  }
  else
  {
    beginResetModel();
    m_items.clear();
    endResetModel();
  }
}
