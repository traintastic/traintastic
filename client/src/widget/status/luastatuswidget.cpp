/**
 * client/src/widget/status/luastatuswidget.cpp
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

#include "luastatuswidget.hpp"
#include <QHBoxLayout>
#include <QSvgWidget>
#include <QLabel>
#include <QResizeEvent>
#include <traintastic/enum/interfacestate.hpp>
#include <traintastic/locale/locale.hpp>
#include "../../network/object.hpp"
#include "../../network/property.hpp"
#include "../../theme/theme.hpp"

LuaStatusWidget::LuaStatusWidget(const ObjectPtr& object, QWidget* parent)
  : QWidget(parent)
  , m_object{object}
  , m_svg{new QSvgWidget(this)}
  , m_runningLabel{new QLabel(this)}
  , m_errorLabel{new QLabel(this)}
{
  assert(m_object);
  assert(m_object->classId() == "status.lua");

  m_svg->load(Theme::getIconFile("lua"));
  m_errorLabel->setStyleSheet("QLabel { color: #DC143C; }");

  auto* l = new QHBoxLayout();
  l->setContentsMargins(0, 0, 0, 0);
  l->addWidget(m_svg);
  l->addWidget(m_runningLabel);
  l->addWidget(m_errorLabel);
  setLayout(l);

  if(auto* property = m_object->getProperty("label"); property)
    connect(property, &Property::valueChanged, this, &LuaStatusWidget::labelChanged);

  if(auto* property = m_object->getProperty("running"); property)
    connect(property, &Property::valueChanged, this, &LuaStatusWidget::runningChanged);

  if(auto* property = m_object->getProperty("error"); property)
    connect(property, &Property::valueChanged, this, &LuaStatusWidget::errorChanged);

  runningChanged();
  errorChanged();
}

void LuaStatusWidget::labelChanged()
{
  QString label;

  if(auto* property = m_object->getProperty("label"); property)
    label = Locale::instance->parse(property->toString());

  if(auto* property = m_object->getProperty("running"); property)
  {
    label.append("\n").append(Locale::tr("status.lua:x_running").arg(property->toInt()));
  }

  if(auto* property = m_object->getProperty("error"); property)
  {
    label.append("\n").append(Locale::tr("status.lua:x_in_error").arg(property->toInt()));
  }

  setToolTip(label);
}

void LuaStatusWidget::runningChanged()
{
  m_runningLabel->setText(QString::number(m_object->getPropertyValueInt("running", 0)));
  labelChanged();
}

void LuaStatusWidget::errorChanged()
{
  const int error = m_object->getPropertyValueInt("error", 0);
  m_errorLabel->setVisible(error > 0);
  if(m_errorLabel->isVisible())
    m_errorLabel->setText(QString("(%1)").arg(error));
  labelChanged();
}

void LuaStatusWidget::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);

  // force same height/width for image as height:
  const int sz = event->size().height();
  m_svg->setMinimumSize(sz, sz);
  m_svg->setMaximumSize(sz, sz);
}
