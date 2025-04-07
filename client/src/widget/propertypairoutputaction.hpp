/**
 * client/src/widget/propertypairoutputaction.hpp
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

#ifndef TRAINTASTIC_CLIENT_WIDGET_PROPERTYPAIROUTPUTACTION_HPP
#define TRAINTASTIC_CLIENT_WIDGET_PROPERTYPAIROUTPUTACTION_HPP

#include <optional>
#include <QWidget>
#include <traintastic/enum/pairoutputaction.hpp>

class Property;

class PropertyPairOutputAction : public QWidget
{
private:
  Property& m_property;
  std::optional<QPoint> m_mouseLeftClickPos = std::nullopt;

  std::pair<QRect, QRect> outputRects() const;

protected:
  void keyPressEvent(QKeyEvent* event) final;
  void mousePressEvent(QMouseEvent* event) final;
  void mouseReleaseEvent(QMouseEvent* event) final;
  void paintEvent(QPaintEvent* event) final;

public:
  PropertyPairOutputAction(Property& property, QWidget* parent = nullptr);

  PairOutputAction value() const;
  void setValue(PairOutputAction action);
  void toggleValue();
  void toggleValue(PairOutputAction output);
};

#endif
