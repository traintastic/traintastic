/**
 * client/src/widget/list/marklincanlocomotivelistwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#include "marklincanlocomotivelistwidget.hpp"
#include "../tablewidget.hpp"
#include "../../misc/methodaction.hpp"
#include "../../network/object.hpp"
#include "../../network/method.hpp"
#include "../../network/tablemodel.hpp"
#include "../../network/callmethod.hpp"
#include "../../theme/theme.hpp"
#include <QVBoxLayout>
#include <QToolBar>

MarklinCANLocomotiveListWidget::MarklinCANLocomotiveListWidget(const ObjectPtr& object, QWidget* parent)
  : ListWidget(object, parent)
  , m_toolbar{new QToolBar(this)}
{
  static_cast<QVBoxLayout*>(this->layout())->insertWidget(0, m_toolbar);

  if(Method* method = object->getMethod("import_or_sync"))
  {
    m_importOrSyncAction = new MethodAction(Theme::getIcon("import_or_sync"), *method,
      [this]()
      {
        if(auto* model = m_tableWidget->selectionModel(); model && model->hasSelection())
        {
          if(const auto rows = model->selectedRows(); !rows.empty())
          {
            const auto name = static_cast<TableModel*>(m_tableWidget->model())->getValue(columnName, rows[0].row());
            callMethod(m_importOrSyncAction->method(), nullptr, name);
          }
        }
      });
    m_toolbar->addAction(m_importOrSyncAction);
  }

  if(Method* method = object->getMethod("import_or_sync_all"))
  {
    m_toolbar->addAction(new MethodAction(Theme::getIcon("import_or_sync_all"), *method));
  }

  if(Method* method = object->getMethod("reload"))
  {
    auto* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacer->show();
    m_toolbar->addWidget(spacer);

    m_toolbar->addAction(new MethodAction(Theme::getIcon("update"), *method));
  }
}

void MarklinCANLocomotiveListWidget::tableSelectionChanged()
{
  const bool hasSelection = m_tableWidget->selectionModel() && m_tableWidget->selectionModel()->hasSelection();
  m_importOrSyncAction->setForceDisabled(!hasSelection);
}
