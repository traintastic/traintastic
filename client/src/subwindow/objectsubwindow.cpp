/**
 * client/src/subwindow/objectsubwindow.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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
#include <QVBoxLayout>
#include <QtWaitingSpinner/waitingspinnerwidget.h>
#include "../network/connection.hpp"
#include "../network/object.hpp"
#include "../network/utils.hpp"
#include "../widget/alertwidget.hpp"
#include "../widget/createwidget.hpp"

ObjectSubWindow::ObjectSubWindow(const ObjectPtr& object, QWidget* parent) :
  QMdiSubWindow(parent),
  m_connection{object->connection()},
  m_requestId{Connection::invalidRequestId}
{
  setObject(object);
}

ObjectSubWindow::ObjectSubWindow(const QSharedPointer<Connection>& connection, const QString& id, QWidget* parent) :
  QMdiSubWindow(parent),
  m_connection{connection},
  m_requestId{Connection::invalidRequestId}
{
  auto* spinner = new WaitingSpinnerWidget(this, true, false);
  spinner->start();

  m_requestId = m_connection->getObject(id,
    [this, spinner](const ObjectPtr& object, Message::ErrorCode ec)
    {
      m_requestId = Connection::invalidRequestId;
      if(object)
        setObject(object);
      else
        static_cast<QVBoxLayout*>(this->layout())->addWidget(AlertWidget::error(errorCodeToText(ec)));
      delete spinner;
    });
}

ObjectSubWindow::~ObjectSubWindow()
{
  m_connection->cancelRequest(m_requestId);
}

void ObjectSubWindow::setObject(const ObjectPtr& object)
{
  setWidget(createWidget(object));
  connect(widget(), &QWidget::windowTitleChanged, this, &ObjectSubWindow::setWindowTitle);
  if(!widget()->windowTitle().isEmpty())
    setWindowTitle(widget()->windowTitle());
}
