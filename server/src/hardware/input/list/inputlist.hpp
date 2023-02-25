/**
 * server/src/hardware/input/list/inpulist.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INPUT_LIST_INPUTLIST_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INPUT_LIST_INPUTLIST_HPP

#include "../../../core/objectlist.hpp"
#include "inputlistcolumn.hpp"
#include "../../../core/method.hpp"
#include "../monitor/inputmonitor.hpp"

class Input;

class InputList : public ObjectList<Input>
{
  CLASS_ID("list.input")

  protected:
    void worldEvent(WorldState state, WorldEvent event) final;
    bool isListedProperty(std::string_view name) final;

  public:
    const InputListColumn columns;

    Method<std::shared_ptr<Input>()> create;
    Method<void(const std::shared_ptr<Input>&)> delete_;
    Method<std::shared_ptr<InputMonitor>()> inputMonitor;
    Method<std::shared_ptr<InputMonitor>(uint32_t)> inputMonitorChannel;

    InputList(Object& _parent, std::string_view parentPropertyName, InputListColumn _columns);

    TableModelPtr getModel() final;
};

#endif
