/**
 * client/src/network/client.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_CONNECTION_HPP
#define TRAINTASTIC_CLIENT_NETWORK_CONNECTION_HPP

#include <QObject>
#include <memory>
#include <unordered_map>
#include <optional>
#include <QAbstractSocket>
#include <QMap>
#include <QUuid>
#include <traintastic/network/message.hpp>
#include "handle.hpp"
#include "objectptr.hpp"
#include "tablemodelptr.hpp"

class QTcpSocket;
class ServerLogTableModel;
class Property;
class ObjectProperty;
class ObjectVectorProperty;
class UnitProperty;
class Method;
class InputMonitor;
class OutputKeyboard;
class Board;
struct Error;

class Connection : public QObject, public std::enable_shared_from_this<Connection>
{
  Q_OBJECT

  template<typename R>
  friend R getResult(Connection&, const Message&);
  friend class Board;

  public:
    enum class State
    {
      Disconnected,
      Disconnecting,
      Connected,
      Connecting,
      Authenticating,
      CreatingSession,
      FetchingWorld,
      SocketError,
      ErrorAuthenticationFailed,
      ErrorNewSessionFailed,
    };

    using SocketError = QAbstractSocket::SocketError;

  protected:
    QTcpSocket* m_socket;
    State m_state;
    QString m_username;
    QByteArray m_password;
    struct
    {
      qint64 offset = 0;
      Message::Header header;
      std::shared_ptr<Message> message;
    } m_readBuffer;
    QMap<uint16_t, std::function<void(const std::shared_ptr<Message>&)>> m_requestCallback;
    QUuid m_sessionUUID;
    ObjectPtr m_traintastic;
    ObjectProperty* m_worldProperty;
    int m_worldRequestId;
    ObjectPtr m_world;
    ServerLogTableModel* m_serverLogTableModel;
    QMap<Handle, std::weak_ptr<Object>> m_objects;
    std::unordered_map<Handle, uint32_t> m_handleCounter;
    std::unordered_map<Handle, std::unique_ptr<Object>> m_requestForRelease;
    QMap<Handle, TableModel*> m_tableModels;

    void setState(State state);
    void processMessage(const std::shared_ptr<Message> message);

    ObjectPtr readObject(const Message &message);
    TableModelPtr readTableModel(const Message& message);

    void getWorld();
    void setWorld(const ObjectPtr& world);

  protected slots:
    void socketConnected();
    void socketDisconnected();
    void socketError(QAbstractSocket::SocketError);
    void socketReadyRead();

  public:
    static const quint16 defaultPort = 5740;
    static constexpr int invalidRequestId = -1;

    Connection();

    inline bool isConnected() const { return m_state == State::Connected; }
    bool isDisconnected() const;
    State state() const { return m_state; }
    SocketError error() const;
    QString errorString() const;

    void connectToHost(const QUrl& url, const QString& username, const QString& password);
    void disconnectFromHost();

    void send(std::unique_ptr<Message>& message);
    void send(std::unique_ptr<Message>& message, std::function<void(const std::shared_ptr<Message>&)> callback);

    void cancelRequest(int requestId);

    const ObjectPtr& traintastic() const { return m_traintastic; }
    const ObjectPtr& world() const { return m_world; }
    QString worldUUID() const;

    void serverLog(ServerLogTableModel& model, bool enable);

    [[nodiscard]] int getObject(const QString& id, std::function<void(const ObjectPtr&, std::optional<const Error>)> callback);
    [[nodiscard]] int getObject(const ObjectProperty& property, std::function<void(const ObjectPtr&, std::optional<const Error>)> callback);
    [[nodiscard]] int getObject(const ObjectVectorProperty& property, uint32_t index, std::function<void(const ObjectPtr&, std::optional<const Error>)> callback);
    [[nodiscard]] int getObjects(const ObjectVectorProperty& property, uint32_t startIndex, uint32_t endIndex, std::function<void(const std::vector<ObjectPtr>&, std::optional<const Error>)> callback);
    void releaseObject(Object* object);

    void setUnitPropertyUnit(UnitProperty& property, int64_t value);

    void setObjectPropertyById(const ObjectProperty& property, const QString& value);

    void callMethod(Method& method);
    void callMethod(Method& method, const QString& arg);
    [[nodiscard]] int callMethod(Method& method, std::function<void(const ObjectPtr&, std::optional<const Error>)> callback);
    [[nodiscard]] int callMethod(Method& method, const QString& arg, std::function<void(const ObjectPtr&, std::optional<const Error>)> callback);

    [[nodiscard]] int getTableModel(const ObjectPtr& object, std::function<void(const TableModelPtr&, std::optional<const Error>)> callback);
    void releaseTableModel(TableModel* tableModel);
    void setTableModelRegion(TableModel* tableModel, int columnMin, int columnMax, int rowMin, int rowMax);

    [[nodiscard]] int getTileData(Board& object);

  signals:
    void stateChanged();
    void worldChanged();
};

#endif
