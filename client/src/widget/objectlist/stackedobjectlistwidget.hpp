/**
 * client/src/widget/objectlist/stackedobjectlistwidget.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_WIDGET_OBJECTLIST_STACKEDOBJECTLISTWIDGET_HPP
#define TRAINTASTIC_CLIENT_WIDGET_OBJECTLIST_STACKEDOBJECTLISTWIDGET_HPP

#include <QWidget>

#include "../../network/objectptr.hpp"
#include "../../network/tablemodelptr.hpp"

class QToolBar;
class QStackedWidget;
class QListView;
class QLabel;
class QMenu;
class MethodIcon;
class MethodAction;

class StackedObjectListWidget : public QWidget
{
protected:
  ObjectPtr m_object;
  TableModelPtr m_tableModel;
  QToolBar* m_navBar;
  QLabel* m_navLabel;
  QStackedWidget* m_stack;
  QListView* m_list;
  QLabel* m_listEmptyLabel;
  MethodIcon* m_create = nullptr;
  QMenu* m_createMenu = nullptr;
  MethodAction* m_actionRemove = nullptr;
  QString m_listObjectId;
  int m_requestId;

  void cancelRequest();

  void back();
  void show(const ObjectPtr& listObject);

  bool eventFilter(QObject* object, QEvent* event) override;

public:
  explicit StackedObjectListWidget(const ObjectPtr& object, QWidget* parent = nullptr);
  ~StackedObjectListWidget() override;
};

#endif
