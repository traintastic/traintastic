/**
 * client/src/board/blockhighlight.hpp
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

#ifndef TRAINTASTIC_CLIENT_BOARD_BLOCKHIGHLIGHT_HPP
#define TRAINTASTIC_CLIENT_BOARD_BLOCKHIGHLIGHT_HPP

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <traintastic/enum/color.hpp>
#include "../misc/colorpool.hpp"

class BlockHighlight : public QObject
{
  Q_OBJECT

public:
  using BlockColors = QMap<QString, QVector<Color>>;

private:
  BlockColors m_blockColors;

public:
  ColorPool colorPool;

  explicit BlockHighlight(QObject* parent);

  const BlockColors& blockColors() const
  {
    return m_blockColors;
  }

  void add(const QString& blockId, Color color);
  void remove(const QString& blockId, Color color);

signals:
  void colorsChanged(const QString& blockId, const QVector<Color>& colors);
};

#endif
