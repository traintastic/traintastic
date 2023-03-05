/**
 * client/src/widget/tablewidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2023 Reinder Feenstra
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
#include <QSettings>
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

TableWidget::~TableWidget()
{
  if(m_model)
  {
    QList<QVariant> columnSizes;
    for(int i = 0; i < m_model->columnCount(); i++)
      columnSizes.append(columnWidth(i));
    QSettings().setValue(m_model->classId() + "/column_sizes", columnSizes);
  }
}

QString TableWidget::getRowObjectId(int row) const
{
  return m_model ? m_model->getRowObjectId(row) : "";
}

void TableWidget::setTableModel(const TableModelPtr& model)
{
  Q_ASSERT(!m_model);
  m_model = model;
  setModel(m_model.get());

  const int defaultWidth = fontMetrics().averageCharWidth() * 10;
  QList<QVariant> columnSizes = QSettings().value(m_model->classId() + "/column_sizes").toList();
  for(int i = 0; i < m_model->columnCount(); i++)
    setColumnWidth(i, i < columnSizes.count() ? columnSizes[i].toInt() : defaultWidth);

  connect(m_model.get(), &TableModel::modelAboutToBeReset, this,
    [this]()
    {
      m_selectedRow = -1;
      if(const auto* sm = selectionModel())
        if(auto rows = sm->selectedRows(); rows.size() == 1)
          m_selectedRow = rows[0].row();
    });
  connect(m_model.get(), &TableModel::modelReset, this,
    [this]()
    {
      updateRegion();
      if(m_selectedRow != -1)
        selectRow(m_selectedRow);
    });
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
