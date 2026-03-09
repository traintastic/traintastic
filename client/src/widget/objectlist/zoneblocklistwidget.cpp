/**
 * client/src/widget/objectlist/zoneblocklistwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "zoneblocklistwidget.hpp"
#include <QToolBar>
#include <QAction>
#include <traintastic/locale/locale.hpp>
#include "../../theme/theme.hpp"
#include "../../network/object.hpp"
#include "../../network/connection.hpp"
#include "../../network/tablemodel.hpp"
#include "../../board/blockhighlight.hpp"
#include "../../widget/tablewidget.hpp"
#include "../../mainwindow.hpp"

ZoneBlockListWidget::ZoneBlockListWidget(const ObjectPtr& object, QWidget* parent)
  : ObjectListWidget(object, parent)
{
  m_tableWidget->setFetchAll(true);
  toolbar()->addSeparator();
  m_actionHighlight = toolbar()->addAction(Theme::getIcon("highlight_zone"), Locale::tr("list.zone:highlight_zone"),
    [this]()
    {
      updateHighlight();
    });
  m_actionHighlight->setCheckable(true);
}

ZoneBlockListWidget::~ZoneBlockListWidget()
{
  if(m_actionHighlight->isChecked())
  {
    m_actionHighlight->setChecked(false);
    updateHighlight();
  }
}

void ZoneBlockListWidget::setTableModel(const TableModelPtr& tableModel)
{
  ObjectListWidget::setTableModel(tableModel);
  connect(tableModel.get(), &TableModel::modelReset, this,
    [this]()
    {
      if(m_actionHighlight->isChecked())
      {
        updateHighlight();
      }
    });
}

void ZoneBlockListWidget::updateHighlight()
{
  const auto highlight = m_actionHighlight->isChecked() ? m_tableWidget->getObjectIds() : QStringList();

  if(!highlight.isEmpty() && m_highlightColor == Color::None)
  {
    m_highlightColor = MainWindow::instance->blockHighlight().colorPool.aquire();
  }

  for(const auto& blockId : m_highlight)
  {
    if(!highlight.contains(blockId))
    {
      MainWindow::instance->blockHighlight().remove(blockId, m_highlightColor);
    }
  }

  for(const auto& blockId : highlight)
  {
    if(!m_highlight.contains(blockId))
    {
      MainWindow::instance->blockHighlight().add(blockId, m_highlightColor);
    }
  }

  m_highlight = highlight;

  if(m_highlight.isEmpty() && m_highlightColor != Color::None)
  {
    MainWindow::instance->blockHighlight().colorPool.release(m_highlightColor);
    m_highlightColor = Color::None;
  }
}
