/**
 * client/src/widget/alertwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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

#include "alertwidget.hpp"
#include <QPainter>

AlertWidget* AlertWidget::error(const QString& text, QWidget* parent)
{
  AlertWidget* w = new AlertWidget(parent);
  w->setType(Type::Error);
  w->setText(text);
  return w;
}

AlertWidget::AlertWidget(QWidget* parent) :
  QWidget(parent),
  m_type{Type::Error}
{
  setMinimumWidth(100);
  setMinimumHeight(qRound(fontMetrics().height() * 1.25));

  updateColors();
}

void AlertWidget::setType(Type value)
{
  if(m_type != value)
  {
    m_type = value;
    updateColors();
    repaint();
  }
}

void AlertWidget::setText(const QString& value)
{
  if(m_text != value)
  {
    m_text = value;
    repaint();
  }
}

void AlertWidget::paintEvent(QPaintEvent*)
{
  const qreal radius = 7;
  const int textMargin = 5;

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);

  QPainterPath path;
  path.addRoundedRect(rect(), radius, radius);
  painter.fillPath(path, m_backgroundColor);
  painter.setPen(m_borderColor);
  painter.drawPath(path);

  painter.setPen(m_textColor);
  painter.drawText(rect().adjusted(textMargin, 0, -textMargin, 0), Qt::AlignLeft | Qt::AlignVCenter, m_text);
}

void AlertWidget::updateColors()
{
  switch(m_type)
  {
    case Type::Error:
      m_borderColor = Qt::red;
      m_backgroundColor = Qt::gray;
      m_textColor = Qt::red;
      break;
  }
}
