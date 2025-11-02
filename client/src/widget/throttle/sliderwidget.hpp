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

#ifndef TRAINTASTIC_CLIENT_WIDGET_THROTTLE_SLIDERWIDGET_HPP
#define TRAINTASTIC_CLIENT_WIDGET_THROTTLE_SLIDERWIDGET_HPP

#include <QWidget>

class SliderWidget final : public QWidget
{
  Q_OBJECT

private:
  static constexpr qreal handleSize = 12.0;

  Qt::Orientation m_orientation = Qt::Vertical;
  float m_minimum = 0.0f;
  float m_maximum = 1.0f;
  float m_value = 0.0f;
  bool m_dragging = false;

  void setSliderValueFromPosition(const QPoint& pos);

protected:
  void mousePressEvent(QMouseEvent* event) final;
  void mouseMoveEvent(QMouseEvent* event) final;
  void mouseReleaseEvent(QMouseEvent* event) final;
  void wheelEvent(QWheelEvent* event) final;
  void paintEvent(QPaintEvent* event) final;

public:
  SliderWidget(QWidget* parent = nullptr);
  SliderWidget(Qt::Orientation orientation, QWidget* parent = nullptr);

  float minimum() const;
  float maximum() const;
  float value() const;

  void setMinimum(float min);
  void setMaximum(float max);
  void setRange(float min, float max);
  void setValue(float val);

signals:
  void valueChanged(float val);
};

#endif
