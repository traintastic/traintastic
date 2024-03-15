/**
 * client/src/dialog/connectdialog.hpp
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

#ifndef TRAINTASTIC_CLIENT_DIALOG_CONNECTDIALOG_HPP
#define TRAINTASTIC_CLIENT_DIALOG_CONNECTDIALOG_HPP

#include <QDialog>
#include <memory>
#include <QTimer>
#include <QMap>
#include <QPair>
#include <QUrl>

class QUdpSocket;
class QComboBox;
class QLineEdit;
class QCheckBox;
class QLabel;
class QPushButton;
class Connection;

class ConnectDialog : public QDialog
{
  protected:
    using Servers = QMap<QUrl, QPair<QString, int>>;

    static constexpr int defaultTTL = 3;

    std::shared_ptr<Connection> m_connection;
    QUdpSocket* m_udpSocket;
    Servers m_servers;
    QTimer m_broadcastTimer;
    QComboBox* m_server;
    QLineEdit* m_username;
    QLineEdit* m_password;
    QCheckBox* m_connectAutomatically;
    QLabel* m_status;
    QPushButton* m_connect;
    QUrl m_url;

    void setControlsEnabled(bool value);

  protected slots:
    void broadcast();
    void socketReadyRead();
    void stateChanged();
    void serverIndexChanged(int index);
    void serverTextChanged(const QString& text);
    void connectClick();

  public:
    ConnectDialog(QWidget* parent = nullptr, const QString& url = QString());

    const std::shared_ptr<Connection>& connection() { return m_connection; }
};

#endif
