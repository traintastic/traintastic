/**
 * client/src/network/object/blockrailtile.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023-2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_OBJECT_BLOCKRAILTILE_HPP
#define TRAINTASTIC_CLIENT_NETWORK_OBJECT_BLOCKRAILTILE_HPP

#include "../object.hpp"
#include "../objectptr.hpp"

class ObjectVectorProperty;

class BlockRailTile final : public Object
{
  Q_OBJECT

  CLASS_ID("board_tile.rail.block")

  private:
    int m_requestId;
    ObjectVectorProperty* m_trainsProperty = nullptr;
    std::vector<ObjectPtr> m_trains;

    void updateTrains();
    void emitTrainsChanged();

  protected:
    void created() final;

  public:
    BlockRailTile(const std::shared_ptr<Connection>& connection, Handle handle, const QString& classId_);
    ~BlockRailTile() final;

    const std::vector<ObjectPtr>& trains() const
    {
      return m_trains;
    }

  signals:
    void trainsChanged();
};

#endif
