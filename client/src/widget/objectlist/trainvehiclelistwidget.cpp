/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "trainvehiclelistwidget.hpp"
#include <QMenu>
#include "../tablewidget.hpp"
#include "../../network/method.hpp"
#include "../../network/object.hpp"
#include "../../network/tablemodel.hpp"
#include "../../misc/methodaction.hpp"

static constexpr int columnVehicleId = 1;

TrainVehicleListWidget::TrainVehicleListWidget(const ObjectPtr& object, QWidget* parent)
  : ObjectListWidget(object, parent)
{
  if((m_toggleDirectionInvert = object->getMethod("toggle_direction_invert"))) [[likely]]
  {
    // TODO: move basic popup logic to ObjectListWidget once more lists need popup support

    m_tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tableWidget, &TableWidget::customContextMenuRequested,
      [this](const QPoint& pos)
      {
        if(int row = m_tableWidget->rowAt(pos.y()); row != -1)
        {
          QMenu popup;
          popup.addAction(new MethodAction(*m_toggleDirectionInvert,
            [this, itemId=m_tableWidget->getRowObjectId(row)]()
            {
              m_toggleDirectionInvert->call(itemId);
            },
            &popup));
          popup.exec(m_tableWidget->mapToGlobal(pos));
        }
      });
  }
}

void TrainVehicleListWidget::tableDoubleClicked(const QModelIndex& index)
{
  const QString vehicleId = static_cast<TableModel*>(m_tableWidget->model())->getValue(columnVehicleId, index.row());
  if(!vehicleId.isEmpty())
  {
    objectDoubleClicked(vehicleId);
  }
}
