/**
 * client/src/widget/tablewidget.cpp
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

#include "tablewidget.hpp"
#include <QHeaderView>
#include <QScrollBar>
#include "../network/tablemodel.hpp"

TableWidget::TableWidget(QWidget* parent) :
  QTableView(parent)
{
  setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
  setWordWrap(false);
  verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  verticalHeader()->setDefaultSectionSize(fontMetrics().height());
  horizontalHeader()->setDefaultSectionSize(fontMetrics().height());
}

void TableWidget::setTableModel(const TableModelPtr& model)
{
  Q_ASSERT(!m_model);
  m_model = model;
  setModel(m_model.data());

  connect(m_model.data(), &TableModel::modelReset, this, &TableWidget::updateRegion);
  connect(horizontalScrollBar(), &QScrollBar::rangeChanged, this, &TableWidget::updateRegion);
  connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, &TableWidget::updateRegion);
  connect(verticalScrollBar(), &QScrollBar::rangeChanged, this, &TableWidget::updateRegion);
  connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &TableWidget::updateRegion);

  updateRegion();
}

void TableWidget::updateRegion()
{
  const int columnCount = m_model->columnCount();
  const int rowCount = m_model->rowCount();

  if(columnCount == 0 || rowCount == 0)
    return;

  const QRect r = viewport()->rect();
  const QModelIndex topLeft = indexAt(r.topLeft());

  int rowMin = qMax(topLeft.row(), 0);
  int rowMax = indexAt(r.bottomLeft()).row();
  if(rowMax == -1)
    rowMax = rowCount - 1;
  else
    rowMax = qMin(rowMax + 1, rowCount - 1);

  int columnMin = qMax(topLeft.column(), 0);
  int columnMax = indexAt(r.topRight()).column();
  if(columnMax == -1)
    columnMax = columnCount - 1;
  else
    columnMax = qMin(columnMax + 1, columnCount - 1);

  m_model->setRegion(columnMin, columnMax, rowMin, rowMax);
}
