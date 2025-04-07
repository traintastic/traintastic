/**
 * client/src/widget/propertypairoutputaction.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "propertypairoutputaction.hpp"
#include <QKeyEvent>
#include <QPainter>
#include "../network/property.hpp"

PropertyPairOutputAction::PropertyPairOutputAction(Property& property, QWidget* parent)
  : QWidget(parent)
  , m_property{property}
{
  assert(m_property.enumName() == "pair_output_action");

  setFocusPolicy(Qt::StrongFocus);

  connect(&m_property, &Property::valueChanged, this,
    [this]()
    {
      update(rect());
    });
}

PairOutputAction PropertyPairOutputAction::value() const
{
  return m_property.toEnum<PairOutputAction>();
}

void PropertyPairOutputAction::setValue(PairOutputAction newValue)
{
  m_property.setValueEnum(newValue);
}

void PropertyPairOutputAction::toggleValue()
{
  switch(value())
  {
    case PairOutputAction::None:
      setValue(PairOutputAction::First);
      break;

    case PairOutputAction::First:
      setValue(PairOutputAction::Second);
      break;

    case PairOutputAction::Second:
    default:
      setValue(PairOutputAction::None);
      break;
  }
}

void PropertyPairOutputAction::toggleValue(PairOutputAction action)
{
  setValue(action == value() ? PairOutputAction::None : action);
}

void PropertyPairOutputAction::keyPressEvent(QKeyEvent* event)
{
  switch(event->key())
  {
    case Qt::Key_Enter:
    case Qt::Key_Space:
      toggleValue();
      return;

    case Qt::Key_1:
    case Qt::Key_R:
      toggleValue(PairOutputAction::First);
      return;

    case Qt::Key_2:
    case Qt::Key_G:
      toggleValue(PairOutputAction::Second);
      return;
  }
  QWidget::keyPressEvent(event);
}

void PropertyPairOutputAction::mousePressEvent(QMouseEvent* event)
{
  if(event->button() == Qt::LeftButton)
  {
    m_mouseLeftClickPos = event->pos();
  }
}

void PropertyPairOutputAction::mouseReleaseEvent(QMouseEvent* event)
{
  if(event->button() == Qt::LeftButton && m_mouseLeftClickPos)
  {
    const auto [first, second] = outputRects();

    if(first.contains(*m_mouseLeftClickPos) && first.contains(event->pos()))
    {
      toggleValue(PairOutputAction::First);
    }
    else if(second.contains(*m_mouseLeftClickPos) && second.contains(event->pos()))
    {
      toggleValue(PairOutputAction::Second);
    }

    m_mouseLeftClickPos.reset();
  }
}

void PropertyPairOutputAction::paintEvent(QPaintEvent* /*event*/)
{
  constexpr int thinkness = 2;
  const QColor firstOnColor(Qt::red);
  const QColor secondOnColor(Qt::green);
  const auto textOnColor = palette().color(QPalette::Active, QPalette::WindowText);
  const auto offColor = palette().color(QPalette::Disabled, QPalette::WindowText);
  const auto [left, right] = outputRects();

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);

  painter.save();

  QPen p(painter.pen());
  p.setWidth(thinkness);

  p.setColor(value() == PairOutputAction::First ? firstOnColor : offColor);
  painter.setPen(p);
  painter.drawEllipse(left.adjusted(thinkness, thinkness, -thinkness, -thinkness));

  p.setColor(value() == PairOutputAction::Second ? secondOnColor : offColor);
  painter.setPen(p);
  painter.drawEllipse(right.adjusted(thinkness, thinkness, -thinkness, -thinkness));

  painter.restore();

  painter.setPen(value() == PairOutputAction::First ? textOnColor : offColor);
  painter.drawText(left, Qt::AlignCenter, "R");

  painter.setPen(value() == PairOutputAction::Second ? textOnColor : offColor);
  painter.drawText(right, Qt::AlignCenter, "G");
}

std::pair<QRect, QRect> PropertyPairOutputAction::outputRects() const
{
  constexpr int margin = 1;
  const int height = rect().height();
  const int hCenter = rect().width() / 2;
  return {
    {hCenter - margin - height, 0, height, height},
    {hCenter + margin, 0, height, height}};
}
