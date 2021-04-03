/**
 * client/src/network/outputkeyboard.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#include "outputkeyboard.hpp"
#include "connection.hpp"

OutputKeyboard::OutputKeyboard(std::shared_ptr<Connection> connection, Handle handle, const QString& classId) :
  Object(std::move(connection), handle, classId),
  m_requestId{Connection::invalidRequestId}
{
}

OutputKeyboard::~OutputKeyboard()
{
  if(m_requestId != Connection::invalidRequestId)
    m_connection->cancelRequest(m_requestId);
}

void OutputKeyboard::refresh()
{
  if(m_requestId != Connection::invalidRequestId)
    m_connection->cancelRequest(m_requestId);
  m_requestId = m_connection->getOutputKeyboardOutputInfo(*this);
}

void OutputKeyboard::outputSetValue(uint32_t address, bool value)
{
  m_connection->setOutputKeyboardOutputValue(*this, address, value);
}
