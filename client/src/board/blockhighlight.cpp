/**
 * client/src/board/blockhighlight.cpp
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

#include "blockhighlight.hpp"

BlockHighlight::BlockHighlight(QObject* parent)
  : QObject(parent)
  , colorPool{{Color::Fuchsia, Color::Green, Color::Blue, Color::Lime, Color::Purple, Color::Red, Color::Aqua}}
{
}

void BlockHighlight::add(const QString& blockId, Color color)
{
  auto& colors = m_blockColors[blockId];
  if(!colors.contains(color))
  {
    colors.append(color);
    emit colorsChanged(blockId, colors);
  }
}

void BlockHighlight::remove(const QString& blockId, Color color)
{
  if(auto it = m_blockColors.find(blockId); it != m_blockColors.end())
  {
    if(int index = it->indexOf(color); index >= 0)
    {
      it->remove(index);
      emit colorsChanged(blockId, *it);
      if(it->isEmpty())
      {
        m_blockColors.remove(blockId);
      }
    }
  }
}
