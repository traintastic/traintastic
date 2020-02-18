/**
 * client/src/widget/objectlistwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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
#include "../network/client.hpp"
//#include "../network/object.hpp"
#include "../network/tablemodel.hpp"
#include "../network/utils.hpp"
#include "../widget/alertwidget.hpp"


#include "../mainwindow.hpp"

ObjectListWidget::ObjectListWidget(const ObjectPtr& object, QWidget* parent) :
  QWidget(parent),
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

  m_requestId = Client::instance->getTableModel(m_object,
    [this, spinner](const TableModelPtr& tableModel, Message::ErrorCode ec)
    {
      if(tableModel)
      {
        m_requestId = Client::invalidRequestId;

        m_tableWidget->setTableModel(tableModel);
        connect(m_tableWidget, &TableWidget::doubleClicked, this, &ObjectListWidget::tableDoubleClicked);

        delete spinner;
      }
      else
        static_cast<QVBoxLayout*>(this->layout())->insertWidget(0, AlertWidget::error(errorCodeToText(ec)));
    });
}

ObjectListWidget::~ObjectListWidget()
{
  Client::instance->cancelRequest(m_requestId);
}

void ObjectListWidget::addActionAdd()
{
  Q_ASSERT(!m_actionAdd);
  m_actionAdd = m_toolbar->addAction(QIcon(":/dark/add.svg"), tr("Add"), [this](){ add(); });
}

void ObjectListWidget::addActionEdit()
{
  Q_ASSERT(!m_actionEdit);
  m_actionEdit = m_toolbar->addAction(QIcon(":/dark/edit.svg"), tr("Edit"));
  m_actionEdit->setEnabled(false);
}

void ObjectListWidget::addActionDelete()
{
  Q_ASSERT(!m_actionDelete);
  m_actionDelete = m_toolbar->addAction(QIcon(":/dark/delete.svg"), tr("Delete"));
  m_actionDelete->setEnabled(false);
}

void ObjectListWidget::tableDoubleClicked(const QModelIndex& index)
{
  const QString id = m_tableWidget->getRowObjectId(index.row());
  if(!id.isEmpty())
    MainWindow::instance->showObject(id);//emit rowDoubleClicked(id);
}



