/**
 * client/src/dialog/objectselectlistdialog.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_DIALOG_OBJECTSELECTLISTDIALOG_HPP
#define TRAINTASTIC_CLIENT_DIALOG_OBJECTSELECTLISTDIALOG_HPP

#include <QDialog>
#include <QModelIndexList>
#include "../network/objectptr.hpp"

class QDialogButtonBox;
class TableWidget;
class Connection;
class InterfaceItem;
class Method;
class ObjectProperty;

class ObjectSelectListDialog : public QDialog
{
  Q_OBJECT

  protected:
    InterfaceItem& m_item;
    const bool m_multiSelect;
    ObjectPtr m_object;
    int m_requestId;
    QDialogButtonBox* m_buttons;
    TableWidget* m_tableWidget;

    ObjectSelectListDialog(InterfaceItem& item, bool multiSelect, QWidget* parent);

    void acceptRow(int row);
    void acceptRows(const QModelIndexList& indexes);

  public:
    ObjectSelectListDialog(Method& method, bool multiSelect, QWidget* parent);
    ObjectSelectListDialog(ObjectProperty& property, QWidget* parent);
};

#endif
