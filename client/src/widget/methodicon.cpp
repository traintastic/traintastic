/**
 * client/src/widget/methodicon.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024-2025 Reinder Feenstra
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

#include "methodicon.hpp"
#include <QMouseEvent>
#include <QIcon>
#include "../network/method.hpp"

MethodIcon::MethodIcon(Method& method, QIcon icon, QWidget* parent) :
  QLabel(parent),
  m_method{method}
{
  setPixmap(icon.pixmap(32, 32));
  setCursor(Qt::PointingHandCursor);
  setEnabled(m_method.getAttributeBool(AttributeName::Enabled, true));
  setVisible(m_method.getAttributeBool(AttributeName::Visible, true));
  setToolTip(m_method.displayName());
  connect(&m_method, &Method::attributeChanged, this,
    [this](AttributeName name, const QVariant& value)
    {
      switch(name)
      {
        case AttributeName::Enabled:
          setEnabled(value.toBool());
          break;

        case AttributeName::Visible:
          setVisible(value.toBool());
          break;

        case AttributeName::DisplayName:
          setToolTip(m_method.displayName());
          break;

        default:
          break;
      }
    });
}

MethodIcon::MethodIcon(Method& item, QIcon icon, std::function<void()> triggered, QWidget* parent)
  : MethodIcon(item, icon, parent)
{
  m_triggered = std::move(triggered);
}

void MethodIcon::mousePressEvent(QMouseEvent* event)
{
  if(event->button() == Qt::LeftButton)
  {
    m_mouseLeftButtonPressed = true;
  }
}

void MethodIcon::mouseReleaseEvent(QMouseEvent* event)
{
  if(m_mouseLeftButtonPressed && event->button() == Qt::LeftButton)
  {
    m_mouseLeftButtonPressed = false;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if(rect().contains(event->localPos().toPoint())) // test if mouse release in widget
#else
    if(rect().contains(event->position().toPoint())) // test if mouse release in widget
#endif
    {
      if(m_triggered)
      {
        m_triggered();
      }
      else
      {
        m_method.call();
      }
    }
  }
}
