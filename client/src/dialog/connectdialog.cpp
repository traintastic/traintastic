/**
 * client/src/dialog/connectdialog.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2023-2024 Reinder Feenstra
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

#include "connectdialog.hpp"
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QUrl>
#include <QTimer>
#include "../network/connection.hpp"
#include "../network/object.hpp"
#include "../settings/generalsettings.hpp"
#include <traintastic/locale/locale.hpp>

ConnectDialog::ConnectDialog(QWidget* parent, const QString& url) :
  QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
  m_connection{std::make_shared<Connection>()},
  m_udpSocket{new QUdpSocket(this)},
  m_server{new QComboBox()},
  m_username{new QLineEdit()},
  m_password{new QLineEdit()},
  m_connectAutomatically{new QCheckBox(Locale::tr("qtapp.settings.general:connect_automatically_to_discovered_server"))},
  m_status{new QLabel()},
  m_connect{new QPushButton(Locale::tr("qtapp.connect_dialog:connect"))}
{
  setWindowTitle(Locale::tr("qtapp.connect_dialog:connect_to_server"));

  m_server->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
  m_server->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_server->setEditable(true);

  m_password->setEchoMode(QLineEdit::Password);

  m_connectAutomatically->setChecked(GeneralSettings::instance().connectAutomaticallyToDiscoveredServer.value());

  #if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
  connect(m_connectAutomatically, &QCheckBox::stateChanged, this,
    [](int state)
  #else
  connect(m_connectAutomatically, &QCheckBox::checkStateChanged, this,
    [](Qt::CheckState state)
  #endif
    {
      GeneralSettings::instance().connectAutomaticallyToDiscoveredServer.setValue(state == Qt::Checked);
    });

  m_status->setAlignment(Qt::AlignCenter);
  m_status->setMinimumWidth(400);

  QFormLayout* formLayout = new QFormLayout();
  formLayout->setContentsMargins(0, 0, 0, 0);
  formLayout->addRow(Locale::tr("qtapp.connect_dialog:server"), m_server);
/*
  formLayout->addRow(Locale::tr("qtapp.connect_dialog:username"), m_username);
  formLayout->addRow(Locale::tr("qtapp.connect_dialog:password"), m_password);
*/
  formLayout->addRow("", m_connectAutomatically);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->addLayout(formLayout);
  layout->addWidget(m_status);
  layout->addWidget(m_connect, 0, Qt::AlignCenter);
  layout->setSizeConstraint(QLayout::SetFixedSize);

  setLayout(layout);
  connect(m_udpSocket, &QUdpSocket::readyRead, this, &ConnectDialog::socketReadyRead);
  connect(&m_broadcastTimer, &QTimer::timeout, this, &ConnectDialog::broadcast);
  connect(m_connection.get(), &Connection::stateChanged, this, &ConnectDialog::stateChanged);
  connect(m_server, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ConnectDialog::serverIndexChanged);
  connect(m_server, &QComboBox::currentTextChanged, this, &ConnectDialog::serverTextChanged);
  connect(m_connect, &QPushButton::clicked, this, &ConnectDialog::connectClick);

  if(!url.isEmpty())
  {
    m_server->setCurrentText(url);
    QTimer::singleShot(100, this, &ConnectDialog::connectClick);
  }

  m_broadcastTimer.start(1000);
  broadcast();
}

void ConnectDialog::setControlsEnabled(bool value)
{
  m_server->setEnabled(value);
  m_username->setEnabled(value);
  m_password->setEnabled(value);
  m_connect->setEnabled(value);
}

void ConnectDialog::broadcast()
{
  // decrement TTL of all discovered servers:
  for(Servers::iterator it = m_servers.begin(); it != m_servers.end(); )
  {
    if(--it.value().second == 0)
    {
      for(int i = 0; i < m_server->count(); i++)
        if(m_server->itemData(i) == it.key())
        {
          m_server->removeItem(i);
          break;
        }
      it = m_servers.erase(it);
    }
    else
      it++;
  }

  // send discovery request on all networks:
  for(auto& interface : QNetworkInterface::allInterfaces())
    for(auto& addressEntry : interface.addressEntries())
      if(!addressEntry.broadcast().isNull())
      {
        auto message = Message::newRequest(Message::Command::Discover);
        m_udpSocket->writeDatagram(reinterpret_cast<const char*>(**message), message->size(), addressEntry.broadcast(), Connection::defaultPort);
      }
}

void ConnectDialog::socketReadyRead()
{
  while(m_udpSocket->hasPendingDatagrams())
  {
    Message message(m_udpSocket->pendingDatagramSize());
    QHostAddress host;
    quint16 port;

    m_udpSocket->readDatagram(static_cast<char*>(*message), message.size(), &host, &port);

    if(message.command() == Message::Command::Discover && message.isResponse() && !message.isError())
    {
      QString name = QString::fromUtf8(message.read<QByteArray>());

      QUrl url;
      url.setScheme("ws");
      url.setHost(host.toString());
      url.setPort(port);
      url.setPath("/client");

      auto it = m_servers.find(url);
      if(it == m_servers.end())
      {
        m_servers[url] = {name, defaultTTL};
        m_server->addItem(url.host() + (url.port() != Connection::defaultPort ? ":" + QString::number(url.port()) : "") + " (" + name + ")", url);
        if(m_connectAutomatically->isChecked() && m_connect->isEnabled())
        {
          m_connect->click();
        }
      }
      else
        it->second = defaultTTL;
    }
  }
}

void ConnectDialog::stateChanged()
{
  switch(m_connection->state())
  {
    case Connection::State::Disconnected:
      m_status->setText("");
      break;

    case Connection::State::Disconnecting:
      m_status->setText(Locale::tr("qtapp.connect_dialog:disconnecting"));
      break;

    case Connection::State::Connecting:
      m_status->setText(Locale::tr("qtapp.connect_dialog:connecting"));
      break;

    case Connection::State::Connected:
      m_status->setText(Locale::tr("qtapp.connect_dialog:connected"));
      QTimer::singleShot(300, this, &ConnectDialog::accept);
      break;

    case Connection::State::Authenticating:
      m_status->setText(Locale::tr("qtapp.connect_dialog:authenticating"));
      break;

    case Connection::State::CreatingSession:
      m_status->setText(Locale::tr("qtapp.connect_dialog:creating_session"));
      break;

    case Connection::State::FetchingWorld:
      m_status->setText(Locale::tr("qtapp.connect_dialog:fetching_world"));
      break;

    case Connection::State::SocketError:
      m_status->setText(m_connection->errorString());
      setControlsEnabled(true);
      break;

    case Connection::State::ErrorAuthenticationFailed:
      m_status->setText(Locale::tr("qtapp.connect_dialog:authentication_failed"));
      break;

    case Connection::State::ErrorNewSessionFailed:
      m_status->setText(Locale::tr("qtapp.connect_dialog:create_session_failed"));
      break;
  }
}

void ConnectDialog::serverIndexChanged(int index)
{
  if(auto itemData = m_server->itemData(index); itemData.isValid())
  {
    m_url = itemData.toUrl();
    m_connect->setEnabled(m_url.isValid());
  }
}

void ConnectDialog::serverTextChanged(const QString& text)
{
  QString url{text};
  m_url = QUrl::fromUserInput(url.remove(QRegularExpression("\\s*\\(.*\\)$")));
  m_url.setScheme("ws");
  m_url.setPath("/client");
  if(m_url.port() == -1)
  {
    m_url.setPort(Connection::defaultPort);
  }
  m_connect->setEnabled(m_url.isValid());
}

void ConnectDialog::connectClick()
{
  if(!m_url.isValid())
    return;

  setControlsEnabled(false);
  m_connection->connectToHost(m_url, m_username->text(), m_password->text());
}
