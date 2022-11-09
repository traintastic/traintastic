/**
 * client/src/network/object.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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
#include <memory>
#include "handle.hpp"
#include "interfaceitems.hpp"

class Connection;
class Message;
class AbstractProperty;
class AbstractVectorProperty;
class Method;
class Event;

class Object : public QObject
{
  Q_OBJECT

  friend class Connection;

  protected:
    std::shared_ptr<Connection> m_connection;
    Handle m_handle;
    const QString m_classId;
    InterfaceItems m_interfaceItems;

    virtual void processMessage(const Message& message);

  public:
    explicit Object(std::shared_ptr<Connection> connection, Handle handle, const QString& classId);
    Object(const Object&) = delete;

    const std::shared_ptr<Connection>& connection() const { return m_connection; }
    Handle handle() const { return m_handle; }

    const QString& classId() const { return m_classId; }

    const InterfaceItems& interfaceItems() const { return m_interfaceItems; }

    const InterfaceItem* getInterfaceItem(const QString& name) const;
    InterfaceItem* getInterfaceItem(const QString& name);

    inline bool hasProperty(const QString& name) const { return getProperty(name); }
    const AbstractProperty* getProperty(const QString& name) const;
    AbstractProperty* getProperty(const QString& name);

    /**
     * \brief Get boolean property value
     * \param[in] name Property name
     * \param[in] defaultValue Value to return if property doesn't exist or isn't boolean
     * \return Property value or \c defaultValue if property doesn't exist or isn't boolean
     */
    bool getPropertyValueBool(const QString& name, bool defaultValue) const;

    /**
     * \brief Get integer property value
     * \param[in] name Property name
     * \param[in] defaultValue Value to return if property doesn't exist or isn't an integer
     * \return Property value or \c defaultValue if property doesn't exist or isn't an integer
     */
    int getPropertyValueInt(const QString& name, int defaultValue) const;

    inline bool hasVectorProperty(const QString& name) const { return getVectorProperty(name); }
    const AbstractVectorProperty* getVectorProperty(const QString& name) const;
    AbstractVectorProperty* getVectorProperty(const QString& name);

    void setPropertyValue(const QString& name, bool value);

    const Method* getMethod(const QString& name) const;
    Method* getMethod(const QString& name);

    const Event* getEvent(const QString& name) const;
    Event* getEvent(const QString& name);

    void callMethod(const QString& name);
};

#endif
