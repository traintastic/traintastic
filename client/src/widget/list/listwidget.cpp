/**
 * client/src/widget/list/istwidget.cpp
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

#include "listwidget.hpp"
#include <QVBoxLayout>
#include <QtWaitingSpinner/waitingspinnerwidget.h>
#include "../alertwidget.hpp"
#include "../tablewidget.hpp"
#include "../../network/connection.hpp"
#include "../../network/object.hpp"
#include "../../network/error.hpp"
#include "../../theme/theme.hpp"

ListWidget::ListWidget(const ObjectPtr& object, QWidget* parent)
  : QWidget(parent)
  , m_object{object}
  , m_tableWidget{new TableWidget(this)}
{
  Theme::setWindowIcon(*this, m_object->classId());

  m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_tableWidget);
  setLayout(layout);

  auto* spinner = new WaitingSpinnerWidget(this, true, false);
  spinner->start();

  m_requestId = m_object->connection()->getTableModel(m_object,
    [this, spinner](const TableModelPtr& tableModel, std::optional<const Error> error)
    {
      if(tableModel)
      {
        m_requestId = Connection::invalidRequestId;
        setTableModel(tableModel);
        delete spinner;
      }
      else if(error)
      {
        static_cast<QVBoxLayout*>(this->layout())->insertWidget(0, AlertWidget::error(error->toString()));
      }
      else /*[[unlikely]]*/
      {
        assert(false);
      }
    });
}

ListWidget::~ListWidget()
{
  object()->connection()->cancelRequest(m_requestId);
}

void ListWidget::setTableModel(const TableModelPtr& tableModel)
{
  m_tableWidget->setTableModel(tableModel);
  connect(m_tableWidget, &TableWidget::doubleClicked, this,
    [this](const QModelIndex& index)
    {
      tableDoubleClicked(index);
    });
  connect(m_tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this,
    [this](const QItemSelection&, const QItemSelection&)
    {
      tableSelectionChanged();
    });
  tableSelectionChanged();
}
