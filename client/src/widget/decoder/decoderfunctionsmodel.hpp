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

#ifndef TRAINTASTIC_CLIENT_WIDGET_DECODER_DECODERFUNCTIONSMODEL_HPP
#define TRAINTASTIC_CLIENT_WIDGET_DECODER_DECODERFUNCTIONSMODEL_HPP

#include <QAbstractTableModel>
#include "../../network/objectptr.hpp"

class ObjectVectorProperty;

class DecoderFunctionsModel : public QAbstractTableModel
{
private:
  ObjectPtr m_object;
  ObjectVectorProperty* m_items;
  std::vector<ObjectPtr> m_functions;
  int m_requestId;

  void cancelRequest();
  void itemsChanged();

public:
  static constexpr int columnNumber = 0;
  static constexpr int columnFunction = 1;
  static constexpr int columnName = 2;
  static constexpr int columnType = 3;

  explicit DecoderFunctionsModel(ObjectPtr object, QObject* parent = nullptr);
  ~DecoderFunctionsModel() override;

  const ObjectPtr& getObject(int row) const;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
};

#endif
