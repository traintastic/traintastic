/**
 * client/src/widget/objectlist/objectlistwidget.hpp
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

#ifndef TRAINTASTIC_CLIENT_WIDGET_OBJECTLIST_OBJECTLISTWIDGET_HPP
#define TRAINTASTIC_CLIENT_WIDGET_OBJECTLIST_OBJECTLISTWIDGET_HPP

#include "../list/listwidget.hpp"
#include "../../network/objectptr.hpp"

class QToolBar;
class QToolButton;
class MethodAction;

class ObjectListWidget : public ListWidget
{
  Q_OBJECT

  private:
    int m_requestIdCreate;
    int m_requestIdInputMonitor;
    int m_requestIdOutputKeyboard;
    QToolBar* m_toolbar;
    QToolButton* m_buttonCreate;
    QAction* m_actionCreate;
    QAction* m_actionAdd = nullptr;
    QAction* m_actionEdit;
    MethodAction* m_actionRemove = nullptr;
    MethodAction* m_actionDelete;
    MethodAction* m_actionMoveUp = nullptr;
    MethodAction* m_actionMoveDown = nullptr;
    MethodAction* m_actionReverse = nullptr;
    MethodAction* m_actionInputMonitor;
    MethodAction* m_actionInputMonitorChannel;
    MethodAction* m_actionOutputKeyboard;
    MethodAction* m_actionOutputKeyboardChannel;

  protected:
    QToolBar* toolbar() { return m_toolbar; }

    void tableSelectionChanged() override;
    void tableDoubleClicked(const QModelIndex& index) override;

    virtual void tableSelectionChanged(bool hasSelection);
    virtual void objectDoubleClicked(const QString& id);
    QStringList getSelectedObjectIds() const;

  public:
    explicit ObjectListWidget(const ObjectPtr& object, QWidget* parent = nullptr);
    ~ObjectListWidget() override;
};

#endif
