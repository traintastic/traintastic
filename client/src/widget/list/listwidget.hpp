/**
 * client/src/widget/list/listwidget.hpp
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

#ifndef TRAINTASTIC_CLIENT_WIDGET_LIST_LISTWIDGET_HPP
#define TRAINTASTIC_CLIENT_WIDGET_LIST_LISTWIDGET_HPP

#include <QWidget>
#include "../../network/objectptr.hpp"

class TableWidget;

class ListWidget : public QWidget
{
private:
  ObjectPtr m_object;
  int m_requestId;

protected:
  TableWidget* m_tableWidget;

  const ObjectPtr& object() { return m_object; }

  virtual void tableSelectionChanged() {}
  virtual void tableDoubleClicked(const QModelIndex& /*index*/) {}

public:
  explicit ListWidget(const ObjectPtr& object, QWidget* parent = nullptr);
  ~ListWidget() override;
};

#endif
