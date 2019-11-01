/**
 * hardware/commandstation/li10x.cpp
 *
 * This file is part of the traintastic source code
 *
 * Copyright (C) 2019 Reinder Feenstra <reinderfeenstra@gmail.com>
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

#include "li10x.hpp"

namespace Hardware::CommandStation {

LI10x::LI10x(const std::string& _id) :
  CommandStation(_id),
  m_xpressnet{std::make_shared<Protocol::XpressNet>()},
  port{this, "port", "", PropertyFlags::AccessWCC},
  baudrate{this, "baudrate", 9600, PropertyFlags::AccessWCC}
{
  m_interfaceItems.add(port);
  m_interfaceItems.add(baudrate);
}

bool LI10x::isDecoderSupported(Decoder& decoder) const
{
  return m_xpressnet->isDecoderSupported(decoder);
}

}
