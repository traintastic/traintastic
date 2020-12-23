/**
 * client/src/widget/createwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020 Reinder Feenstra
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

#include "createwidget.hpp"
#include "commandstationlistwidget.hpp"
#include "decoderfunctionlistwidget.hpp"
#include "decoderlistwidget.hpp"
#include "inputlistwidget.hpp"
#include "luascriptlistwidget.hpp"
#include "object/luascripteditwidget.hpp"
#include "object/objecteditwidget.hpp"
#include "inputmonitorwidget.hpp"
#include "outputkeyboardwidget.hpp"
#include "../board/boardwidget.hpp"
#include "../network/object.hpp"
#include "../network/inputmonitor.hpp"
#include "../network/outputkeyboard.hpp"
#include "../network/board.hpp"

QWidget* createWidgetIfCustom(const ObjectPtr& object, QWidget* parent)
{
  const QString& classId = object->classId();

  if(classId == "command_station_list")
    return new CommandStationListWidget(object, parent); // todo remove
  else if(classId == "decoder_function_list")
    return new DecoderListWidget(object, parent); // todo remove
  else if(classId == "decoder_list")
    return new DecoderFunctionListWidget(object, parent); // todo remove
  else if(classId == "input_list")
    return new InputListWidget(object, parent); // todo remove
  else if(classId == "controller_list")
    return new ObjectListWidget(object, parent); // todo remove
  else if(classId == "rail_vehicle_list")
    return new ObjectListWidget(object, parent); // todo remove
  else if(classId == "lua.script_list")
    return new LuaScriptListWidget(object, parent); // todo remove
  else if(classId == "world_list")
    return new ObjectListWidget(object, parent);
  else if(classId == "lua.script")
    return new LuaScriptEditWidget(object, parent);
  else
    return nullptr;
}

QWidget* createWidget(const ObjectPtr& object, QWidget* parent)
{
  if(QWidget* widget = createWidgetIfCustom(object, parent))
    return widget;
  else if(object->classId().startsWith("list."))
    return new ObjectListWidget(object, parent);
  else if(auto inputMonitor = std::dynamic_pointer_cast<InputMonitor>(object))
    return new InputMonitorWidget(inputMonitor, parent);
  else if(auto outputKeyboard = std::dynamic_pointer_cast<OutputKeyboard>(object))
    return new OutputKeyboardWidget(outputKeyboard, parent);
  else if(auto board = std::dynamic_pointer_cast<Board>(object))
    return new BoardWidget(board, parent);
  else
    return new ObjectEditWidget(object, parent);
}
