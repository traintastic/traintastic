/**
 * client/src/widget/serverlogwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#include "serverlogwidget.hpp"
#include <QHeaderView>
#include <QSettings>
#include <QScrollBar>
#include "../network/serverlogtablemodel.hpp"

ServerLogWidget::ServerLogWidget(std::shared_ptr<Connection> connection, QWidget* parent)
  : QTableView(parent)
  , m_model{new ServerLogTableModel(std::move(connection))}
{
  setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
  setWordWrap(false);

  verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  verticalHeader()->setDefaultSectionSize(fontMetrics().height() + 1);

  horizontalHeader()->setDefaultSectionSize(fontMetrics().height() + 1);
  horizontalHeader()->setStretchLastSection(true);

  connect(verticalScrollBar(), &QScrollBar::rangeChanged, [this]() { updateStickToBottom(); });
  connect(verticalScrollBar(), &QScrollBar::valueChanged, [this]() { updateStickToBottom(); });

  setModel(m_model);

  connect(m_model, &ServerLogTableModel::rowCountChanged,
    [this]()
    {
      if(m_stickToBottom)
        scrollToBottom();
    });

  const int averageCharWidth = fontMetrics().averageCharWidth();
  setColumnWidth(ServerLogTableModel::columnTime, averageCharWidth * 25);
  setColumnWidth(ServerLogTableModel::columnObject, averageCharWidth * 20);
  setColumnWidth(ServerLogTableModel::columnCode, averageCharWidth * 10);

  QList<QVariant> columnSizes = QSettings().value(settingColumSizes).toList();
  for(int i = 0; i < m_model->columnCount(); i++)
    if(i < columnSizes.count() && i != ServerLogTableModel::columnMessage)
      setColumnWidth(i, columnSizes[i].toInt());
}

ServerLogWidget::~ServerLogWidget()
{
  QList<QVariant> columnSizes;
  for(int i = 0; i < m_model->columnCount(); i++)
    columnSizes.append(columnWidth(i));
  QSettings().setValue(settingColumSizes, columnSizes);
}

void ServerLogWidget::updateStickToBottom()
{
  const int bottomRow = indexAt(viewport()->rect().bottomLeft()).row();
  m_stickToBottom = (bottomRow == -1) || (bottomRow == m_model->rowCount() - 1);
}
