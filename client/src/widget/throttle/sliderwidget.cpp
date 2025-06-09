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

#include "sliderwidget.hpp"
#include <QPainter>
#include <QStyleOption>
#include <QPaintEvent>
#include <QDebug>

SliderWidget::SliderWidget(QWidget* parent)
  : QWidget(parent)
{
  setMinimumSize(40, 0);
  setMouseTracking(true);
}

SliderWidget::SliderWidget(Qt::Orientation orientation, QWidget* parent)
  : SliderWidget(parent)
{
  m_orientation = orientation;

  if(m_orientation == Qt::Horizontal)
  {
    setMinimumSize(0, 40);
  }
}

float SliderWidget::minimum() const
{
  return m_minimum;
}

float SliderWidget::maximum() const
{
  return m_maximum;
}

float SliderWidget::value() const
{
  return m_value;
}

void SliderWidget::setMinimum(float min)
{
  m_minimum = min;
  if(m_maximum < m_minimum)
  {
    m_maximum = m_minimum;
  }
  if(m_value < m_minimum)
  {
    m_value = m_minimum;
  }
  update();
}

void SliderWidget::setMaximum(float max)
{
  m_maximum = max;
  if(m_minimum > m_maximum)
  {
    m_minimum = m_maximum;
  }
  if(m_value > m_maximum)
  {
    m_value = m_maximum;
  }
  update();
}

void SliderWidget::setRange(float min, float max)
{
  m_minimum = min;
  m_maximum = max;
  if(m_value < m_minimum)
  {
    m_value = m_minimum;
  }
  else if(m_value > m_maximum)
  {
    m_value = m_maximum;
  }
  update();
}

void SliderWidget::setValue(float val)
{
  float clamped = std::clamp(val, m_minimum, m_maximum);
  if(qFuzzyCompare(m_value, clamped))
  {
    return;
  }
  m_value = clamped;
  update();
  emit valueChanged(m_value);
}

void SliderWidget::mousePressEvent(QMouseEvent* event)
{
  if(event->button() == Qt::LeftButton)
  {
    setSliderValueFromPosition(event->pos());
    m_dragging = true;
  }
}

void SliderWidget::mouseMoveEvent(QMouseEvent* event)
{
  if(m_dragging)
  {
    setSliderValueFromPosition(event->pos());
  }
}

void SliderWidget::mouseReleaseEvent(QMouseEvent* event)
{
  if(event->button() == Qt::LeftButton)
  {
    m_dragging = false;
  }
}

void SliderWidget::setSliderValueFromPosition(const QPoint& pos)
{
  float ratio;
  if(m_orientation == Qt::Vertical)
  {
    const auto height = rect().height() - 2 * handleSize;
    ratio = 1.0f - (float(pos.y() - handleSize) / height);
  }
  else
  {
    const auto width = rect().width() - 2 * handleSize;
    ratio = float(pos.x() - handleSize) / width;
  }
  setValue(m_minimum + std::clamp(ratio, 0.0f, 1.0f) * (m_maximum - m_minimum));
}

void SliderWidget::wheelEvent(QWheelEvent* event)
{
  const float step = 0.01f * (m_maximum - m_minimum); // 1% of range per step
  const float delta = event->angleDelta().y() > 0 ? step : -step;
  setValue(m_value + delta);
  event->accept();
}

void SliderWidget::paintEvent(QPaintEvent* /*event*/)
{
  constexpr qreal gutterSize = 5.0;
  constexpr qreal gutterRadius = (gutterSize + 1) / 2;
  constexpr qreal handleRadius = (handleSize + 1) / 2;

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // Draw gutter
  QRectF backgroundRect = rect();
  if(m_orientation == Qt::Vertical)
  {
    backgroundRect.adjust(0.5, handleSize / 2, -0.5, -handleSize / 2);
    backgroundRect.setLeft((backgroundRect.width() - gutterSize) / 2);
    backgroundRect.setWidth(9);
  }
  else
  {
    backgroundRect.adjust(handleSize / 2, 0.5, -handleSize / 2, -0.5);
    backgroundRect.setTop((backgroundRect.height() - gutterSize) / 2);
    backgroundRect.setHeight(9);
  }
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(33, 33, 33));
  painter.drawRoundedRect(backgroundRect, gutterRadius, gutterRadius);

  // Calculate handle position
  float ratio = (m_value - m_minimum) / (m_maximum - m_minimum);
  QRectF handleRect;

  if(m_orientation == Qt::Vertical)
  {
    const float height = rect().height() - 2 * handleSize;
    const float y = height * (1.0f - ratio);
    handleRect = QRectF(1, handleSize / 2 + y, width() - 2, handleSize);
  }
  else
  {
    const float width = rect().width() - 2 * handleSize;
    const float x = width * ratio;
    handleRect = QRectF(handleSize / 2 + x, 1, handleSize, height() - 2);
  }

  // Draw handle
  painter.setBrush(Qt::white);
  painter.drawRoundedRect(handleRect, handleRadius, handleRadius);
}
