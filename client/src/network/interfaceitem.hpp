/**
 * client/src/network/interfaceitem.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_INTERFACEITEM_HPP
#define TRAINTASTIC_CLIENT_NETWORK_INTERFACEITEM_HPP

#include <QObject>
#include <QMap>
#include <QVariant>
#include <traintastic/enum/attributename.hpp>

class Object;

class InterfaceItem : public QObject
{
  Q_OBJECT

  friend class Connection;

  protected:
    const QString m_name;
    QMap<AttributeName, QVariant> m_attributes;

  public:
    explicit InterfaceItem(Object& object, const QString& name);

    const Object& object() const;
    Object& object();
    const QString& name() const { return m_name; }
    QString displayName() const;

    bool hasAttribute(AttributeName name) const;
    QVariant getAttribute(AttributeName name, const QVariant& default_) const;
    bool getAttributeBool(AttributeName name, bool default_) const;
    int getAttributeInt(AttributeName name, int default_) const;
    qint64 getAttributeInt64(AttributeName name, qint64 default_) const;
    double getAttributeDouble(AttributeName name, double default_) const;
    QString getAttributeString(AttributeName name, const QString& default_) const;

    template<typename T>
    T getAttributeEnum(AttributeName name, T default_) const
    {
      return static_cast<T>(getAttributeInt64(name, static_cast<qint64>(default_)));
    }

  signals:
    void attributeChanged(AttributeName name, const QVariant& value);
};

#endif
