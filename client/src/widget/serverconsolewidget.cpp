/**
 * client/src/widget/serverconsolewidget.cpp
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

#include "serverconsolewidget.hpp"
#include <QVBoxLayout>
//#include <QToolBar>
#include <QTableView>
#include <QtWaitingSpinner/waitingspinnerwidget.h>
#include "tablewidget.hpp"
#include "../network/client.hpp"
//#include "../network/object.hpp"
#include "../network/tablemodel.hpp"
#include "../network/utils.hpp"
#include "../widget/alertwidget.hpp"

ServerConsoleWidget::ServerConsoleWidget(QWidget* parent) :
  QWidget(parent),
  m_tableWidget{new TableWidget()}
{
  QVBoxLayout* layout = new QVBoxLayout();
  layout->setMargin(0);
  //layout->addWidget(new QToolBar());
  layout->addWidget(m_tableWidget);
  setLayout(layout);

  auto* spinner = new WaitingSpinnerWidget(this, true, false);
  spinner->start();

  m_requestId = Client::instance->getObject(objectId,
    [this, spinner](const ObjectPtr& object, Message::ErrorCode ec)
    {
      m_requestId = Client::invalidRequestId;
      if(object)
      {
        m_object = object;

        m_requestId = Client::instance->getTableModel(objectId,
          [this, spinner](const TableModelPtr& tableModel, Message::ErrorCode ec)
          {
            if(tableModel)
            {
              m_requestId = Client::invalidRequestId;

              m_tableWidget->setTableModel(tableModel);

              delete spinner;
            }
            else
              static_cast<QVBoxLayout*>(this->layout())->insertWidget(0, AlertWidget::error(errorCodeToText(ec)));
          });
        }
      else
        static_cast<QVBoxLayout*>(this->layout())->insertWidget(0, AlertWidget::error(errorCodeToText(ec)));
    });
}

ServerConsoleWidget::~ServerConsoleWidget()
{
  Client::instance->cancelRequest(m_requestId);
}
