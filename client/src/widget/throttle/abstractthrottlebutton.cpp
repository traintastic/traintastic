/**
 * client/src/widget/throttle/abstractthrottlebutton.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#include "abstractthrottlebutton.hpp"
#include <QPainter>
#include <QMouseEvent>
#include <QSvgRenderer>
#include <QFile>
#include <traintastic/enum/decoderfunctiontype.hpp>
#include <traintastic/enum/decoderfunctionfunction.hpp>
#include "../../network/object.hpp"
#include "../../network/abstractproperty.hpp"

AbstractThrottleButton::AbstractThrottleButton(ObjectPtr object, QWidget* parent)
  : QWidget(parent)
  , m_object{std::move(object)}
  , m_textColor{Qt::white}
{
}

void AbstractThrottleButton::paintEvent(QPaintEvent*)
{
  QPainter painter{this};
  painter.setRenderHint(QPainter::Antialiasing, true);

  painter.setPen(Qt::white);

  const double size = qMin(rect().width(), rect().height());
  const double borderWidth = size / 24;
  const double borderRadius = size / 6;
  QRectF borderRect{borderWidth / 2, borderWidth / 2, size - borderWidth, size - borderWidth};
  painter.setPen(QPen(Qt::white, borderWidth));
  painter.drawRoundedRect(borderRect, borderRadius, borderRadius);

  if(!m_resource.isEmpty() && QFile::exists(m_resource))
  {
    QSvgRenderer svg{m_resource};
    const double iconSize = size * 0.75;
    svg.render(&painter, QRectF((borderRect.width() - iconSize) / 2, (borderRect.height() - iconSize) / 2, iconSize, iconSize));
  }
  else
  {
    painter.setPen(m_textColor);
    painter.drawText(borderRect, Qt::AlignCenter, m_text);
  }
}

void AbstractThrottleButton::mousePressEvent(QMouseEvent* event)
{
  m_mousePressPos = event->pos();
}

void AbstractThrottleButton::mouseReleaseEvent(QMouseEvent* event)
{
  if((m_mousePressPos - event->pos()).manhattanLength() < 3)
    click();
}

void AbstractThrottleButton::setResource(QString resource)
{
  m_resource = std::move(resource);
  update();
}

void AbstractThrottleButton::setText(QString text)
{
  m_text = std::move(text);
  update();
}

void AbstractThrottleButton::setTextColor(QColor color)
{
  m_textColor = color;
  update();
}
