/**
 * client/src/dialog/worldlistdialog.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_DIALOG_WORLDLISTDIALOG_HPP
#define TRAINTASTIC_CLIENT_DIALOG_WORLDLISTDIALOG_HPP

#include <QDialog>
#include "../network/objectptr.hpp"
#include "../network/tablemodelptr.hpp"

class QDialogButtonBox;
class QLineEdit;
class QListView;
class Connection;

class WorldListDialog final : public QDialog
{
  Q_OBJECT

  protected:
    std::shared_ptr<Connection> m_connection;
    int m_requestId;
    ObjectPtr m_object;
    QDialogButtonBox* m_buttons; // TODO: m_buttonLoad;
    QLineEdit* m_search;
    QListView* m_list;
    QString m_uuid;
    TableModelPtr m_tableModel;

  public:
    explicit WorldListDialog(std::shared_ptr<Connection> connection, QWidget* parent = nullptr);
    ~WorldListDialog() final;

    QString uuid() const { return m_uuid; }
};

#endif
