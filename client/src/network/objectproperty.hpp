/**
 * client/src/network/objectproperty.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_OBJECTPROPERTY_HPP
#define TRAINTASTIC_CLIENT_NETWORK_OBJECTPROPERTY_HPP

#include "abstractproperty.hpp"
#include "objectptr.hpp"
#include <traintastic/network/message.hpp>

struct Error;

class ObjectProperty : public AbstractProperty
{
  friend class Connection;

  protected:
    QString m_id;

  public:
    ObjectProperty(Object& object, const QString& name, PropertyFlags flags, const QString& id);

    bool hasObject() const
    {
      return !m_id.isEmpty();
    }

    [[nodiscard]] int getObject(std::function<void(const ObjectPtr&, std::optional<const Error>)> callback);
    const QString& objectId() const { return m_id; }
    void setByObjectId(const QString& value);
};

#endif
