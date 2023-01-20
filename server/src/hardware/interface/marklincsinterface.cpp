/**
 * server/src/hardware/interface/marklincsinterface.cpp
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

#include "marklincsinterface.hpp"
#include "../../core/attributes.hpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/displayname.hpp"

MarklinCSInterface::MarklinCSInterface(World& world, std::string_view _id)
  : Interface(world, _id)
  , hostname{this, "hostname", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  name = "M\u00E4rklin CS2/CS3";

  Attributes::addDisplayName(hostname, DisplayName::IP::hostname);
  Attributes::addEnabled(hostname, !online);
  m_interfaceItems.insertBefore(hostname, notes);
}

bool MarklinCSInterface::setOnline(bool& value, bool /*simulation*/)
{
  if(/*!m_kernel &&*/ value)
  {
    try
    {
      status.setValueInternal(InterfaceStatus::Online);

      Attributes::setEnabled({hostname}, false);
    }
    catch(const LogMessageException& e)
    {
      status.setValueInternal(InterfaceStatus::Offline);
      Log::log(*this, e.message(), e.args());
      return false;
    }
  }
  else //if(m_kernel && !value)
  {
    Attributes::setEnabled({hostname}, true);

    status.setValueInternal(InterfaceStatus::Offline);
  }
  return true;
}

void MarklinCSInterface::addToWorld()
{
  Interface::addToWorld();
}

void MarklinCSInterface::destroying()
{
  Interface::destroying();
}

void MarklinCSInterface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);

  //if(m_kernel)
  {
    switch(event)
    {
      case WorldEvent::PowerOff:
        break;

      case WorldEvent::PowerOn:
        break;

      case WorldEvent::Stop:
        break;

      case WorldEvent::Run:
        break;

      default:
        break;
    }
  }
}
