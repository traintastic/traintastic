/**
 * client/src/widget/objectlistwidget.cpp
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

#include "objectlistwidget.hpp"
#include <QVBoxLayout>
#include <QToolBar>
#include <QTableView>
#include <QtWaitingSpinner/waitingspinnerwidget.h>
#include "tablewidget.hpp"
#include "../network/connection.hpp"
#include "../network/object.hpp"
#include "../network/tablemodel.hpp"
#include "../network/method.hpp"
#include "../network/utils.hpp"
#include "../widget/alertwidget.hpp"


#include "../mainwindow.hpp"



#include <QMenu>
#include <QToolButton>



ObjectListWidget::ObjectListWidget(const ObjectPtr& object, QWidget* parent) :
  QWidget(parent),
  m_buttonAdd{nullptr},
  m_object{object},
  m_toolbar{new QToolBar()},
  m_actionAdd{nullptr},
  m_actionEdit{nullptr},
  m_actionDelete{nullptr},
  m_tableWidget{new TableWidget()}
{
  QVBoxLayout* layout = new QVBoxLayout();
  layout->setMargin(0);
  layout->addWidget(m_toolbar);
  layout->addWidget(m_tableWidget);
  setLayout(layout);

  auto* spinner = new WaitingSpinnerWidget(this, true, false);
  spinner->start();

  m_requestId = m_object->connection()->getTableModel(m_object,
    [this, spinner](const TableModelPtr& tableModel, Message::ErrorCode ec)
    {
      if(tableModel)
      {
        m_requestId = Connection::invalidRequestId;

        m_tableWidget->setTableModel(tableModel);
        connect(m_tableWidget, &TableWidget::doubleClicked, this, &ObjectListWidget::tableDoubleClicked);

        delete spinner;
      }
      else
        static_cast<QVBoxLayout*>(this->layout())->insertWidget(0, AlertWidget::error(errorCodeToText(ec)));
    });

  if(Method* method = m_object->getMethod("add");
     method && method->resultType() == ValueType::Object)
  {
    if(method->argumentTypes().size() == 0) // Add method witout argument
    {
      m_actionAdd = m_toolbar->addAction(QIcon(":/dark/add.svg"), method->displayName(),
        [this, method]()
        {
          if(m_requestIdAdd != Connection::invalidRequestId)
            m_object->connection()->cancelRequest(m_requestIdAdd);

          m_requestIdAdd = method->call(
            [this](const ObjectPtr& object, Message::ErrorCode /*ec*/)
            {
              m_requestIdAdd = Connection::invalidRequestId;
              if(object)
              {
                MainWindow::instance->showObject(object);
              }
              // TODO: show error
            });
        });
      m_actionAdd->setEnabled(method->getAttributeBool(AttributeName::Enabled, true));
      connect(method, &Method::attributeChanged,
        [this](AttributeName name, QVariant value)
        {
          if(name == AttributeName::Enabled)
            m_actionAdd->setEnabled(value.toBool());
        });
    }
    else if(method->argumentTypes().size() == 1 && method->argumentTypes()[0] == ValueType::String)
    {
      m_buttonAdd = new QToolButton(m_toolbar);
      m_buttonAdd->setIcon(QIcon(":/dark/add.svg"));
      m_buttonAdd->setText(method->displayName());
      m_buttonAdd->setPopupMode(QToolButton::InstantPopup);

      QMenu* menu = new QMenu(m_buttonAdd);
      menu->addAction("li10x");
      menu->addAction("z21");
      menu->addAction("loconet_serial",
        [this, method]()
        {
          if(m_requestIdAdd != Connection::invalidRequestId)
            m_object->connection()->cancelRequest(m_requestIdAdd);

          m_requestIdAdd = method->call("hardware.command_station.loconet_serial",
            [this](const ObjectPtr& object, Message::ErrorCode /*ec*/)
            {
              m_requestIdAdd = Connection::invalidRequestId;
              if(object)
              {
                MainWindow::instance->showObject(object);
              }
              // TODO: show error
            });
        });
      menu->addAction("locomotive",
        [this, method]()
        {
          if(m_requestIdAdd != Connection::invalidRequestId)
            m_object->connection()->cancelRequest(m_requestIdAdd);

          m_requestIdAdd = method->call("vehicle.rail.locomotive",
            [this](const ObjectPtr& object, Message::ErrorCode /*ec*/)
            {
              m_requestIdAdd = Connection::invalidRequestId;
              if(object)
              {
                MainWindow::instance->showObject(object);
              }
              // TODO: show error
            });
        });
      menu->addAction("locomotive",
        [this, method]()
        {
          if(m_requestIdAdd != Connection::invalidRequestId)
            m_object->connection()->cancelRequest(m_requestIdAdd);

          m_requestIdAdd = method->call("vehicle.rail.freight_car",
            [this](const ObjectPtr& object, Message::ErrorCode /*ec*/)
            {
              m_requestIdAdd = Connection::invalidRequestId;
              if(object)
              {
                MainWindow::instance->showObject(object);
              }
              // TODO: show error
            });
        });

      m_buttonAdd->setMenu(menu);

      m_toolbar->addWidget(m_buttonAdd);


            /*
            m_requestIdAdd = method->call("hardware.command_station.li10x",
              [this](const ObjectPtr& object, Message::ErrorCode /*ec*//*)
              {
                m_requestIdAdd = Connection::invalidRequestId;

                if(object)
                {
                  MainWindow::instance->showObject(object);
                }

                // TODO: show error


              });

            */


      m_buttonAdd->setEnabled(method->getAttributeBool(AttributeName::Enabled, true));
      connect(method, &Method::attributeChanged,
        [this](AttributeName name, QVariant value)
        {
          if(name == AttributeName::Enabled)
            m_buttonAdd->setEnabled(value.toBool());
        });
    }
    else
      Q_ASSERT(false); // unsupported method prototype
  }

  m_actionEdit = m_toolbar->addAction(QIcon(":/dark/edit.svg"), tr("Edit"));
  m_actionEdit->setEnabled(false);

  if(Method* method = m_object->getMethod("remove"))
  {
    m_actionDelete = m_toolbar->addAction(QIcon(":/dark/delete.svg"), method->displayName());
    //m_actionDelete->setEnabled(false);
  }
}

ObjectListWidget::~ObjectListWidget()
{
  m_object->connection()->cancelRequest(m_requestId);
}

void ObjectListWidget::addActionEdit()
{
 // Q_ASSERT(!m_actionEdit);

}

void ObjectListWidget::addActionDelete()
{
 // Q_ASSERT(!m_actionDelete);
}

void ObjectListWidget::tableDoubleClicked(const QModelIndex& index)
{
  const QString id = m_tableWidget->getRowObjectId(index.row());
  if(!id.isEmpty())
    MainWindow::instance->showObject(id);//emit rowDoubleClicked(id);
}



