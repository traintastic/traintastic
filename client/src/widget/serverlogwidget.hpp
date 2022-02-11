/**
 * client/src/widget/serverlogwidget.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_WIDGET_SERVERLOGWIDGET_HPP
#define TRAINTASTIC_CLIENT_WIDGET_SERVERLOGWIDGET_HPP

#include <QTableView>
#include <memory>

class Connection;
class ServerLogTableModel;

class ServerLogWidget final : public QTableView
{
  private:
    inline static QString settingColumSizes = QStringLiteral("server_log/column_sizes");

    ServerLogTableModel* m_model;
    bool m_stickToBottom = true;

    void updateStickToBottom();

  public:
    ServerLogWidget(std::shared_ptr<Connection> connection, QWidget* parent = nullptr);
    ~ServerLogWidget() final;
};

#endif
