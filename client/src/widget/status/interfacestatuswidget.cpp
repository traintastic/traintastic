/**
 * client/src/widget/status/interfacestatuswidget.cpp
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

#include "interfacestatuswidget.hpp"
#include <QResizeEvent>
#include <traintastic/enum/interfacestate.hpp>
#include <traintastic/locale/locale.hpp>
#include "../../network/object.hpp"
#include "../../network/property.hpp"
#include "../../theme/theme.hpp"

InterfaceStatusWidget::InterfaceStatusWidget(const ObjectPtr& object, QWidget* parent)
  : QSvgWidget(parent)
  , m_object{object}
{
  assert(m_object);
  assert(m_object->classId() == "status.interface");

  if(auto* property = m_object->getProperty("label"); property)
    connect(property, &Property::valueChanged, this, &InterfaceStatusWidget::labelChanged);

  if(auto* property = m_object->getProperty("state"); property)
    connect(property, &Property::valueChanged, this, &InterfaceStatusWidget::stateChanged);

  stateChanged();
}

void InterfaceStatusWidget::labelChanged()
{
  QString label;

  if(auto* property = m_object->getProperty("label"); property)
    label = property->toString();

  if(auto* property = m_object->getProperty("state"); property)
  {
    auto state = property->toEnum<InterfaceState>();
    if(EnumValues<InterfaceState>::value.count(state) != 0)
      label.append(": ").append(Locale::tr(QString(EnumName<InterfaceState>::value).append(":").append(EnumValues<InterfaceState>::value.at(state))));
  }

  setToolTip(label);
}

void InterfaceStatusWidget::stateChanged()
{
  if(auto* property = m_object->getProperty("state"); property)
  {
    labelChanged();

    auto state = property->toEnum<InterfaceState>();
    if(EnumValues<InterfaceState>::value.count(state) != 0)
    {
      load(Theme::getIconFile(QString("interface_state.").append(EnumValues<InterfaceState>::value.at(state))));
      return;
    }
  }

  assert(false);
  load(QString(":/appicon.svg")); // show the Traintastic icon if something is missing
}

void InterfaceStatusWidget::resizeEvent(QResizeEvent* event)
{
  QSvgWidget::resizeEvent(event);

  // force same width as height:
  setMinimumWidth(event->size().height());
  setMaximumWidth(event->size().height());
}
