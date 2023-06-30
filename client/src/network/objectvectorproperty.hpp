/**
 * client/src/network/objectvectorproperty.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_OBJECTVECTORPROPERTY_HPP
#define TRAINTASTIC_CLIENT_NETWORK_OBJECTVECTORPROPERTY_HPP

#include "abstractvectorproperty.hpp"
#include "objectptr.hpp"
#include <traintastic/network/message.hpp>

class ObjectVectorProperty : public AbstractVectorProperty
{
  friend class Connection;

  protected:
    QStringList m_ids;

  public:
    using const_iterator = QStringList::const_iterator;

    ObjectVectorProperty(Object& object, const QString& name, PropertyFlags flags, QStringList ids) :
      AbstractVectorProperty(object, name, ValueType::Object, flags),
      m_ids{std::move(ids)}
    {
    }

    inline const_iterator begin() const { return m_ids.begin(); }
    inline const_iterator end() const { return m_ids.end(); }

    int size() const final
    {
      return m_ids.size();
    }

    [[nodiscard]] int getObject(int index, std::function<void(const ObjectPtr&, Message::ErrorCode)> callback);
    [[nodiscard]] int getObjects(int startIndex, int endIndex, std::function<void(const std::vector<ObjectPtr>&, Message::ErrorCode)> callback);
    [[nodiscard]] int getObjects(std::function<void(const std::vector<ObjectPtr>&, Message::ErrorCode)> callback)
    {
      return getObjects(0, size() - 1, std::move(callback));
    }

    const QString& getObjectId(int index) const
    {
      Q_ASSERT(index >= 0 && index < size());
      return m_ids[index];
    }
};

#endif
