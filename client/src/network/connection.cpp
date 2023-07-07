/**
 * client/src/network/client.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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

#include "connection.hpp"
#include <QTcpSocket>
#include <QUrl>
#include <QCryptographicHash>
#include <traintastic/network/message.hpp>
#include "serverlogtablemodel.hpp"
#include "object.hpp"
#include "property.hpp"
#include "unitproperty.hpp"
#include "objectproperty.hpp"
#include "vectorproperty.hpp"
#include "objectvectorproperty.hpp"
#include "method.hpp"
#include "event.hpp"
#include "tablemodel.hpp"
#include "createobject.hpp"
#include "board.hpp"
#include <traintastic/enum/interfaceitemtype.hpp>
#include <traintastic/enum/attributetype.hpp>
#include <traintastic/locale/locale.hpp>

inline static QVariant readValue(const Message& message, const ValueType valueType)
{
  switch(valueType)
  {
    case ValueType::Boolean:
      return message.read<bool>();

    case ValueType::Enum:
    case ValueType::Integer:
    case ValueType::Set:
      return message.read<qint64>();

    case ValueType::Float:
      return message.read<double>();

    case ValueType::String:
      return QString::fromUtf8(message.read<QByteArray>());

    case ValueType::Object:
      return QString::fromLatin1(message.read<QByteArray>());

    case ValueType::Invalid:
      break;
  }

  Q_ASSERT(false);
  return QVariant();
}

inline static QList<QVariant> readArray(const Message& message, const ValueType valueType, const int length)
{
  Q_ASSERT(length >= 0);

  QList<QVariant> values;
  values.reserve(length);

  switch(valueType)
  {
    case ValueType::Boolean:
      for(int i = 0; i < length; i++)
        values.append(message.read<bool>());
      break;

    case ValueType::Enum:
    case ValueType::Integer:
    case ValueType::Set:
      for(int i = 0; i < length; i++)
        values.append(message.read<qint64>());
      break;

    case ValueType::Float:
      for(int i = 0; i < length; i++)
        values.append(message.read<double>());
      break;

    case ValueType::String:
      for(int i = 0; i < length; i++)
        values.append(QString::fromUtf8(message.read<QByteArray>()));
      break;

    case ValueType::Object:
    case ValueType::Invalid:
      break;
  }

  Q_ASSERT(values.size() == length);

  return values;
}

inline static QStringList readObjectIdArray(const Message& message, const int length)
{
  Q_ASSERT(length >= 0);

  QStringList values;
  values.reserve(length);

  for(int i = 0; i < length; i++)
    values.append(QString::fromLatin1(message.read<QByteArray>()));

  Q_ASSERT(values.size() == length);

  return values;
}


Connection::Connection() :
  QObject(),
  m_socket{new QTcpSocket(this)},
  m_state{State::Disconnected},
  m_worldProperty{nullptr},
  m_worldRequestId{invalidRequestId}
  , m_serverLogTableModel{nullptr}
{
  connect(m_socket, &QTcpSocket::connected, this, &Connection::socketConnected);
  connect(m_socket, &QTcpSocket::disconnected, this, &Connection::socketDisconnected);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  connect(m_socket, static_cast<void(QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &Connection::socketError);
#else
  connect(m_socket, &QTcpSocket::errorOccurred, this, &Connection::socketError);
#endif

  m_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

  connect(m_socket, &QTcpSocket::readyRead, this, &Connection::socketReadyRead);
}

bool Connection::isDisconnected() const
{
  return m_state != State::Connected && m_state != State::Connecting && m_state != State::Disconnecting;
}

Connection::SocketError Connection::error() const
{
  return m_socket->error();
}

QString Connection::errorString() const
{
  return m_socket->errorString();
}

void Connection::connectToHost(const QUrl& url, const QString& username, const QString& password)
{
  m_username = username;
  if(password.isEmpty())
    m_password.clear();
  else
    m_password = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
  setState(State::Connecting);
  m_socket->connectToHost(url.host(), static_cast<quint16>(url.port(defaultPort)));
}

void Connection::disconnectFromHost()
{
  m_socket->disconnectFromHost();
}

void Connection::cancelRequest(int requestId)
{
  if(requestId < std::numeric_limits<uint16_t>::min() || requestId > std::numeric_limits<uint16_t>::max())
    return;

  auto it = m_requestCallback.find(static_cast<uint16_t>(requestId));
  if(it != m_requestCallback.end())
    m_requestCallback.erase(it);
}

QString Connection::worldUUID() const
{
  if(!m_world)
    return QString();

  auto* p = m_world->getProperty("uuid");
  if(!p)
    return QString();

  return p->toString();
}

void Connection::serverLog(ServerLogTableModel& model, bool enable)
{
  assert(enable || m_serverLogTableModel == &model);
  m_serverLogTableModel = enable ? &model : nullptr;

  std::unique_ptr<Message> request{Message::newEvent(Message::Command::ServerLog)};
  request->write(enable);
  send(request);
}

int Connection::getObject(const QString& id, std::function<void(const ObjectPtr&, Message::ErrorCode)> callback)
{
  std::unique_ptr<Message> request{Message::newRequest(Message::Command::GetObject)};
  request->write(id.toLatin1());
  send(request,
    [this, callback](const std::shared_ptr<Message> message)
    {
      ObjectPtr object;
      if(!message->isError())
        object = readObject(*message);
      callback(object, message->errorCode());
    });
  return request->requestId();
}

int Connection::getObject(const ObjectProperty& property, std::function<void(const ObjectPtr&, Message::ErrorCode)> callback)
{
  std::unique_ptr<Message> request{Message::newRequest(Message::Command::ObjectGetObjectPropertyObject)};
  request->write(property.object().handle());
  request->write(property.name().toLatin1());
  send(request,
    [this, callback](const std::shared_ptr<Message> message)
    {
      ObjectPtr object;
      if(!message->isError())
        object = readObject(*message);
      callback(object, message->errorCode());
    });
  return request->requestId();
}

int Connection::getObject(const ObjectVectorProperty& property, uint32_t index, std::function<void(const ObjectPtr&, Message::ErrorCode)> callback)
{
  std::unique_ptr<Message> request{Message::newRequest(Message::Command::ObjectGetObjectVectorPropertyObject)};
  request->write(property.object().handle());
  request->write(property.name().toLatin1());
  request->write(index); // start index
  request->write(index); // end index
  send(request,
    [this, callback](const std::shared_ptr<Message> message)
    {
      ObjectPtr object;
      if(!message->isError())
        object = readObject(*message);
      callback(object, message->errorCode());
    });
  return request->requestId();
}

int Connection::getObjects(const ObjectVectorProperty& property, uint32_t startIndex, uint32_t endIndex, std::function<void(const std::vector<ObjectPtr>&, Message::ErrorCode)> callback)
{
  std::unique_ptr<Message> request{Message::newRequest(Message::Command::ObjectGetObjectVectorPropertyObject)};
  request->write(property.object().handle());
  request->write(property.name().toLatin1());
  request->write(startIndex);
  request->write(endIndex);
  send(request,
    [this, size=(endIndex - startIndex + 1), callback](const std::shared_ptr<Message> message)
    {
      std::vector<ObjectPtr> objects;
      objects.reserve(size);
      if(!message->isError())
        for(uint32_t i = 0; i < size; i++)
          objects.emplace_back(readObject(*message));
      callback(objects, message->errorCode());
    });
  return request->requestId();
}

void Connection::setUnitPropertyUnit(UnitProperty& property, int64_t value)
{
  auto event = Message::newEvent(Message::Command::ObjectSetUnitPropertyUnit);
  event->write(static_cast<Object*>(property.parent())->m_handle);
  event->write(property.name().toLatin1());
  event->write(value);
  send(event);
}

void Connection::setObjectPropertyById(const ObjectProperty& property, const QString& value)
{
  auto event = Message::newEvent(Message::Command::ObjectSetObjectPropertyById);
  event->write(static_cast<Object*>(property.parent())->m_handle);
  event->write(property.name().toLatin1());
  event->write(value.toLatin1());
  send(event);
}

void Connection::callMethod(Method& method)
{
  auto event = Message::newEvent(Message::Command::ObjectCallMethod);
  event->write(method.object().handle());
  event->write(method.name().toLatin1());
  event->write(ValueType::Invalid); // no result
  event->write<uint8_t>(0); // no arguments
  send(event);
}

void Connection::callMethod(Method& method, const QString& arg)
{
  auto event = Message::newEvent(Message::Command::ObjectCallMethod);
  event->write(method.object().handle());
  event->write(method.name().toLatin1());
  event->write(ValueType::Invalid); // no result
  event->write<uint8_t>(1); // 1 argument
  event->write(ValueType::String);
  event->write(arg.toUtf8());
  send(event);
}

int Connection::callMethod(Method& method, std::function<void(const ObjectPtr&, Message::ErrorCode)> callback)
{
  auto request = Message::newRequest(Message::Command::ObjectCallMethod);
  request->write(method.object().handle());
  request->write(method.name().toLatin1());
  request->write(ValueType::Object); // object result
  request->write<uint8_t>(0); // no arguments
  send(request,
    [this, callback](const std::shared_ptr<Message> message)
    {
      ObjectPtr object;
      if(!message->isError())
        object = readObject(*message);
      callback(object, message->errorCode());
    });
  return request->requestId();
}

int Connection::callMethod(Method& method, const QString& arg, std::function<void(const ObjectPtr&, Message::ErrorCode)> callback)
{
  auto request = Message::newRequest(Message::Command::ObjectCallMethod);
  request->write(method.object().handle());
  request->write(method.name().toLatin1());
  request->write(ValueType::Object); // object result
  request->write<uint8_t>(1); // 1 argument
  request->write(ValueType::String);
  request->write(arg.toUtf8());
  send(request,
    [this, callback](const std::shared_ptr<Message> message)
    {
      ObjectPtr object;
      if(!message->isError())
        object = readObject(*message);
      callback(object, message->errorCode());
    });
  return request->requestId();
}

int Connection::getTableModel(const ObjectPtr& object, std::function<void(const TableModelPtr&, Message::ErrorCode)> callback)
{
  std::unique_ptr<Message> request{Message::newRequest(Message::Command::GetTableModel)};
  request->write(object->handle());
  send(request,
    [this, callback](const std::shared_ptr<Message> message)
    {
      TableModelPtr tableModel;
      if(!message->isError())
      {
        tableModel = readTableModel(*message);
        m_tableModels[tableModel->handle()] = tableModel.get();
      }
      callback(tableModel, message->errorCode());
    });
  return request->requestId();
}

void Connection::releaseTableModel(TableModel* tableModel)
{
  Q_ASSERT(tableModel);
  m_tableModels.remove(tableModel->handle());
  auto event = Message::newEvent(Message::Command::ReleaseTableModel, sizeof(tableModel->m_handle));
  event->write(tableModel->m_handle);
  send(event);
  tableModel->m_handle = invalidHandle;
}

void Connection::setTableModelRegion(TableModel* tableModel, int columnMin, int columnMax, int rowMin, int rowMax)
{
  auto event = Message::newEvent(Message::Command::TableModelSetRegion);
  event->write(tableModel->handle());
  event->write(columnMin);
  event->write(columnMax);
  event->write(rowMin);
  event->write(rowMax);
  send(event);
}

int Connection::getTileData(Board& object)
{
  auto request = Message::newRequest(Message::Command::BoardGetTileData);
  request->write(object.handle());
  send(request,
    [&object](const std::shared_ptr<Message> message)
    {
      object.getTileDataResponse(*message);
    });
  return request->requestId();
}

void Connection::send(std::unique_ptr<Message>& message)
{
  Q_ASSERT(!message->isRequest());
  m_socket->write(static_cast<const char*>(**message), message->size());
}

void Connection::send(std::unique_ptr<Message>& message, std::function<void(const std::shared_ptr<Message>&)> callback)
{
  Q_ASSERT(message->isRequest());
  Q_ASSERT(!m_requestCallback.contains(message->requestId()));
  m_requestCallback[message->requestId()] = callback;
  m_socket->write(static_cast<const char*>(**message), message->size());
}

ObjectPtr Connection::readObject(const Message& message)
{
  message.readBlock(); // object

  const Handle handle = message.read<Handle>();

  ObjectPtr obj = m_objects.value(handle).lock(); // try get object by handle
  if(!obj)
  {
    {
      Object* p;

      if(auto it = m_requestForRelease.find(handle); it != m_requestForRelease.end())
      {
        // object not yet released, reuse it.
        p = it->second.release();
        m_requestForRelease.erase(it);
        m_handleCounter[handle]++;
      }
      else
      {
        p = createObject(shared_from_this(), handle, QString::fromLatin1(message.read<QByteArray>()));
        m_handleCounter[handle] = 1;
      }

      obj = std::shared_ptr<Object>(p,
        [this](Object* object)
        {
          // try to release object, if it succeeds the server send a ReleaseObject message else nothing
          // release will fail if handle counters don't match, which means that the is a handle "on the wire"
          m_objects.remove(object->m_handle);
          m_requestForRelease.emplace(object->m_handle, std::unique_ptr<Object>(object));

          auto event = Message::newEvent(Message::Command::ReleaseObject, sizeof(object->m_handle));
          event->write(object->m_handle);
          event->write(m_handleCounter[object->m_handle]);
          send(event);
        });
    }
    m_objects[handle] = obj;

    if(m_handleCounter[handle] > 1) // object was still in memory
      return obj;

    message.readBlock(); // items
    while(!message.endOfBlock())
    {
      message.readBlock(); // item
      InterfaceItem* item = nullptr;
      const QString name = QString::fromLatin1(message.read<QByteArray>());
      const InterfaceItemType type = message.read<InterfaceItemType>();
      switch(type)
      {
        case InterfaceItemType::Property:
        case InterfaceItemType::UnitProperty:
        case InterfaceItemType::VectorProperty:
        {
          const PropertyFlags flags = message.read<PropertyFlags>();
          const ValueType valueType = message.read<ValueType>();

          QString enumOrSetName;
          if(valueType == ValueType::Enum || valueType == ValueType::Set)
            enumOrSetName = QString::fromLatin1(message.read<QByteArray>());

          if(type == InterfaceItemType::VectorProperty)
          {
            const int length = message.read<int>(); // read uint32_t as int, Qt uses int for length

            if(valueType == ValueType::Object)
            {
              item = new ObjectVectorProperty(*obj, name, flags, readObjectIdArray(message, length));
            }
            else
            {
              VectorProperty* p = new VectorProperty(*obj, name, valueType, flags, readArray(message, valueType, length));
              if(valueType == ValueType::Enum || valueType == ValueType::Set)
                p->m_enumOrSetName = enumOrSetName;
              item = p;
            }
          }
          else
          {
            Q_ASSERT(type == InterfaceItemType::Property || type == InterfaceItemType::UnitProperty);

            QVariant value = readValue(message, valueType);

            if(Q_LIKELY(value.isValid()))
            {
              if(type == InterfaceItemType::UnitProperty)
              {
                QString unitName = QString::fromLatin1(message.read<QByteArray>());
                qint64 unitValue = message.read<qint64>();
                item = new UnitProperty(*obj, name, valueType, flags, value, unitName, unitValue);
              }
              else if(valueType == ValueType::Object)
              {
                item = new ObjectProperty(*obj, name, flags, value.toString());
              }
              else
              {
                Property* p = new Property(*obj, name, valueType, flags, value);
                if(valueType == ValueType::Enum || valueType == ValueType::Set)
                  p->m_enumOrSetName = enumOrSetName;
                item = p;
              }
            }
          }
          break;
        }
        case InterfaceItemType::Method:
        {
          const ValueType resultType = message.read<ValueType>();
          const uint8_t argumentCount = message.read<uint8_t>();
          QVector<ValueType> argumentTypes;
          for(uint8_t i = 0; i < argumentCount; i++)
            argumentTypes.append(message.read<ValueType>());
          item = new Method(*obj, name, resultType, argumentTypes);
          break;
        }
        case InterfaceItemType::Event:
        {
          const uint8_t argumentCount = message.read<uint8_t>();
          std::vector<ValueType> argumentTypes;
          for(uint8_t i = 0; i < argumentCount; i++)
          {
            const auto argumentType = message.read<ValueType>();
            argumentTypes.emplace_back(argumentType);
            if(argumentType == ValueType::Enum || argumentType == ValueType::Set)
              message.read<QByteArray>(); // enum/set type, currently unused
          }
          item = new Event(*obj, name, std::move(argumentTypes));
          break;
        }
      }

      if(Q_LIKELY(item))
      {
        message.readBlock(); // attributes
        while(!message.endOfBlock())
        {
          message.readBlock(); // item
          const AttributeName attributeName = message.read<AttributeName>();
          const ValueType valueType = message.read<ValueType>();

          switch(message.read<AttributeType>())
          {
            case AttributeType::Value:
            {
              QVariant value;
              switch(valueType)
              {
                case ValueType::Boolean:
                  value = message.read<bool>();
                  break;

                case ValueType::Enum:
                case ValueType::Integer:
                  value = message.read<qint64>();
                  break;

                case ValueType::Float:
                  value = message.read<double>();
                  break;

                case ValueType::String:
                  value = QString::fromUtf8(message.read<QByteArray>());
                  break;

                case ValueType::Object:
                case ValueType::Invalid:
                default:
                  Q_ASSERT(false);
                  break;
              }
              if(Q_LIKELY(value.isValid()))
                item->m_attributes[attributeName] = value;
              break;
            }
            case AttributeType::Values:
            {
              const int length = message.read<int>(); // read uint32_t as int, Qt uses int for length
              QList<QVariant> values = readArray(message, valueType, length);
              if(Q_LIKELY(values.length() == length))
                item->m_attributes[attributeName] = values;
              break;
            }

            default:
              Q_ASSERT(false);
          }
          message.readBlockEnd(); // end attribute
        }
        message.readBlockEnd(); // end attributes

        obj->m_interfaceItems.add(*item);
      }
      message.readBlockEnd(); // end item
    }
    message.readBlockEnd(); // end items

    obj->created();
  }
  else
    m_handleCounter[handle]++;

  message.readBlockEnd(); // end object

  return obj;
}

TableModelPtr Connection::readTableModel(const Message& message)
{
  message.readBlock(); // model
  const Handle handle = message.read<Handle>();
  const QString classId = QString::fromLatin1(message.read<QByteArray>());
  TableModelPtr tableModel = std::make_shared<TableModel>(shared_from_this(), handle, classId);
  const int columnCount = message.read<int>();
  for(int i = 0; i < columnCount; i++)
    tableModel->m_columnHeaders.push_back(Locale::tr(QString::fromLatin1(message.read<QByteArray>())));
  Q_ASSERT(tableModel->m_columnHeaders.size() == columnCount);
  message.read(tableModel->m_rowCount);
  message.readBlockEnd(); // end model
  return tableModel;
}

void Connection::getWorld()
{
  if(m_worldRequestId != invalidRequestId)
    return;

  if(!m_worldProperty->hasObject())
    setWorld(nullptr);
  else
    m_worldRequestId = m_worldProperty->getObject(
      [this](const ObjectPtr& object, Message::ErrorCode /*ec*/)
      {
        m_worldRequestId = invalidRequestId;
        setWorld(object);
        // TODO: show error??
        if(m_state == State::Connecting)
          setState(State::Connected);
      });
}

void Connection::setWorld(const ObjectPtr& world)
{
  if(m_world != world)
  {
    m_world = world;
    emit worldChanged();
  }
}

void Connection::setState(State state)
{
  if(m_state != state)
  {
    m_state = state;
    emit stateChanged();
  }
}

void Connection::processMessage(const std::shared_ptr<Message> message)
{
  if(message->isResponse())
  {
    auto it = m_requestCallback.find(message->requestId());
    if(it != m_requestCallback.end())
    {
      it.value()(message);
      m_requestCallback.erase(it);
    }
  }
  else if(message->isEvent())
  {
    switch(message->command())
    {
      case Message::Command::ServerLog:
        if(m_serverLogTableModel)
          m_serverLogTableModel->processMessage(*message);
        break;

      case Message::Command::ReleaseObject:
      {
        Handle handle = message->read<Handle>();
        m_handleCounter.erase(handle);
        m_requestForRelease.erase(handle);
        break;
      }
      case Message::Command::ObjectPropertyChanged:
        if(ObjectPtr object = m_objects.value(message->read<Handle>()).lock())
        {
          const QString name = QString::fromLatin1(message->read<QByteArray>());
          const ValueType valueType = message->read<ValueType>();

          if(AbstractProperty* property = object->getProperty(name))
          {
            switch(valueType)
            {
              case ValueType::Boolean:
              {
                const bool value = message->read<bool>();
                static_cast<Property*>(property)->m_value = value;
                emit property->valueChanged();
                emit property->valueChangedBool(value);
                break;
              }
              case ValueType::Integer:
              case ValueType::Enum:
              case ValueType::Set:
              {
                const qlonglong value = message->read<qlonglong>();
                static_cast<Property*>(property)->m_value = value;
                if(valueType == ValueType::Integer)
                  if(UnitProperty* unitProperty = dynamic_cast<UnitProperty*>(property))
                    unitProperty->m_unitValue = message->read<qint64>();
                emit property->valueChanged();
                emit property->valueChangedInt64(value);
                if(value >= std::numeric_limits<int>::min() && value <= std::numeric_limits<int>::max())
                  emit property->valueChangedInt(static_cast<int>(value));
                break;
              }
              case ValueType::Float:
              {
                const double value = message->read<double>();
                static_cast<Property*>(property)->m_value = value;
                if(UnitProperty* unitProperty = dynamic_cast<UnitProperty*>(property))
                  unitProperty->m_unitValue = message->read<qint64>();
                emit property->valueChanged();
                emit property->valueChangedDouble(value);
                break;
              }
              case ValueType::String:
              {
                const QString value = QString::fromUtf8(message->read<QByteArray>());
                static_cast<Property*>(property)->m_value = value;
                emit property->valueChanged();
                emit property->valueChangedString(value);
                break;
              }
              case ValueType::Object:
              {
                const QString id = QString::fromLatin1(message->read<QByteArray>());
                static_cast<ObjectProperty*>(property)->m_id = id;
                emit property->valueChanged();
                break;
              }
              case ValueType::Invalid:
                Q_ASSERT(false);
                break;
            }
          }
          else if(AbstractVectorProperty* vectorProperty = object->getVectorProperty(name))
          {
            const int length = message->read<int>(); // read uint32_t as int, Qt uses int for length

            if(valueType == ValueType::Object)
              static_cast<ObjectVectorProperty*>(vectorProperty)->m_ids = readObjectIdArray(*message, length);
            else
              static_cast<VectorProperty*>(vectorProperty)->m_values = readArray(*message, valueType, length);

            emit vectorProperty->valueChanged();
          }
        }
        break;

      case Message::Command::ObjectAttributeChanged:
        if(ObjectPtr object = m_objects.value(message->read<Handle>()).lock())
        {
          if(InterfaceItem* item = object->getInterfaceItem(QString::fromLatin1(message->read<QByteArray>())))
          {
            AttributeName attributeName = message->read<AttributeName>();
            const ValueType type = message->read<ValueType>();
            switch(message->read<AttributeType>())
            {
              case AttributeType::Value:
              {
                QVariant value;
                switch(type)
                {
                  case ValueType::Boolean:
                    value = message->read<bool>();
                    break;

                  case ValueType::Integer:
                  case ValueType::Enum:
                    value = message->read<qlonglong>();
                    break;

                  case ValueType::Float:
                    value = message->read<double>();
                    break;

                  case ValueType::String:
                    value = QString::fromUtf8(message->read<QByteArray>());
                    break;

                  case ValueType::Object:
                  case ValueType::Set:
                  case ValueType::Invalid:
                    Q_ASSERT(false);
                    break;
                }

                if(Q_LIKELY(value.isValid()))
                {
                  item->m_attributes[attributeName] = value;
                  emit item->attributeChanged(attributeName, value);
                }
                break;
              }
              case AttributeType::Values:
              {
                const int length = message->read<int>(); // read uint32_t as int, Qt uses int for length
                QList<QVariant> values;
                switch(type)
                {
                  case ValueType::Boolean:
                  {
                    for(int i = 0; i < length; i++)
                      values.append(message->read<bool>());
                    break;
                  }
                  case ValueType::Enum:
                  case ValueType::Integer:
                  {
                    for(int i = 0; i < length; i++)
                      values.append(message->read<qint64>());
                    break;
                  }
                  case ValueType::Float:
                  {
                    for(int i = 0; i < length; i++)
                      values.append(message->read<double>());
                    break;
                  }
                  case ValueType::String:
                  {
                    for(int i = 0; i < length; i++)
                      values.append(QString::fromUtf8(message->read<QByteArray>()));
                    break;
                  }
                  case ValueType::Object:
                  case ValueType::Set:
                  case ValueType::Invalid:
                    Q_ASSERT(false);
                    break;
                }
                if(Q_LIKELY(values.length() == length))
                {
                  item->m_attributes[attributeName] = values;
                  emit item->attributeChanged(attributeName, values);
                }
                break;
              }

              default:
                Q_ASSERT(false);
            }
          }
        }
        break;

      case Message::Command::ObjectDestroyed:
      {
        //const Handle handle = message->read<Handle>();
        break;
      }

      case Message::Command::TableModelColumnHeadersChanged:
        if(TableModel* model = m_tableModels.value(message->read<Handle>(), nullptr))
        {
          const int columnCount = message->read<int>();
          model->m_columnHeaders.clear();
          for(int i = 0; i < columnCount; i++)
            model->m_columnHeaders.push_back(QString::fromUtf8(message->read<QByteArray>()));
          Q_ASSERT(model->m_columnHeaders.size() == columnCount);
        }
        break;

      case Message::Command::TableModelRowCountChanged:
        if(TableModel* model = m_tableModels.value(message->read<Handle>(), nullptr))
          model->setRowCount(message->read<int>());
        break;

      case Message::Command::TableModelUpdateRegion:
        if(TableModel* model = m_tableModels.value(message->read<Handle>(), nullptr))
        {
          const uint32_t columnMin = message->read<uint32_t>();
          const uint32_t columnMax = message->read<uint32_t>();
          const uint32_t rowMin = message->read<uint32_t>();
          const uint32_t rowMax = message->read<uint32_t>();

          model->beginResetModel();

          TableModel::ColumnRow index;
          QByteArray data;
          for(index.second = rowMin; index.second <= rowMax; index.second++)
            for(index.first = columnMin; index.first <= columnMax; index.first++)
            {
              message->read(data);
              model->m_texts[index] = Locale::instance->parse(QString::fromUtf8(data));
            }

          model->endResetModel();
        }
        break;

      case Message::Command::ObjectEventFired:
      case Message::Command::InputMonitorInputIdChanged:
      case Message::Command::InputMonitorInputValueChanged:
      case Message::Command::OutputKeyboardOutputIdChanged:
      case Message::Command::OutputKeyboardOutputValueChanged:
      case Message::Command::BoardTileDataChanged:
      case Message::Command::OutputMapOutputsChanged:
      {
        const auto handle = message->read<Handle>();
        if(auto object = m_objects.value(handle).lock())
        {
          m_handleCounter[handle]++;
          object->processMessage(*message);
        }
        break;
      }
      default:
        Q_ASSERT(false);
        break;
    }
  }
}

void Connection::socketConnected()
{
  std::unique_ptr<Message> loginRequest{Message::newRequest(Message::Command::Login)};
  loginRequest->write(m_username.toUtf8());
  loginRequest->write(m_password);
  send(loginRequest,
    [this](const std::shared_ptr<Message> loginResponse)
    {
      if(loginResponse && loginResponse->isResponse() && !loginResponse->isError())
      {
        std::unique_ptr<Message> newSessionRequest{Message::newRequest(Message::Command::NewSession)};
        send(newSessionRequest,
          [this](const std::shared_ptr<Message> newSessionResonse)
          {
            if(newSessionResonse && newSessionResonse->isResponse() && !newSessionResonse->isError())
            {
              newSessionResonse->read(m_sessionUUID);
              m_traintastic = readObject(*newSessionResonse);
              m_worldProperty = dynamic_cast<ObjectProperty*>(m_traintastic->getProperty("world"));
              connect(m_worldProperty, &ObjectProperty::valueChanged, this,
                [this]()
                {
                  getWorld();
                });
              {
                auto request{Message::newRequest(Message::Command::BoardGetTileInfo)};

                send(request,
                  [this](const std::shared_ptr<Message> boardGetTileInfoResponse)
                  {
                    if(boardGetTileInfoResponse->isResponse() && !boardGetTileInfoResponse->isError())
                    {
                      Board::tileInfo.clear();
                      const auto count = boardGetTileInfoResponse->read<uint32_t>();
                      for(uint32_t i = 0; i < count; i++)
                      {
                        boardGetTileInfoResponse->readBlock();
                        Board::TileInfo info;
                        info.classId = QString::fromLatin1(boardGetTileInfoResponse->read<QByteArray>());
                        boardGetTileInfoResponse->read(info.tileId);
                        boardGetTileInfoResponse->read(info.rotates);
                        const uint32_t length = boardGetTileInfoResponse->read<uint32_t>();
                        for(uint32_t j = 0; j < length; j++)
                          info.menu.append(QString::fromLatin1(boardGetTileInfoResponse->read<QByteArray>()));
                        Board::tileInfo.emplace_back(info);
                        boardGetTileInfoResponse->readBlockEnd();
                      }
                    }
                  });
              }

              if(m_worldProperty->hasObject())
                getWorld();
              else
                setState(State::Connected);
            }
            else
            {
              setState(State::ErrorNewSessionFailed);
              m_socket->disconnectFromHost();
            }
          });
      }
      else
      {
        setState(State::ErrorAuthenticationFailed);
        m_socket->disconnectFromHost();
      }
    });
}

void Connection::socketDisconnected()
{
  setState(State::Disconnected);
}

void Connection::socketError(QAbstractSocket::SocketError)
{
  setState(State::SocketError);
}

void Connection::socketReadyRead()
{
  while(m_socket->bytesAvailable() != 0)
  {
    if(!m_readBuffer.message) // read header
    {
      m_readBuffer.offset += m_socket->read(reinterpret_cast<char*>(&m_readBuffer.header) + m_readBuffer.offset, sizeof(m_readBuffer.header) - m_readBuffer.offset);
      if(m_readBuffer.offset == sizeof(m_readBuffer.header))
      {
        if(m_readBuffer.header.dataSize != 0)
          m_readBuffer.message = std::make_shared<Message>(m_readBuffer.header);
        else
          processMessage(std::make_shared<Message>(m_readBuffer.header));
        m_readBuffer.offset = 0;
      }
    }
    else // read data
    {
      m_readBuffer.offset += m_socket->read(reinterpret_cast<char*>(m_readBuffer.message->data()) + m_readBuffer.offset, m_readBuffer.message->dataSize() - m_readBuffer.offset);
      if(m_readBuffer.offset == m_readBuffer.message->dataSize())
      {
        processMessage(m_readBuffer.message);
        m_readBuffer.message.reset();
        m_readBuffer.offset = 0;
      }
    }
  }
}
