/**
 * client/src/subwindow/objectsubwindow.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#include "objectsubwindow.hpp"
#include "../widget/createwidget.hpp"

ObjectSubWindow* ObjectSubWindow::create(const ObjectPtr& object, QWidget* parent)
{
  auto* w = new ObjectSubWindow(parent);
  w->setObject(object);
  return w;
}

ObjectSubWindow* ObjectSubWindow::create(std::shared_ptr<Connection> connection, const QString& id, QWidget* parent)
{
  return new ObjectSubWindow(std::move(connection), id, parent);
}

ObjectSubWindow::ObjectSubWindow(QWidget* parent)
  : SubWindow(SubWindowType::Object, parent)
{
}

ObjectSubWindow::ObjectSubWindow(std::shared_ptr<Connection> connection, const QString& id, QWidget* parent)
  : SubWindow(SubWindowType::Object, std::move(connection), id, parent)
{
}

QWidget* ObjectSubWindow::createWidget(const ObjectPtr& object)
{
  return ::createWidget(object);
}

