/**
 * client/src/subwindow/boardsubwindow.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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
  : SubWindow(SubWindowType::Throttle, parent)
{
}

BoardSubWindow::BoardSubWindow(std::shared_ptr<Connection> connection, const QString& id, QWidget* parent)
  : SubWindow(SubWindowType::Throttle, std::move(connection), id, parent)
{
}

QWidget* BoardSubWindow::createWidget(const ObjectPtr& object)
{
  return new BoardWidget(std::dynamic_pointer_cast<Board>(object), this);
}
