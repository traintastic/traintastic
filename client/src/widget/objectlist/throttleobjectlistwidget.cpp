/**
 * client/src/widget/objectlist/throttleobjectlistwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#include "throttleobjectlistwidget.hpp"
#include <QToolBar>
#include <traintastic/locale/locale.hpp>
#include "../../network/connection.hpp"
#include "../../network/object.hpp"
#include "../../network/abstractproperty.hpp"
#include "../../mainwindow.hpp"

ThrottleObjectListWidget::ThrottleObjectListWidget(const ObjectPtr& object, QWidget* parent)
  : ObjectListWidget(object, parent)
{
  toolbar()->addSeparator();
  m_actionThrottle = toolbar()->addAction(Locale::tr("throttle_object_list:throttle"),
    [this]()
    {
      for(const QString& id : getSelectedObjectIds())
        MainWindow::instance->showObject(id, "", SubWindowType::Throttle);
    });
  m_actionThrottle->setEnabled(false);
}

void ThrottleObjectListWidget::tableSelectionChanged(bool hasSelection)
{
  ObjectListWidget::tableSelectionChanged(hasSelection);
  m_actionThrottle->setEnabled(hasSelection);
}

void ThrottleObjectListWidget::objectDoubleClicked(const QString& id)
{
  if(object())
  {
    if(const auto& c = object()->connection())
    {
      if(const auto& w = c->world())
      {
        if(auto* p = w->getProperty("edit"); p && !p->toBool())
        {
          MainWindow::instance->showObject(id, "", SubWindowType::Throttle);
          return;
        }
      }
    }
  }

  ObjectListWidget::objectDoubleClicked(id);
}
