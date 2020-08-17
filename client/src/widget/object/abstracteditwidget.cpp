/**
 * client/src/widget/object/abstracteditwidget.cpp
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

#include "abstracteditwidget.hpp"
//#include <QFormLayout>
//#include <QVBoxLayout>
//#include <QTabWidget>
#include <QtWaitingSpinner/waitingspinnerwidget.h>
#include "../../network/connection.hpp"
#include "../../network/object.hpp"
#include "../../network/abstractproperty.hpp"
//#include "../../network/property.hpp"
//#include "../../network/objectproperty.hpp"
//#include "../../network/utils.hpp"
//#include "../alertwidget.hpp"
//#include "../propertycheckbox.hpp"
//#include "../propertycombobox.hpp"
//#include "../propertyspinbox.hpp"
//#include "../propertylineedit.hpp"
//#include "../propertytextedit.hpp"
//#include "../propertydirectioncontrol.hpp"
//#include "../propertyvaluelabel.hpp"
//#include "../createwidget.hpp"
#include "../../mainwindow.hpp"
//#include <traintastic/enum/category.hpp>
//#include <traintastic/enum/direction.hpp>

/*
QString toString(Category value)
{
  switch(value)
  {
    case Category::General: return "General";
    case Category::Info: return "Info";
    case Category::Status: return "Status";
    case Category::XpressNet: return "XpressNet";
  }
  return "?";
}*/


AbstractEditWidget::AbstractEditWidget(const ObjectPtr& object, QWidget* parent) :
  QWidget(parent),
  m_requestId{Connection::invalidRequestId},
  m_object{object}
{
}

AbstractEditWidget::AbstractEditWidget(const QString& id, QWidget* parent) :
  QWidget(parent)
{
  setWindowTitle(id);

  auto* spinner = new WaitingSpinnerWidget(this, true, false);
  spinner->start();

  m_requestId = MainWindow::instance->connection()->getObject(id,
    [this, spinner](const ObjectPtr& object, Message::ErrorCode /*ec*/)
    {
      m_requestId = Connection::invalidRequestId;
      if(object)
      {
        m_object = object;
        buildForm();
      }
 // TODO     else
 // TODO       static_cast<QFormLayout*>(this->layout())->addRow(AlertWidget::error(errorCodeToText(ec)));
      delete spinner;
    });
}

AbstractEditWidget::~AbstractEditWidget()
{
  m_object->connection()->cancelRequest(m_requestId);
}

void AbstractEditWidget::setIdAsWindowTitle()
{
  if(AbstractProperty* id = m_object->getProperty("id"))
  {
    connect(id, &AbstractProperty::valueChangedString, this, &AbstractEditWidget::setWindowTitle);
    setWindowTitle(id->toString());
  }
}
