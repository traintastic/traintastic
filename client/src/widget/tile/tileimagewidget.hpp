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

#ifndef TRAINTASTIC_CLIENT_WIDGET_TILE_TILEIMAGEWIDGET_HPP
#define TRAINTASTIC_CLIENT_WIDGET_TILE_TILEIMAGEWIDGET_HPP

#include <QWidget>
#include <traintastic/board/tileid.hpp>
#include <traintastic/enum/turnoutposition.hpp>

struct BoardColorScheme;

class TileImageWidget : public QWidget
{
private:
  const BoardColorScheme* m_colorScheme;
  TileId m_tileId = TileId::None;
  TurnoutPosition m_turnoutPosition = TurnoutPosition::Unknown;

protected:
  void paintEvent(QPaintEvent* event) final;

public:
  explicit TileImageWidget(QWidget* parent = nullptr);

  int heightForWidth(int w) const final;
  bool hasHeightForWidth() const final;
  QSize minimumSizeHint() const final;

  void setTileId(TileId value);
  void setTurnoutPosition(TurnoutPosition value);
};

#endif
