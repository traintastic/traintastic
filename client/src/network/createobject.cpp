/**
 * client/src/network/createobject.cpp
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

#include "createobject.hpp"
#include "inputmonitor.hpp"
#include "outputkeyboard.hpp"
#include "outputmap.hpp"
#include "board.hpp"
#include "object/blockrailtile.hpp"
#include "object/trainblockstatus.hpp"

Object* createObject(std::shared_ptr<Connection> connection, Handle handle, const QString& classId)
{
  if(classId == InputMonitor::classId)
    return new InputMonitor(std::move(connection), handle, classId);
  if(classId == OutputKeyboard::classId)
    return new OutputKeyboard(std::move(connection), handle, classId);
  if(classId.startsWith(OutputMap::classIdPrefix))
    return new OutputMap(std::move(connection), handle, classId);
  if(classId == Board::classId)
    return new Board(std::move(connection), handle);
  if(classId == BlockRailTile::classId)
    return new BlockRailTile(std::move(connection), handle, classId);
  if(classId == TrainBlockStatus::classId)
    return new TrainBlockStatus(std::move(connection), handle, classId);

  return new Object(std::move(connection), handle, classId);
}
