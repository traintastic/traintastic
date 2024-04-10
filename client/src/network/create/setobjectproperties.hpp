/**
 * client/src/network/create/setbjectproperties.hpp
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_CREATE_SETOBJECTPROPERTIES_HPP
#define TRAINTASTIC_CLIENT_NETWORK_CREATE_SETOBJECTPROPERTIES_HPP

#include <memory>
#include <QFuture>
#include <QFutureInterface>
#include <QFutureWatcher>
#include "properties.hpp"
#include "../objectptr.hpp"

class ObjectProperty;

class SetObjectProperties : public std::enable_shared_from_this<SetObjectProperties>
{
  private:
    QFutureInterface<ObjectPtr> m_promise;
    ObjectPtr m_object;
    ObjectProperty* m_objectProperty = nullptr;
    Properties m_properties;
    int m_requestId;
    std::vector<std::shared_ptr<SetObjectProperties>> m_setObjectProperties;
    std::vector<std::unique_ptr<QFutureWatcher<ObjectPtr>>> m_setObjectPropertiesFutureWatcher;
    size_t m_setObjectPropertiesFinishedCount = 0;
    bool m_canceled = false;

    void setProperties();

  public:
    SetObjectProperties(ObjectPtr object, Properties properties);
    SetObjectProperties(ObjectProperty& objectProperty, Properties properties);
    ~SetObjectProperties();

    void cancel();

    QFuture<ObjectPtr> future()
    {
      return m_promise.future();
    }
};

#endif
