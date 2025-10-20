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

#include "tileimagewidget.hpp"
#include <QPainter>
#include "../../board/boardcolorscheme.hpp"
#include "../../board/getboardcolorscheme.hpp"
#include "../../board/tilepainter.hpp"
#include "../../settings/boardsettings.hpp"

TileImageWidget::TileImageWidget(QWidget* parent)
  : QWidget(parent)
  , m_colorScheme{getBoardColorScheme(BoardSettings::instance().colorScheme.value())}
{
  connect(&BoardSettings::instance(), &BoardSettings::changed, this, qOverload<>(&TileImageWidget::update));
}

int TileImageWidget::heightForWidth(int w) const
{
  return w;
}

bool TileImageWidget::hasHeightForWidth() const
{
  return true;
}

QSize TileImageWidget::minimumSizeHint() const
{
  return {64, 64};
}

void TileImageWidget::setTileId(TileId value)
{
  if(m_tileId != value)
  {
    m_tileId = value;
    update();
  }
}

void TileImageWidget::setTurnoutPosition(TurnoutPosition value)
{
  if(m_turnoutPosition != value)
  {
    m_turnoutPosition = value;
    update();
  }
}

void TileImageWidget::paintEvent(QPaintEvent* /*event*/)
{
  const int margin = 1;

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.fillRect(rect(), m_colorScheme->background);

  if(m_tileId != TileId::None)
  {
    const auto tileRect = rect().adjusted(margin, margin, -margin, -margin);
    painter.setClipRect(tileRect);

    TilePainter tilePainter(painter, tileRect.height(), *m_colorScheme);
    if(isRailTurnout(m_tileId))
    {
      tilePainter.drawTurnout(m_tileId, tileRect, TileRotate::Deg0, TurnoutPosition::Unknown, m_turnoutPosition);
    }
    else
    {
      tilePainter.draw(m_tileId, tileRect, TileRotate::Deg0);
    }
  }
}
