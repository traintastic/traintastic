/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#include "throttlebutton.hpp"
#include <QPainter>
#include <QMouseEvent>
#include <QSvgRenderer>
#include <QFile>
#include <traintastic/enum/decoderfunctiontype.hpp>
#include <traintastic/enum/decoderfunctionfunction.hpp>
#include "throttlestyle.hpp"

ThrottleButton::ThrottleButton(QWidget* parent)
  : QWidget(parent)
  , m_color{ThrottleStyle::buttonColor}
{
  setMinimumHeight(32);
}

ThrottleButton::ThrottleButton(const QString& text_, QWidget* parent)
  : ThrottleButton(parent)
{
  m_text = text_;
}

void ThrottleButton::paintEvent(QPaintEvent*)
{
  const double borderRadius = rect().height() * 0.3;
  const QRectF buttonRect = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);

  QPainter painter{this};
  painter.setRenderHint(QPainter::Antialiasing, true);

  // Draw button:
  QColor c = m_color;
  if(!isEnabled())
  {
    c = ThrottleStyle::buttonDisabledColor;
  }
  painter.setPen(c);
  painter.setBrush(c);
  painter.drawRoundedRect(buttonRect, borderRadius, borderRadius);

  // Draw icon or text:
  if(!m_resource.isEmpty() && QFile::exists(m_resource))
  {
    QSvgRenderer svg{m_resource};
    const double iconSize = qMin(rect().width(), rect().height()) - qCeil(borderRadius);
    svg.render(&painter, QRectF((rect().width() - iconSize) / 2, (rect().height() - iconSize) / 2, iconSize, iconSize));
  }
  else
  {
    painter.setPen(ThrottleStyle::buttonTextColor);
    painter.drawText(rect(), Qt::AlignCenter, m_text);
  }
}

void ThrottleButton::mousePressEvent(QMouseEvent* event)
{
  m_mousePressPos = event->pos();
  emit pressed();
}

void ThrottleButton::mouseReleaseEvent(QMouseEvent* event)
{
  emit released();
  if((m_mousePressPos - event->pos()).manhattanLength() < 3)
  {
    emit clicked();
  }
}

QSize ThrottleButton::minimumSizeHint() const
{
  if(!m_resource.isEmpty() && QFile::exists(m_resource))
  {
    return QSize{minimumHeight(), minimumHeight()};
  }
  const int w = std::max(minimumHeight(), minimumHeight() / 2 + fontMetrics().boundingRect(m_text).width());
  return QSize{w, minimumHeight()};
}

const QColor& ThrottleButton::color() const
{
  return m_color;
}

void ThrottleButton::setColor(const QColor& value)
{
  if(m_color != value)
  {
    m_color = value;
    update();
  }
}

const QString& ThrottleButton::resource() const
{
  return m_resource;
}

void ThrottleButton::setResource(const QString& value)
{
  if(m_resource != value)
  {
    m_resource = value;
    updateGeometry();
    update();
  }
}

const QString& ThrottleButton::text() const
{
  return m_text;
}

void ThrottleButton::setText(const QString& value)
{
  if(m_text != value)
  {
    m_text = value;
    updateGeometry();
    update();
  }
}
