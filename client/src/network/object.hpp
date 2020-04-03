/**
 * client/src/network/object.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_OBJECT_HPP
#define TRAINTASTIC_CLIENT_NETWORK_OBJECT_HPP

#include <QObject>
#include <QSharedPointer>
#include "handle.hpp"
#include "interfaceitems.hpp"

class Connection;
class AbstractProperty;
class Method;

class Object : public QObject
{
  Q_OBJECT

  friend class Connection;

  protected:
    QSharedPointer<Connection> m_connection;
    Handle m_handle;
    const QString m_classId;
    InterfaceItems m_interfaceItems;

  public:
    explicit Object(const QSharedPointer<Connection>& connection, Handle handle, const QString& classId);
    Object(const Object& copy) = delete;
    ~Object() final;

    const QSharedPointer<Connection>& connection() const { return m_connection; }
    Handle handle() const { return m_handle; }

    const QString& classId() const { return m_classId; }

    const InterfaceItems& interfaceItems() const { return m_interfaceItems; }

    const InterfaceItem* getInterfaceItem(const QString& name) const;
    InterfaceItem* getInterfaceItem(const QString& name);

    const AbstractProperty* getProperty(const QString& name) const;
    AbstractProperty* getProperty(const QString& name);

    const Method* getMethod(const QString& name) const;
    Method* getMethod(const QString& name);
};

#endif
