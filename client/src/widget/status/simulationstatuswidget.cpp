/**
 * client/src/widget/status/simulationstatuswidget.cpp
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

#include "simulationstatuswidget.hpp"
#include <QResizeEvent>
#include <traintastic/locale/locale.hpp>
#include "../../network/object.hpp"
#include "../../network/property.hpp"
#include "../../theme/theme.hpp"

SimulationStatusWidget::SimulationStatusWidget(const ObjectPtr& object, QWidget* parent)
  : QSvgWidget(parent)
  , m_object{object}
{
  assert(m_object);
  assert(m_object->classId() == "status.simulation");

  load(Theme::getIconFile("simulation"));

  if(auto* property = m_object->getProperty("label"))
  {
    connect(property, &Property::valueChanged, this, &SimulationStatusWidget::labelChanged);
  }

  labelChanged();
}

void SimulationStatusWidget::labelChanged()
{
  QString label;

  if(auto* property = m_object->getProperty("label"))
  {
    label = Locale::instance->parse(property->toString());
  }

  setToolTip(label);
}

void SimulationStatusWidget::resizeEvent(QResizeEvent* event)
{
  QSvgWidget::resizeEvent(event);

  // force same width as height:
  setMinimumWidth(event->size().height());
  setMaximumWidth(event->size().height());
}
