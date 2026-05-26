/**
 * client/src/subwindow/subwindow.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2025 Reinder Feenstra
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

#include "subwindow.hpp"
#include <QVBoxLayout>
#include <QSettings>
#include <QIcon>
#include <QtWaitingSpinner/waitingspinnerwidget.h>
#include "../network/connection.hpp"
#include "../network/object.hpp"
#include "../network/property.hpp"
#include "../network/error.hpp"
#include "../widget/alertwidget.hpp"

SubWindow::SubWindow(SubWindowType type, QWidget* parent)
  : QMdiSubWindow(parent)
  , m_type{type}
  , m_requestId{Connection::invalidRequestId}
{
  setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
}

SubWindow::SubWindow(SubWindowType type, std::shared_ptr<Connection> connection, const QString& id, QWidget* parent)
  : SubWindow(type, parent)
{
  m_connection = std::move(connection);
  m_id = id;

  auto* spinner = new WaitingSpinnerWidget(this, true, false);
  spinner->start();

  m_requestId = m_connection->getObject(id,
    [this, spinner](const ObjectPtr& object, std::optional<const Error> error)
    {
      m_requestId = Connection::invalidRequestId;
      if(object)
        setObject(object);
      else if(error)
      {
        static_cast<QVBoxLayout*>(this->layout())->addWidget(AlertWidget::error(error->toString()));
      }
      else
      {
        assert(false);
      }
      delete spinner;
    });
}

SubWindow::~SubWindow()
{
  if(m_connection)
    m_connection->cancelRequest(m_requestId);

  {
    QSettings s;
    s.beginGroup(settingsGroupName());
    s.setValue("size", size());
    s.setValue("pos", pos());
  }
}

QString SubWindow::settingsGroupName() const
{
  QString uuid = m_connection->worldUUID();
  if(uuid.isEmpty())
    uuid = "00000000-0000-0000-0000-000000000000";
  return uuid + "/" + toString(m_type) + "/" + m_id;
}

void SubWindow::setObject(const ObjectPtr& object)
{
  connect(object.get(), &Object::dead, this, &SubWindow::close);
  m_connection = object->connection();
  if(auto* property = object->getProperty("id"))
  {
    m_id = property->toString();
    connect(property, &Property::valueChangedString, this,
      [this](const QString& newId)
      {
        emit objectIdChanged(this, newId);
      });
  }
  else
    m_id = object->classId();
  setWidget(createWidget(object));
  connect(widget(), &QWidget::windowIconChanged, this, &SubWindow::setWindowIcon);
  connect(widget(), &QWidget::windowTitleChanged, this, &SubWindow::setWindowTitle);
  if(!widget()->windowTitle().isEmpty())
    setWindowTitle(widget()->windowTitle());
  if(!widget()->windowIcon().isNull())
    setWindowIcon(widget()->windowIcon());
  restoreSizeFromSettings();
}

void SubWindow::showEvent(QShowEvent*)
{
  restoreSizeFromSettings();
}

void SubWindow::restoreSizeFromSettings()
{
  QSettings s;
  s.beginGroup(settingsGroupName());

  if(QSize sz = s.value("size", QSize()).toSize(); sz.isValid())
    resize(sz);
  else
    resize(defaultSize());

  static constexpr QPoint invalidPoint{std::numeric_limits<int>::min(), std::numeric_limits<int>::min()};
  if(QPoint pnt = s.value("pos", invalidPoint).toPoint(); pnt != invalidPoint)
    move(pnt);
}
