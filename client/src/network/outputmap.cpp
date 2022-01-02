/**
 * client/src/network/outputmap.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#include "outputmap.hpp"
#include "connection.hpp"

OutputMap::OutputMap(std::shared_ptr<Connection> connection, Handle handle, const QString& classId) :
  Object(std::move(connection), handle, classId),
  m_getItemsRequestId{Connection::invalidRequestId},
  m_getOutputsRequestId{Connection::invalidRequestId}
{
}

OutputMap::~OutputMap()
{
  if(m_getItemsRequestId != Connection::invalidRequestId)
    if(auto c = connection())
      c->cancelRequest(m_getItemsRequestId);
}

void OutputMap::getItems()
{
  if(m_getItemsRequestId != Connection::invalidRequestId)
    return;

  if(auto c = connection())
  {
    auto request = Message::newRequest(Message::Command::OutputMapGetItems);
    request->write(handle());
    c->send(request,
      [this](const std::shared_ptr<Message> response)
      {
        m_getItemsRequestId = Connection::invalidRequestId;

        if(auto con = connection())
        {
          Items objects;
          while(!response->endOfMessage())
            objects.emplace_back(con->readObject(*response));
          m_items = std::move(objects);

          emit itemsChanged();
        }
      });
    m_getItemsRequestId = request->requestId();
  }
}

void OutputMap::getOutputs()
{
  if(m_getOutputsRequestId != Connection::invalidRequestId)
    return;

  if(auto c = connection())
  {
    auto request = Message::newRequest(Message::Command::OutputMapGetOutputs);
    request->write(handle());
    c->send(request,
      [this](const std::shared_ptr<Message> response)
      {
        m_getOutputsRequestId = Connection::invalidRequestId;
        readOutputs(*response);
      });
    m_getOutputsRequestId = request->requestId();
  }
}

void OutputMap::processMessage(const Message& message)
{
  switch(message.command())
  {
    case Message::Command::OutputMapOutputsChanged:
      readOutputs(message);
      break;

    default:
      Q_ASSERT(false);
      break;
  }
}

void OutputMap::readOutputs(const Message& message)
{
  if(auto c = connection())
  {
    Outputs objects;
    while(!message.endOfMessage())
      objects.emplace_back(c->readObject(message));
    m_outputs = std::move(objects);

    emit outputsChanged();
  }
}
