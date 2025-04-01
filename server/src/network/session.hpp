/**
 * server/src/network/session.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_NETWORK_SESSION_HPP
#define TRAINTASTIC_SERVER_NETWORK_SESSION_HPP

#include <memory>
#include <boost/uuid/uuid.hpp>
#include <boost/signals2/connection.hpp>
#include <traintastic/network/message.hpp>
#include <traintastic/enum/tristate.hpp>
#include "handlelist.hpp"
#include "../core/objectptr.hpp"
#include "../core/tablemodelptr.hpp"
#include "../core/argument.hpp"

class ClientConnection;
class MemoryLogger;
class BaseProperty;
class AbstractProperty;
class AbstractVectorProperty;
class AbstractAttribute;
class AbstractEvent;
class InputMonitor;
class OutputKeyboard;
class Board;
class OutputMap;
struct TypeInfo;
struct TileLocation;
struct TileData;

class Session : public std::enable_shared_from_this<Session>
{
  friend class ClientConnection;

  private:
    static void writePropertyValue(Message& message, const AbstractProperty& property);
    static void writeVectorPropertyValue(Message& message, const AbstractVectorProperty& vectorProperty);
    static void writeAttribute(Message& message, const AbstractAttribute& attribute);
    static void writeTypeInfo(Message& message, const TypeInfo& typeInfo);

    boost::signals2::scoped_connection m_memoryLoggerChanged;

  protected:
    using Handle = uint32_t;
    using Handles = HandleList<Handle, ObjectPtr>;

    std::shared_ptr<ClientConnection> m_connection;
    boost::uuids::uuid m_uuid;
    Handles m_handles;
    std::unordered_multimap<Handle, boost::signals2::scoped_connection> m_objectSignals;

    bool processMessage(const Message& message);

    void writeObject(Message& message, const ObjectPtr& object);
    void writeTableModel(Message& message, const TableModelPtr& model);

    void memoryLoggerChanged(const MemoryLogger& logger, uint32_t added, uint32_t removed);

    void objectDestroying(Object& object);
    void objectPropertyChanged(BaseProperty& property);
    void objectAttributeChanged(AbstractAttribute& attribute);
    void objectEventFired(const AbstractEvent& event, const Arguments& arguments);

    void inputMonitorInputIdChanged(InputMonitor& inputMonitor, uint32_t address, std::string_view id);
    void inputMonitorInputValueChanged(InputMonitor& inputMonitor, uint32_t address, TriState value);

    void boardTileDataChanged(Board& board, const TileLocation& location, const TileData& data);

  public:
    Session(const std::shared_ptr<ClientConnection>& connection);
    ~Session();

    const boost::uuids::uuid& uuid() const { return m_uuid; }
};

#endif
