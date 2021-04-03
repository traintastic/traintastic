/**
 * client/src/network/client.cpp
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

#include "connection.hpp"
#include <QTcpSocket>
#include <QUrl>
#include <QCryptographicHash>
#include <traintastic/network/message.hpp>
#include "object.hpp"
#include "property.hpp"
#include "unitproperty.hpp"
#include "objectproperty.hpp"
#include "method.hpp"
#include "tablemodel.hpp"
#include "inputmonitor.hpp"
#include "outputkeyboard.hpp"
#include "outputmap.hpp"
#include "board.hpp"
#include <traintastic/enum/interfaceitemtype.hpp>
#include <traintastic/enum/attributetype.hpp>
#include <traintastic/locale/locale.hpp>
//#include <enum/valuetype.hpp>
//#include <enum/propertyflags.hpp>

//Client* Client::instance = nullptr;

Connection::Connection() :
  QObject(),
  m_socket{new QTcpSocket(this)},
  m_state{State::Disconnected},
  m_worldProperty{nullptr},
  m_worldRequestId{invalidRequestId}
{
  connect(m_socket, &QTcpSocket::connected, this, &Connection::socketConnected);
  connect(m_socket, &QTcpSocket::disconnected, this, &Connection::socketDisconnected);
  connect(m_socket, static_cast<void(QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &Connection::socketError);

  m_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

  connect(m_socket, &QTcpSocket::readyRead, this, &Connection::socketReadyRead);
}

Connection::~Connection()
{
 // instance = nullptr;
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
/*
int Connection::createObject(const QString& classId, const QString& id, std::function<void(const ObjectPtr&, Message::ErrorCode)> callback)
{
  std::unique_ptr<Message> request{Message::newRequest(Message::Command::CreateObject)};
  request->write(classId.toLatin1());
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
*/
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

void Connection::releaseObject(Object* object)
{
  Q_ASSERT(object);
  m_objects.remove(object->handle());
  auto event = Message::newEvent(Message::Command::ReleaseObject, sizeof(object->m_handle));
  event->write(object->m_handle);
  send(event);
  object->m_handle = invalidHandle;
}

void Connection::setPropertyBool(Property& property, bool value)
{
  auto event = Message::newEvent(Message::Command::ObjectSetProperty);
  event->write(static_cast<Object*>(property.parent())->m_handle);
  event->write(property.name().toLatin1());
  event->write(ValueType::Boolean);
  event->write(value);
  send(event);
}

void Connection::setPropertyInt64(Property& property, int64_t value)
{
  auto event = Message::newEvent(Message::Command::ObjectSetProperty);
  event->write(static_cast<Object*>(property.parent())->m_handle);
  event->write(property.name().toLatin1());
  event->write(ValueType::Integer);
  event->write(value);
  send(event);
}

void Connection::setPropertyDouble(Property& property, double value)
{
  auto event = Message::newEvent(Message::Command::ObjectSetProperty);
  event->write(static_cast<Object*>(property.parent())->m_handle);
  event->write(property.name().toLatin1());
  event->write(ValueType::Float);
  event->write(value);
  send(event);
}

void Connection::setPropertyString(Property& property, const QString& value)
{
  auto event = Message::newEvent(Message::Command::ObjectSetProperty);
  event->write(static_cast<Object*>(property.parent())->m_handle);
  event->write(property.name().toLatin1());
  event->write(ValueType::String);
  event->write(value.toUtf8());
  send(event);
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
        m_tableModels[tableModel->handle()] = tableModel.data();
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

int Connection::getInputMonitorInputInfo(InputMonitor& inputMonitor)
{
  auto request = Message::newRequest(Message::Command::InputMonitorGetInputInfo);
  request->write(inputMonitor.handle());
  send(request,
    [&inputMonitor](const std::shared_ptr<Message> message)
    {
      uint32_t count = message->read<uint32_t>();
      while(count > 0)
      {
        const uint32_t address = message->read<uint32_t>();
        const QString id = QString::fromUtf8(message->read<QByteArray>());
        const TriState value = message->read<TriState>();
        emit inputMonitor.inputIdChanged(address, id);
        emit inputMonitor.inputValueChanged(address, value);
        count--;
      }

/*
      TableModelPtr tableModel;
      if(!message->isError())
      {
        tableModel = readTableModel(*message);
        m_tableModels[tableModel->handle()] = tableModel.data();
      }
      callback(tableModel, message->errorCode());
*/
    });
  return request->requestId();
}

int Connection::getOutputKeyboardOutputInfo(OutputKeyboard& object)
{
  auto request = Message::newRequest(Message::Command::OutputKeyboardGetOutputInfo);
  request->write(object.handle());
  send(request,
    [&object](const std::shared_ptr<Message> message)
    {
      uint32_t count = message->read<uint32_t>();
      while(count > 0)
      {
        const uint32_t address = message->read<uint32_t>();
        const QString id = QString::fromUtf8(message->read<QByteArray>());
        const TriState value = message->read<TriState>();
        emit object.outputIdChanged(address, id);
        emit object.outputValueChanged(address, value);
        count--;
      }
    });
  return request->requestId();
}

void Connection::setOutputKeyboardOutputValue(OutputKeyboard& object, uint32_t address, bool value)
{
  auto event = Message::newEvent(Message::Command::OutputKeyboardSetOutputValue);
  event->write(object.handle());
  event->write(address);
  event->write(value);
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
    const QString classId = QString::fromLatin1(message.read<QByteArray>());
    if(classId.startsWith(InputMonitor::classIdPrefix))
      obj = std::make_shared<InputMonitor>(sharedFromThis(), handle, classId);
    else if(classId.startsWith(OutputKeyboard::classIdPrefix))
      obj = std::make_shared<OutputKeyboard>(sharedFromThis(), handle, classId);
    else if(classId.startsWith(OutputMap::classIdPrefix))
      obj = std::make_shared<OutputMap>(sharedFromThis(), handle, classId);
    else if(classId == Board::classId)
      obj = std::make_shared<Board>(sharedFromThis(), handle, classId);
    else
      obj = std::make_shared<Object>(sharedFromThis(), handle, classId);
    m_objects[handle] = obj;

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
        {
          const PropertyFlags flags = message.read<PropertyFlags>();
          const ValueType valueType = message.read<ValueType>();
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
              value = QString::fromLatin1(message.read<QByteArray>());
              break;

            case ValueType::Invalid:
              break;
          }



    //      Q_ASSERT(value.isValid());
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
              if(valueType == ValueType::Enum)
                p->m_enumName = QString::fromLatin1(message.read<QByteArray>());
              item = p;
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
      }

      if(Q_LIKELY(item))
      {
        message.readBlock(); // attributes
        while(!message.endOfBlock())
        {
          message.readBlock(); // item
          const AttributeName attributeName = message.read<AttributeName>();
          const ValueType type = message.read<ValueType>();

          switch(message.read<AttributeType>())
          {
            case AttributeType::Value:
            {
              QVariant value;
              switch(type)
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
              QList<QVariant> values;
              switch(type)
              {
                case ValueType::Boolean:
                {
                  for(int i = 0; i < length; i++)
                    values.append(message.read<bool>());
                  break;
                }
                case ValueType::Enum:
                case ValueType::Integer:
                {
                  for(int i = 0; i < length; i++)
                    values.append(message.read<qint64>());
                  break;
                }
                case ValueType::Float:
                {
                  for(int i = 0; i < length; i++)
                    values.append(message.read<double>());
                  break;
                }
                case ValueType::String:
                {
                  for(int i = 0; i < length; i++)
                    values.append(QString::fromUtf8(message.read<QByteArray>()));
                  break;
                }
                case ValueType::Object:
                case ValueType::Invalid:
                default:
                  Q_ASSERT(false);
                  break;
              }
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
  }

  message.readBlockEnd(); // end object

  return obj;
}

TableModelPtr Connection::readTableModel(const Message& message)
{
  message.readBlock(); // model
  const Handle handle = message.read<Handle>();
  const QString classId = QString::fromLatin1(message.read<QByteArray>());
  TableModelPtr tableModel = TableModelPtr::create(sharedFromThis(), handle, classId);
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
    cancelRequest(m_worldRequestId);

  if(m_worldProperty->objectId().isEmpty())
    setWorld(nullptr);
  else
    m_worldRequestId = getObject(m_worldProperty->objectId(),
      [this](const ObjectPtr& object, Message::ErrorCode ec)
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
      case Message::Command::ObjectPropertyChanged:
        if(ObjectPtr object = m_objects.value(message->read<Handle>()).lock())
        {
          if(AbstractProperty* property = object->getProperty(QString::fromLatin1(message->read<QByteArray>())))
          {
            const ValueType valueType = message->read<ValueType>();
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

            /*
            switch(message->read<ValueType>())
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
              case ValueType::Invalid:
                Q_ASSERT(false);
                break;
            }

            if(Q_LIKELY(value.isValid()))
            {
              item->m_attributes[attributeName] = value;
              emit item->attributeChanged(attributeName, value);
            }
            */
          }
        }
        break;

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

      case Message::Command::InputMonitorInputIdChanged:
        if(auto inputMonitor = std::dynamic_pointer_cast<InputMonitor>(m_objects.value(message->read<Handle>()).lock()))
        {
          const uint32_t address = message->read<uint32_t>();
          const QString id = QString::fromUtf8(message->read<QByteArray>());
          emit inputMonitor->inputIdChanged(address, id);
        }
        break;

      case Message::Command::InputMonitorInputValueChanged:
        if(auto inputMonitor = std::dynamic_pointer_cast<InputMonitor>(m_objects.value(message->read<Handle>()).lock()))
        {
          const uint32_t address = message->read<uint32_t>();
          const TriState value = message->read<TriState>();
          emit inputMonitor->inputValueChanged(address, value);
        }
        break;

      case Message::Command::OutputKeyboardOutputIdChanged:
        if(auto outputKeyboard = std::dynamic_pointer_cast<OutputKeyboard>(m_objects.value(message->read<Handle>()).lock()))
        {
          const uint32_t address = message->read<uint32_t>();
          const QString id = QString::fromUtf8(message->read<QByteArray>());
          emit outputKeyboard->outputIdChanged(address, id);
        }
        break;

      case Message::Command::OutputKeyboardOutputValueChanged:
        if(auto outputKeyboard = std::dynamic_pointer_cast<OutputKeyboard>(m_objects.value(message->read<Handle>()).lock()))
        {
          const uint32_t address = message->read<uint32_t>();
          const TriState value = message->read<TriState>();
          emit outputKeyboard->outputValueChanged(address, value);
        }
        break;

      case Message::Command::BoardTileDataChanged:
        if(auto board = std::dynamic_pointer_cast<Board>(m_objects.value(message->read<Handle>()).lock()))
          board->processMessage(*message);
        break;

      case Message::Command::OutputMapOutputsChanged:
        if(auto outputMap = std::dynamic_pointer_cast<OutputMap>(m_objects.value(message->read<Handle>()).lock()))
          outputMap->processMessage(*message);
        break;

      default:
        Q_ASSERT(false);
        break;
    }
  }
}

void Connection::socketConnected()
{
  std::unique_ptr<Message> request{Message::newRequest(Message::Command::Login)};
  request->write(m_username.toUtf8());
  request->write(m_password);
  send(request,
    [this](const std::shared_ptr<Message> message)
    {
      if(message && message->isResponse() && !message->isError())
      {
        std::unique_ptr<Message> request{Message::newRequest(Message::Command::NewSession)};
        send(request,
          [this](const std::shared_ptr<Message> message)
          {
            if(message && message->isResponse() && !message->isError())
            {
              message->read(m_sessionUUID);
              m_traintastic = readObject(*message);
              m_worldProperty = dynamic_cast<ObjectProperty*>(m_traintastic->getProperty("world"));
              connect(m_worldProperty, &ObjectProperty::valueChanged,
                [this]()
                {
                  getWorld();
                  /*if(!m_worldProperty->objectId().isEmpty())
                  {
                    if(m_worldRequestId != invalidRequestId)
                      cancelRequest(m_worldRequestId);

                    m_worldRequestId = getObject(m_worldProperty->objectId(),
                      [this](const ObjectPtr& object, Message::ErrorCode ec)
                      {
                        m_worldRequestId = invalidRequestId;
                        setWorld(object);
                        // TODO: show error??
                      });
                  }
                  else
                    setWorld(nullptr);*/
                });

              if(!m_worldProperty->objectId().isEmpty())
                getWorld();
              else
                setState(State::Connected);


/*
              getObject("traintastic",
                [this](const ObjectPtr& object, Message::ErrorCode errorCode)
                {
                  if(object && errorCode == Message::ErrorCode::None)
                  {
                    m_traintastic = object;
                    m_worldProperty = object->getProperty("world");
                    if(!m_worldProperty)
                    {
                      Q_ASSERT(false);
                      disconnectFromServer();
                    }
                    m_worldProperty->getObject(
                      [this](const ObjectPtr& object, Message::ErrorCode ec)
                      {
                        if(!ec)
                        {
                          if(object)
                        }

                      });





                    setState(State::Connected);
                  }
                });
                */
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
