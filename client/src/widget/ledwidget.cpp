/**
 * client/src/widget/ledwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2024 Reinder Feenstra
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

#include "ledwidget.hpp"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

LEDWidget::LEDWidget(const Colors& colors, QWidget* parent) :
  QWidget(parent),
  m_colors{colors},
  m_mouseLeftButtonPressed{false},
  m_enabled{true},
  m_state{State::Undefined}
{
  setMinimumWidth(fontMetrics().averageCharWidth() * 4);
  setMinimumHeight((fontMetrics().height() * 3) / 2);
}

void LEDWidget::setEnabled(bool value)
{
  if(m_enabled != value)
  {
    m_enabled = value;
    update();
  }
}

void LEDWidget::setState(State value)
{
  if(m_state != value)
  {
    m_state = value;
    update();
  }
}

void LEDWidget::setText(const QString& value)
{
  if(m_text != value)
  {
    m_text = value;
    update();
  }
}

void LEDWidget::mousePressEvent(QMouseEvent* event)
{
  if(event->button() == Qt::LeftButton)
    m_mouseLeftButtonPressed = true;
}

void LEDWidget::mouseReleaseEvent(QMouseEvent* event)
{
  if(m_mouseLeftButtonPressed && event->button() == Qt::LeftButton)
  {
    m_mouseLeftButtonPressed = false;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if(rect().contains(event->localPos().toPoint())) // test if mouse release in widget
#else
    if(rect().contains(event->position().toPoint())) // test if mouse release in widget
#endif
      emit clicked();
  }
}

void LEDWidget::paintEvent(QPaintEvent*)
{
  const double scale = qMax(1.0, qMin(static_cast<double>(height()) / minimumHeight(), static_cast<double>(width()) / minimumWidth()));
  const int marginV = (height() - minimumHeight() * scale) / 2;
  const int ledHeight = minimumHeight() * scale / 3;
  const int ledWidth = fontMetrics().averageCharWidth() * scale * 3;
  const int ledLeft = (width() - ledWidth) / 2;
  const int ledRadius = ledHeight / 2;

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);

  if(!m_text.isEmpty())
  {
    const QRect textRect = rect().adjusted(0, marginV + minimumHeight() / 2, 0, -marginV);
    painter.setPen(palette().color(m_enabled ? QPalette::Normal : QPalette::Disabled, QPalette::WindowText));
    QFont font = painter.font();
    font.setPointSize(font.pointSize() * scale);
    painter.setFont(font);
    painter.drawText(textRect, Qt::AlignCenter | Qt::AlignBaseline, m_text);
  }

  const QRect ledRect{ledLeft, marginV + 1, ledWidth, ledHeight};// = rect().adjusted(0, marginV, 0, -(marginV + minimumHeight() / 2));
  QPainterPath path;
  path.addRoundedRect(ledRect, ledRadius, ledRadius);
  switch(m_state)
  {
    case State::Undefined:
      break;

    case State::Off:
      painter.fillPath(path, m_colors.off);// QColor(0x20, 0x20, 0x20));//painter.fillPath(path, QColor(0x70, 0x80, 0x90));
      break;

    case State::On:
      painter.fillPath(path, m_colors.on);//QColor(0x00, 0xBF, 0xFF));
      break;
  }

  QPen pen = painter.pen();
  pen.setColor(m_colors.border);
  pen.setWidth(2);
  painter.setPen(pen);
  painter.drawPath(path);
}
