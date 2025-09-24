/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#include "decoderfunctionsmodel.hpp"
#include <traintastic/locale/locale.hpp>
#include "../../network/connection.hpp"
#include "../../network/error.hpp"
#include "../../network/abstractproperty.hpp"
#include "../../network/object.hpp"
#include "../../network/objectvectorproperty.hpp"
#include "../../utils/enum.hpp"

namespace {

const std::array<QString, 4> columnProperty{{
  QStringLiteral("number"),
  QStringLiteral("function"),
  QStringLiteral("name"),
  QStringLiteral("type"),
}};

}

DecoderFunctionsModel::DecoderFunctionsModel(ObjectPtr object, QObject* parent)
  : QAbstractTableModel(parent)
  , m_object{std::move(object)}
  , m_items{m_object->getObjectVectorProperty("items")}
  , m_requestId{Connection::invalidRequestId}
{
  connect(m_items, &ObjectVectorProperty::valueChanged, this, &DecoderFunctionsModel::itemsChanged);
  itemsChanged();
}

DecoderFunctionsModel::~DecoderFunctionsModel()
{
  cancelRequest();
}

const ObjectPtr& DecoderFunctionsModel::getObject(int row) const
{
  assert(row >= 0 && row < static_cast<int>(m_functions.size()));
  return m_functions[row];
}

Qt::ItemFlags DecoderFunctionsModel::flags(const QModelIndex& index) const
{
  if(auto* p = m_functions[index.row()]->getProperty(columnProperty[index.column()]); p && p->getAttributeBool(AttributeName::Enabled, true))
  {
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
  }
  return QAbstractTableModel::flags(index);
}

int DecoderFunctionsModel::rowCount(const QModelIndex& /*parent*/) const
{
  return static_cast<int>(m_functions.size());
}

int DecoderFunctionsModel::columnCount(const QModelIndex& /*parent*/) const
{
  return 4;
}

QVariant DecoderFunctionsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
  {
    switch(section)
    {
      case columnNumber:
        return QStringLiteral("#");

      case columnFunction:
        return Locale::tr("decoder_function:function");

      case columnName:
        return Locale::tr("object:name");

      case columnType:
        return Locale::tr("decoder_function:type");
    }
  }
  return {};
}

QVariant DecoderFunctionsModel::data(const QModelIndex& index, int role) const
{
  if(role == Qt::DisplayRole || role == Qt::EditRole)
  {
    const auto& function = *m_functions[index.row()];

    if(role == Qt::EditRole)
    {
      return function.getPropertyValue(columnProperty[index.column()]);
    }

    switch(index.column())
    {
      case columnNumber:
        return QString("F%1").arg(function.getPropertyValueInt(QStringLiteral("number"), -1));

      case columnFunction:
        if(auto* property = function.getProperty(QStringLiteral("function"))) [[likely]]
        {
          return translateEnum(*property);
        }
        break;

      case columnName:
        return function.getPropertyValueString(QStringLiteral("name"));

      case columnType:
        if(auto* property = function.getProperty(QStringLiteral("type"))) [[likely]]
        {
          return translateEnum(*property);
        }
        break;
    }
  }
  return {};
}

bool DecoderFunctionsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if(role == Qt::EditRole)
  {
    auto& function = *m_functions[index.row()];
    switch(index.column())
    {
      case columnNumber:
        function.setPropertyValue(columnProperty[index.column()], value.toInt());
        break;

      default:
        function.setPropertyValue(columnProperty[index.column()], value);
        break;
    }
    return true;
  }
  return false;
}

void DecoderFunctionsModel::cancelRequest()
{
  if(m_requestId != Connection::invalidRequestId)
  {
    m_items->object().connection()->cancelRequest(m_requestId);
    m_requestId = Connection::invalidRequestId;
  }
}

void DecoderFunctionsModel::itemsChanged()
{
  if(!m_items) [[unlikely]]
  {
    return;
  }

  cancelRequest();

  if(!m_items->empty())
  {
    m_requestId = m_items->getObjects(
      [this](const std::vector<ObjectPtr>& objects, std::optional<const Error> error)
      {
        m_requestId = Connection::invalidRequestId;
        if(!error)
        {
          beginResetModel();
          m_functions = objects;
          endResetModel();
        }
      });
  }
  else
  {
    beginResetModel();
    m_functions.clear();
    endResetModel();
  }
}
