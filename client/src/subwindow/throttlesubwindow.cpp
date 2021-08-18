/**
 * client/src/subwindow/throttlesubwindow.cpp
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

#include "throttlesubwindow.hpp"
#include "../widget/throttle/throttlewidget.hpp"

ThrottleSubWindow* ThrottleSubWindow::create(const ObjectPtr& object, QWidget* parent)
{
  auto* w = new ThrottleSubWindow(parent);
  w->setObject(object);
  return w;
}

ThrottleSubWindow* ThrottleSubWindow::create(std::shared_ptr<Connection> connection, const QString& id, QWidget* parent)
{
  return new ThrottleSubWindow(std::move(connection), id, parent);
}

ThrottleSubWindow::ThrottleSubWindow(QWidget* parent)
  : SubWindow(SubWindowType::Throttle, parent)
{
}

ThrottleSubWindow::ThrottleSubWindow(std::shared_ptr<Connection> connection, const QString& id, QWidget* parent)
  : SubWindow(SubWindowType::Throttle, std::move(connection), id, parent)
{
}

QWidget* ThrottleSubWindow::createWidget(const ObjectPtr& object)
{
  return new ThrottleWidget(object, this);
}
