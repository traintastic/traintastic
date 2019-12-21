/**
 * client/src/network/client.cpp
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

#include "client.hpp"
#include <QTcpSocket>
#include <QUrl>
#include <QCryptographicHash>
#include <message.hpp>
#include "object.hpp"
#include "property.hpp"
#include "tablemodel.hpp"
#include <enum/interfaceitemtype.hpp>
#include <enum/valuetype.hpp>

Client* Client::instance = nullptr;

Client::Client() :
  QObject(),
  m_socket{new QTcpSocket(this)},
  m_state{State::Disconnected}
{
  connect(m_socket, &QTcpSocket::connected, this, &Client::socketConnected);
  connect(m_socket, &QTcpSocket::disconnected, this, &Client::socketDisconnected);
  connect(m_socket, static_cast<void(QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &Client::socketError);

  m_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

  connect(m_socket, &QTcpSocket::readyRead, this, &Client::socketReadyRead);
}

Client::~Client()
{
}

bool Client::isDisconnected() const
{
  return m_state != State::Connected && m_state != State::Connecting && m_state != State::Disconnecting;
}

Client::SocketError Client::error() const
{
  return m_socket->error();
}

QString Client::errorString() const
{
  return m_socket->errorString();
}

void Client::connectToHost(const QUrl& url, const QString& username, const QString& password)
{  
  m_username = username;
  if(password.isEmpty())
    m_password.clear();
  else
    m_password = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
  setState(State::Connecting);
  m_socket->connectToHost(url.host(), static_cast<quint16>(url.port(defaultPort)));
}

void Client::disconnectFromHost()
{
  m_socket->disconnectFromHost();
}

void Client::cancelRequest(int requestId)
{
  if(requestId < std::numeric_limits<uint16_t>::min() || requestId > std::numeric_limits<uint16_t>::max())
    return;

  auto it = m_requestCallback.find(static_cast<uint16_t>(requestId));
  if(it != m_requestCallback.end())
    m_requestCallback.erase(it);
}

int Client::getObject(const QString& id, std::function<void(const ObjectPtr&, Message::ErrorCode)> callback)
{
  std::unique_ptr<Message> request{Message::newRequest(Message::Command::GetObject)};
  request->write(id.toLatin1());
  send(request,
    [this, callback](const std::shared_ptr<Message> message)
    {
      ObjectPtr object;
      if(!message->isError())
      {
        object = readObject(*message);
        m_objects[object->handle()] = object.data();
      }
      callback(object, message->errorCode());
    });
  return request->requestId();
}

void Client::releaseObject(Object* object)
{
  Q_ASSERT(object);
  m_objects.remove(object->handle());
  auto event = Message::newEvent(Message::Command::ReleaseObject, sizeof(object->m_handle));
  event->write(object->m_handle);
  send(event);
  object->m_handle = invalidHandle;
}

void Client::setPropertyBool(Property& property, bool value)
{
  auto event = Message::newEvent(Message::Command::ObjectSetProperty);
  event->write(static_cast<Object*>(property.parent())->m_handle);
  event->write(property.name().toLatin1());
  event->write(ValueType::Boolean);
  event->write(value);
  send(event);
}

void Client::setPropertyInt64(Property& property, int64_t value)
{
  auto event = Message::newEvent(Message::Command::ObjectSetProperty);
  event->write(static_cast<Object*>(property.parent())->m_handle);
  event->write(property.name().toLatin1());
  event->write(ValueType::Integer);
  event->write(value);
  send(event);
}

void Client::setPropertyDouble(Property& property, double value)
{
  auto event = Message::newEvent(Message::Command::ObjectSetProperty);
  event->write(static_cast<Object*>(property.parent())->m_handle);
  event->write(property.name().toLatin1());
  event->write(ValueType::Float);
  event->write(value);
  send(event);
}

void Client::setPropertyString(Property& property, const QString& value)
{
  auto event = Message::newEvent(Message::Command::ObjectSetProperty);
  event->write(static_cast<Object*>(property.parent())->m_handle);
  event->write(property.name().toLatin1());
  event->write(ValueType::String);
  event->write(value.toUtf8());
  send(event);
}

int Client::getTableModel(const QString& id, std::function<void(const TableModelPtr&, Message::ErrorCode)> callback)
{
  std::unique_ptr<Message> request{Message::newRequest(Message::Command::GetTableModel)};
  request->write(id.toLatin1());
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

void Client::releaseTableModel(TableModel* tableModel)
{
  Q_ASSERT(tableModel);
  m_tableModels.remove(tableModel->handle());
  auto event = Message::newEvent(Message::Command::ReleaseTableModel, sizeof(tableModel->m_handle));
  event->write(tableModel->m_handle);
  send(event);
  tableModel->m_handle = invalidHandle;
}

void Client::setTableModelRegion(TableModel* tableModel, int columnMin, int columnMax, int rowMin, int rowMax)
{
  auto event = Message::newEvent(Message::Command::TableModelSetRegion);
  event->write(tableModel->handle());
  event->write(columnMin);
  event->write(columnMax);
  event->write(rowMin);
  event->write(rowMax);
  send(event);
}

void Client::send(std::unique_ptr<Message>& message)
{
  Q_ASSERT(!message->isRequest());
  m_socket->write(static_cast<const char*>(**message), message->size());
}

void Client::send(std::unique_ptr<Message>& message, std::function<void(const std::shared_ptr<Message>&)> callback)
{
  Q_ASSERT(message->isRequest());
  Q_ASSERT(!m_requestCallback.contains(message->requestId()));
  m_requestCallback[message->requestId()] = callback;
  m_socket->write(static_cast<const char*>(**message), message->size());
}

ObjectPtr Client::readObject(const Message& message)
{
  message.readBlock(); // object

  const Handle handle = message.read<Handle>();

  ObjectPtr obj; // get object by handle
  if(!obj)
  {
    const QString classId = QString::fromLatin1(message.read<QByteArray>());
    obj = QSharedPointer<Object>::create(handle, classId);

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
        {
          const ValueType type = message.read<ValueType>();
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
              // TODO
              break;

            case ValueType::Invalid:
              break;
          }

    //      Q_ASSERT(value.isValid());
          if(Q_LIKELY(value.isValid()))
          {
            Property* p = new Property(*obj, name, type, value);
            if(type == ValueType::Enum)
              p->m_enumName = QString::fromLatin1(message.read<QByteArray>());
            item = p;
          }
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
          QVariant value;
          switch(message.read<ValueType>())
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
              Q_ASSERT(false);
              break;
          }

          if(Q_LIKELY(value.isValid()))
            item->m_attributes[attributeName] = value;

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

TableModelPtr Client::readTableModel(const Message& message)
{
  message.readBlock(); // model
  const Handle handle = message.read<Handle>();
  const QString classId = QString::fromLatin1(message.read<QByteArray>());
  TableModelPtr tableModel = TableModelPtr::create(handle, classId);
  const int columnCount = message.read<int>();
  for(int i = 0; i < columnCount; i++)
    tableModel->m_columnHeaders.push_back(QString::fromUtf8(message.read<QByteArray>()));
  Q_ASSERT(tableModel->m_columnHeaders.size() == columnCount);
  message.read(tableModel->m_rowCount);
  message.readBlockEnd(); // end model
  return tableModel;
}

void Client::setState(State state)
{
  if(m_state != state)
  {
    m_state = state;
    emit stateChanged();
  }
}

void Client::processMessage(const std::shared_ptr<Message> message)
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
        if(Object* object = m_objects.value(message->read<Handle>(), nullptr))
        {
          if(Property* property = object->getProperty(QString::fromLatin1(message->read<QByteArray>())))
          {
            switch(message->read<ValueType>())
            {
              case ValueType::Boolean:
              {
                const bool value = message->read<bool>();
                property->m_value = value;
                emit property->valueChanged();
                emit property->valueChangedBool(value);
                break;
              }
              case ValueType::Integer:
              case ValueType::Enum:
              {
                const qlonglong value = message->read<qlonglong>();
                property->m_value = value;
                emit property->valueChanged();
                emit property->valueChangedInt64(value);
                if(value >= std::numeric_limits<int>::min() && value <= std::numeric_limits<int>::max())
                  emit property->valueChangedInt(static_cast<int>(value));
                break;
              }
              case ValueType::Float:
              {
                const double value = message->read<double>();
                property->m_value = value;
                emit property->valueChanged();
                emit property->valueChangedDouble(value);
                break;
              }
              case ValueType::String:
              {
                const QString value = QString::fromUtf8(message->read<QByteArray>());
                property->m_value = value;
                emit property->valueChanged();
                emit property->valueChangedString(value);
                break;
              }
            }
          }
        }
        break;

      case Message::Command::ObjectAttributeChanged:
        if(Object* object = m_objects.value(message->read<Handle>(), nullptr))
        {
          if(Property* property = object->getProperty(QString::fromLatin1(message->read<QByteArray>())))
          {
            AttributeName attributeName = message->read<AttributeName>();
            QVariant value;
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
            }

            if(Q_LIKELY(value.isValid()))
            {
              property->m_attributes[attributeName] = value;
              emit property->attributeChanged(attributeName, value);
            }
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
              model->m_texts[index] = QString::fromUtf8(data);
            }

          model->endResetModel();
        }
        break;

      default:
        Q_ASSERT(false);
        break;
    }
  }
}

void Client::socketConnected()
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

              getObject("traintastic",
                [this](const ObjectPtr& object, Message::ErrorCode errorCode)
                {
                  if(object && errorCode == Message::ErrorCode::None)
                  {
                    m_traintastic = object;
                    setState(State::Connected);
                  }
                });
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

void Client::socketDisconnected()
{
  setState(State::Disconnected);
}

void Client::socketError(QAbstractSocket::SocketError)
{
  setState(State::SocketError);
}

void Client::socketReadyRead()
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
