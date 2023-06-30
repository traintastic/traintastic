/**
 * client/src/widget/throttle/speedometerwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#include "speedometerwidget.hpp"
#include <cmath>
#include <QPainter>

SpeedoMeterWidget::SpeedoMeterWidget(QWidget* parent)
  : QWidget(parent)
  , m_eStop{false}
  , m_speed{0.0}
  , m_speedMax{1.0}
  , m_speedLimit{std::numeric_limits<double>::quiet_NaN()}
  , m_speedTarget{std::numeric_limits<double>::quiet_NaN()}
  , m_unit{""}
{
}

void SpeedoMeterWidget::setEStop(bool value)
{
  if(m_eStop != value)
  {
    m_eStop = value;
    update();
  }
}

void SpeedoMeterWidget::setSpeed(double value)
{
  if(m_speed != value)
  {
    m_speed = value;
    update();
  }
}

void SpeedoMeterWidget::setSpeedMax(double value)
{
  if(m_speedMax != value)
  {
    m_speedMax = value;
    update();
  }
}

void SpeedoMeterWidget::setSpeedLimit(double value)
{
  if(m_speedLimit != value)
  {
    m_speedLimit = value;
    update();
  }
}

void SpeedoMeterWidget::setSpeedTarget(double value)
{
  if(m_speedTarget != value)
  {
    m_speedTarget = value;
    update();
  }
}

void SpeedoMeterWidget::setUnit(QString value)
{
  if(m_unit != value)
  {
    m_unit = std::move(value);
    update();
  }
}

void SpeedoMeterWidget::paintEvent(QPaintEvent*)
{
  const QColor borderColor{0xFF, 0xFF, 0xFF};
  const QColor barColorOn{0x00, 0xFF, 0xFF};
  const QColor barColorOff{0x00, 0x40, 0x40};
  const QColor barColorLimit{0x80, 0x00, 0x00};
  const QColor barColorLimitOverride{0xFF, 0x00, 0x00};
  const QColor barColorTarget{0xFF, 0xFF, 0xFF};

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);

  const double size = qMin(rect().width(), rect().height()) * 0.95;
  QRectF borderRect{size * 0.025, size * 0.025, size, size};
  painter.setPen(QPen(borderColor, size / 100));
  painter.drawArc(borderRect, 0 * 16, 360 * 16);

  painter.save();

  constexpr qreal angleStart = -135;
  constexpr qreal angleEnd = 135;
  constexpr qreal angleStep = 2.5;
  constexpr int steps = qRound((angleEnd - angleStart) / angleStep);

  const QPointF p1(0, -size / 2 + size / 50);
  const QPointF p2(0, -size / 2 + size / 20);
  const qreal penWidth = size / 100;
  const int stepLimitOverride = std::isfinite(m_speedLimit) && (m_speed > m_speedLimit) ? qRound(m_speedLimit / m_speedMax * steps) : -1;
  const int stepLimit = std::isfinite(m_speedLimit) ? qRound((m_speed > m_speedLimit ? m_speed : m_speedLimit) / m_speedMax * steps) : -1;
  const int stepOff = !std::isfinite(m_speedLimit) || (m_speed < m_speedLimit) ? qRound(m_speed / m_speedMax * steps) : -1;
  const int stepTarget = std::isfinite(m_speedTarget) ? qRound(m_speedTarget / m_speedMax * steps) : -1;

  painter.translate(borderRect.center()); // circle center is now 0,0
  painter.rotate(angleStart);
  painter.setPen(QPen(barColorOn, penWidth));

  for(int i = 0; i <= steps; i++)
  {
    if(i == stepLimitOverride)
      painter.setPen(QPen(barColorLimitOverride, penWidth));
    else if(i == stepLimit)
      painter.setPen(QPen(barColorLimit, penWidth));
    else if(i == stepOff)
      painter.setPen(QPen(barColorOff, penWidth));

    if(i == stepTarget)
    {
      painter.save();
      painter.setPen(QPen(barColorTarget, penWidth));
    }

    painter.drawLine(p1, p2);

    if(i == stepTarget)
      painter.restore();

    painter.rotate(angleStep);
  }

  painter.restore();

  QFont font = painter.font();

  font.setPixelSize(size / 4);
  painter.setFont(font);
  painter.drawText(borderRect, Qt::AlignCenter, QString::number(qRound(m_speed)));

  if(!m_unit.isEmpty())
  {
    font.setPixelSize(size / 8);
    painter.setFont(font);
    painter.drawText(borderRect.adjusted(0, size / 2, 0, 0), Qt::AlignCenter, m_unit);
  }

  if(m_eStop)
  {
    const auto eStop = QStringLiteral("ESTOP");
    font.setPixelSize(size / 12);
    painter.setFont(font);
    QRect r = painter.fontMetrics().boundingRect(eStop);
    r.moveTo(borderRect.left() + (borderRect.width() - r.width()) / 2, borderRect.top() + (borderRect.height() / 2 - r.height()) / 2);
    const int margin = font.pixelSize() / 6;
    r.adjust(-margin, -margin, margin, margin);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::red);
    const qreal radius = r.height() / 4.;
    painter.drawRoundedRect(r, radius, radius);
    painter.setPen(Qt::white);
    painter.drawText(r, Qt::AlignCenter, eStop);
  }
}
