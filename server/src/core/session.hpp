/**
 * server/src/core/session.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CORE_SESSION_HPP
#define TRAINTASTIC_SERVER_CORE_SESSION_HPP

#include <memory>
#include <boost/uuid/uuid.hpp>
#include <boost/signals2/connection.hpp>
#include <message.hpp>
#include "handlelist.hpp"
#include "objectptr.hpp"
#include "tablemodelptr.hpp"

class Client;
class AbstractProperty;
class AbstractAttribute;

class Session : public std::enable_shared_from_this<Session>
{
  friend class Client;

  protected:
    using Handle = uint32_t;
    using Handles = HandleList<Handle, ObjectPtr>;

    std::shared_ptr<Client> m_client;
    boost::uuids::uuid m_uuid;
    Handles m_handles;
    std::unordered_map<Handle, boost::signals2::connection> m_propertyChanged;
    std::unordered_map<Handle, boost::signals2::connection> m_attributeChanged;

    bool processMessage(const Message& message);

    void writeObject(Message& message, const ObjectPtr& object);
    void writeTableModel(Message& message, const TableModelPtr& model);

    void objectPropertyChanged(AbstractProperty& property);
    void objectAttributeChanged(AbstractAttribute& attribute);

  public:
    Session(const std::shared_ptr<Client>& client);

    const boost::uuids::uuid& uuid() const { return m_uuid; }
};

#endif
