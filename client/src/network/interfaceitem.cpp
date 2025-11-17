/**
 * client/src/network/interfaceitem.cpp
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

#include "interfaceitem.hpp"
#include "object.hpp"
#include <traintastic/locale/locale.hpp>

InterfaceItem::InterfaceItem(Object& object, const QString& name) :
  QObject(&object),
  m_name{name}
{
}

const Object& InterfaceItem::object() const
{
  return *static_cast<Object*>(parent());
}

Object& InterfaceItem::object()
{
  return *static_cast<Object*>(parent());
}

QString InterfaceItem::displayName() const
{
  QString id;
  if(QVariant attr = getAttribute(AttributeName::DisplayName, QVariant()); attr.isValid())
    id = attr.toString();
  else
    id = QString(object().classId()).append(':').append(name());
  return Locale::tr(id);
}
QString InterfaceItem::helpText() const
{
    QString id;

    if (QVariant attr = getAttribute(AttributeName::Help, QVariant()); attr.isValid())
        id = attr.toString();
    else if (QVariant displayAttr = getAttribute(AttributeName::DisplayName, QVariant()); displayAttr.isValid())
        id = displayAttr.toString() + "/help";
    else
        id = QString(object().classId()) + ":" + name() + "/help";

    QString translated = Locale::tr(id);
    if (translated == id)
        return QString(); 

    return translated;
}

bool InterfaceItem::hasAttribute(AttributeName name) const
{
  return m_attributes.contains(name);
}

QVariant InterfaceItem::getAttribute(AttributeName name, const QVariant& default_) const
{
  return m_attributes.value(name, default_);
}

bool InterfaceItem::getAttributeBool(AttributeName name, bool default_) const
{
  return m_attributes.value(name, default_).toBool();
}

int InterfaceItem::getAttributeInt(AttributeName name, int default_) const
{
  return m_attributes.value(name, default_).toInt();
}

qint64 InterfaceItem::getAttributeInt64(AttributeName name, qint64 default_) const
{
  return m_attributes.value(name, default_).toLongLong();
}

double InterfaceItem::getAttributeDouble(AttributeName name, double default_) const
{
  return m_attributes.value(name, default_).toDouble();
}

QString InterfaceItem::getAttributeString(AttributeName name, const QString& default_) const
{
  return m_attributes.value(name, default_).toString();
}
