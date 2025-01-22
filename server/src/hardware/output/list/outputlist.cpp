/**
 * server/src/hardware/output/list/outputlist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2024 Reinder Feenstra
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

#include "outputlist.hpp"
#include "outputlisttablemodel.hpp"
#include "../outputcontroller.hpp"
#include "../keyboard/outputkeyboard.hpp"
#include "../../../world/getworld.hpp"
#include "../../../core/attributes.hpp"
#include "../../../core/method.tpp"
#include "../../../core/objectproperty.tpp"
#include "../../../utils/displayname.hpp"

OutputList::OutputList(Object& _parent, std::string_view parentPropertyName, OutputListColumn _columns)
  : ObjectList<Output>(_parent, parentPropertyName)
  , columns{_columns}
  , outputKeyboard{*this, "output_keyboard",
      [this](OutputChannel channel)
      {
        if(const auto controller = std::dynamic_pointer_cast<OutputController>(parent().shared_from_this()))
        {
          return controller->outputKeyboard(channel);
        }
        return std::shared_ptr<OutputKeyboard>();
      }}
{
  if(auto* controller = dynamic_cast<OutputController*>(&_parent))
  {
    std::vector<OutputChannel> channels;
    for(auto channel : controller->outputChannels())
    {
      if(controller->hasOutputKeyboard(channel))
      {
        channels.emplace_back(channel);
      }
    }

    if(!channels.empty()) /*[[likely]]*/
    {
      Attributes::addDisplayName(outputKeyboard, DisplayName::Hardware::outputKeyboard);
      Attributes::addValues(outputKeyboard, std::move(channels));
      m_interfaceItems.add(outputKeyboard);
    }
  }
}

TableModelPtr OutputList::getModel()
{
  return std::make_shared<OutputListTableModel>(*this);
}

bool OutputList::isListedProperty(std::string_view name)
{
  return OutputListTableModel::isListedProperty(name);
}
