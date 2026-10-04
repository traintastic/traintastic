/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2024-2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_MISC_MIMEDATA_HPP
#define TRAINTASTIC_CLIENT_MISC_MIMEDATA_HPP

#include <QMimeData>
#include <QIODevice>
#include <traintastic/enum/blocktraindirection.hpp>

class BlockReservePathMimeData : public QMimeData
{
public:
  inline static const auto mimeType = QLatin1String("application/vnd.traintastic.block_reserve_path");

  explicit BlockReservePathMimeData(const QString& blockId, BlockTrainDirection direction)
  {
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out << blockId << static_cast<std::underlying_type_t<BlockTrainDirection>>(direction);
    setData(mimeType, payload);
  }

  inline std::tuple<QString, BlockTrainDirection> values() const
  {
    QString blockId;
    std::underlying_type_t<BlockTrainDirection> directionRaw;
    QDataStream in(data(mimeType));
    in >> blockId >> directionRaw;
    return std::make_tuple(blockId, static_cast<BlockTrainDirection>(directionRaw));
  }
};

class AssignTrainMimeData : public QMimeData
{
public:
  inline static const auto mimeType = QLatin1String("application/vnd.traintastic.assign_train");

  explicit AssignTrainMimeData(const QString& trainId)
  {
    setData(mimeType, trainId.toUtf8());
  }

  inline QString trainId() const
  {
    return QString::fromUtf8(data(mimeType));
  }
};

#endif
