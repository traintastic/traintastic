/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2021-2026 Reinder Feenstra
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

#include "boardsubwindow.hpp"
#include <QSettings>
#include "../board/boardwidget.hpp"
#include "../network/board.hpp"

BoardSubWindow* BoardSubWindow::create(const ObjectPtr& object, QWidget* parent)
{
  auto* w = new BoardSubWindow(parent);
  w->setObject(object);
  return w;
}

BoardSubWindow* BoardSubWindow::create(std::shared_ptr<Connection> connection, const QString& id, QWidget* parent)
{
  return new BoardSubWindow(std::move(connection), id, parent);
}

BoardSubWindow::BoardSubWindow(QWidget* parent)
  : SubWindow(SubWindowType::Board, parent)
{
}

BoardSubWindow::BoardSubWindow(std::shared_ptr<Connection> connection, const QString& id, QWidget* parent)
  : SubWindow(SubWindowType::Board, std::move(connection), id, parent)
{
}

BoardSubWindow::~BoardSubWindow()
{
  if(auto* board = qobject_cast<BoardWidget*>(widget())) [[likely]]
  {
    QSettings s;
    s.beginGroup(settingsGroupName());
    s.setValue("zoom_level", board->zoomLevel());
  }
}

QWidget* BoardSubWindow::createWidget(const ObjectPtr& object)
{
  auto* board = new BoardWidget(std::dynamic_pointer_cast<Board>(object), this);
  QSettings s;
  s.beginGroup(settingsGroupName());
  board->setZoomLevel(s.value("zoom_level", board->zoomLevel()).toInt());
  return board;
}
