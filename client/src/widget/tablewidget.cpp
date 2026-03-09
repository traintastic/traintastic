/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2019-2026 Reinder Feenstra
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
#include <QMouseEvent>
#include <QApplication>
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

QStringList TableWidget::getObjectIds() const
{
  assert(m_fetchAll);
  QStringList ids;
  if(m_model)
  {
    ids.reserve(m_model->rowCount());
    for(int row = 0; row < m_model->rowCount(); ++row)
    {
      ids.append(m_model->getRowObjectId(row));
    }
  }
  return ids;
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

void TableWidget::setFetchAll(bool value)
{
  if(m_fetchAll != value)
  {
    m_fetchAll = value;
    if(m_model)
    {
      updateRegion();
    }
  }
}

void TableWidget::updateRegion()
{
  assert(m_model);

  const int columnCount = m_model->columnCount();
  const int rowCount = m_model->rowCount();

  if(columnCount == 0 || rowCount == 0)
    return;

  if(m_fetchAll)
  {
    m_model->setRegion(0, columnCount - 1, 0, rowCount - 1);
    return;
  }

  const QRect r = viewport()->rect();
  const QModelIndex topLeft = indexAt(r.topLeft());

  int rowMin = qMax(topLeft.row(), 0);
  int rowMax = indexAt(r.bottomLeft()).row();

  if(rowCount == 0)
  {
    // Invalid region to represent empty model
    rowMin = 1;
    rowMax = 0;
  }
  else if(rowMax == -1)
    rowMax = rowCount - 1;
  else
    rowMax = qMin(rowMax + 1, rowCount - 1);

  int columnMin = qMax(topLeft.column(), 0);
  int columnMax = indexAt(r.topRight()).column();

  if(columnCount == 0)
  {
    // Invalid region to represent empty model
    columnMin = 1;
    columnMax = 0;
  }
  if(columnMax == -1)
    columnMax = columnCount - 1;
  else
    columnMax = qMin(columnMax + 1, columnCount - 1);

  m_model->setRegion(uint32_t(columnMin), uint32_t(columnMax),
                     uint32_t(rowMin), uint32_t(rowMax));
}

void TableWidget::mouseMoveEvent(QMouseEvent* event)
{
  QTableView::mouseMoveEvent(event);

  if(!m_dragStarted && (event->buttons() & Qt::LeftButton) && (event->pos() - m_dragStartPosition).manhattanLength() >= QApplication::startDragDistance())
  {
    if(const int row = indexAt(m_dragStartPosition).row(); row >= 0)
    {
      m_dragStarted = true;
      emit rowDragged(row);
    }
  }
}

void TableWidget::mousePressEvent(QMouseEvent* event)
{
  QTableView::mousePressEvent(event);

  if(event->button() == Qt::LeftButton)
  {
    m_dragStartPosition = event->pos();
  }
}

void TableWidget::mouseReleaseEvent(QMouseEvent* event)
{
  QTableView::mouseReleaseEvent(event);

  if(event->button() == Qt::LeftButton)
  {
    m_dragStarted = false;
  }
}
