/**
 * client/src/widget/objectlistwidget.hpp
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

#ifndef TRAINTASTIC_CLIENT_WIDGET_OBJECTLISTWIDGET_HPP
#define TRAINTASTIC_CLIENT_WIDGET_OBJECTLISTWIDGET_HPP

#include <QWidget>
#include "../network/objectptr.hpp"

class QToolBar;
class TableWidget;

class ObjectListWidget : public QWidget
{
  Q_OBJECT

  protected:
    const QString m_id;
    int m_requestId;
    int m_requestIdAdd;
    ObjectPtr m_object;
    QToolBar* m_toolbar;
    QAction* m_actionAdd;
    QAction* m_actionEdit;
    QAction* m_actionDelete;
    TableWidget* m_tableWidget;

    void addActionEdit();
    void addActionDelete();

    virtual void add() { Q_ASSERT(false); }

  protected slots:
    void tableDoubleClicked(const QModelIndex& index);

  public:
    explicit ObjectListWidget(const ObjectPtr& object, QWidget* parent = nullptr);
    ~ObjectListWidget() override;

  //signals:
  //  void rowDoubleClicked(const QString& id);
};

#endif
