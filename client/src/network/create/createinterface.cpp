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
#include "setobjectproperties.hpp"
#include "../connection.hpp"
#include "../error.hpp"
#include "../method.hpp"
#include "../object.hpp"
#include "../property.hpp"
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
                  m_interface = interface;

                  if(m_properties.empty())
                  {
                    m_promise.reportFinished(&m_interface);
                  }
                  else
                  {
                    m_setObjectProperties = std::make_shared<SetObjectProperties>(m_interface, m_properties);
                    if(m_setObjectProperties->future().isFinished())
                    {
                      m_promise.reportFinished(&m_interface);
                    }
                    else
                    {
                      m_setObjectPropertiesFutureWatcher = std::make_unique<QFutureWatcher<ObjectPtr>>();
                      m_setObjectPropertiesFutureWatcher->setFuture(m_setObjectProperties->future());
                      QObject::connect(m_setObjectPropertiesFutureWatcher.get(), &QFutureWatcher<ObjectPtr>::finished,
                        [this]()
                        {
                          if(!m_canceled)
                          {
                            m_promise.reportFinished(&m_interface);
                          }
                        });
                    }
                  }
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
  if(m_requestId != Connection::invalidRequestId)
  {
    m_world->connection()->cancelRequest(m_requestId);
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
