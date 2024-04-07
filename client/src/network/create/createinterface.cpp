/**
 * client/src/network/create/createinterface.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "createinterface.hpp"
#include "../connection.hpp"
#include "../error.hpp"
#include "../method.hpp"
#include "../object.hpp"
#include "../objectproperty.hpp"

CreateInterface::CreateInterface(ObjectPtr world, QString classId, Properties properties)
  : m_world{std::move(world)}
  , m_classId{std::move(classId)}
  , m_properties{std::move(properties)}
  , m_requestId{Connection::invalidRequestId}
{
  assert(m_world);
  assert(!m_classId.isEmpty());

  m_promise.reportStarted();

  if(auto* interfaces = m_world->getObjectProperty("interfaces"))
  {
    m_requestId = interfaces->getObject(
      [this](const ObjectPtr& interfaceList, std::optional<const Error> /*error*/)
      {
        m_requestId = Connection::invalidRequestId;
        if(interfaceList && !m_canceled)
        {
          m_interfaceList = interfaceList;

          if(auto* create = m_interfaceList->getMethod("create"))
          {
            m_requestId = create->call(m_classId,
              [this](const ObjectPtr& interface, std::optional<const Error> /*error*/)
              {
                m_requestId = Connection::invalidRequestId;
                if(m_canceled)
                {
                  if(interface)
                  {
                    if(auto* del = m_interfaceList->getMethod("delete"))
                    {
                      del->call(interface->getPropertyValueString("id"));
                    }
                  }
                }
                else if(interface)
                {
                  for(const auto& it : m_properties)
                  {
                    // a bit hacky but easier for now #FIXME
                    auto event = Message::newEvent(Message::Command::ObjectSetProperty);
                    event->write(interface->handle());
                    event->write(it.first);

                    switch(it.second.type())
                    {
                      case QVariant::Bool:
                        event->write(ValueType::Boolean);
                        event->write(it.second.toBool());
                        break;

                      case QVariant::Int:
                      case QVariant::UInt:
                      case QVariant::LongLong:
                      case QVariant::ULongLong:
                        event->write(ValueType::Integer);
                        event->write(it.second.toLongLong());
                        break;

                      case QVariant::Double:
                        event->write(ValueType::Float);
                        event->write(it.second.toDouble());
                        break;

                      case QVariant::String:
                        event->write(ValueType::String);
                        event->write(it.second.toString().toUtf8());
                        break;

                      default: /*[[unlikely]]*/
                        assert(false);
                        continue;
                    }
                    interface->connection()->send(event);
                  }
                  m_promise.reportFinished(&interface);
                }
                else
                {
                  //interfacePromise.reportException();
                }
              });
          }
          else
          {
            //interfacePromise.reportException();
          }
        }
      });
  }
}

CreateInterface::~CreateInterface()
{
  if(!m_promise.isFinished() && !m_canceled)
  {
    cancel();
  }
}

void CreateInterface::cancel()
{
  m_canceled = true;
  if(!m_promise.isFinished())
  {
    m_promise.reportCanceled();
  }

  if(m_interfaceList && m_promise.isResultReadyAt(0))
  {
    if(auto* del = m_interfaceList->getMethod("delete"))
    {
      del->call(m_promise.resultReference(0)->getPropertyValueString("id"));
    }
  }
}
