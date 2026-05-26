/**
 * client/src/misc/colorpool.cpp
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

#include "colorpool.hpp"

ColorPool::ColorPool(std::initializer_list<Color> colors)
{
  m_colors.reserve(colors.size());
  for(auto color : colors)
  {
    m_colors.emplace_back(std::make_pair(color, false));
  }
}

Color ColorPool::aquire()
{
  for(auto& item : m_colors)
  {
    if(!item.second)
    {
      item.second = true;
      return item.first;
    }
  }
  return Color::None;
}

void ColorPool::release(Color color)
{
  for(auto& item: m_colors)
  {
    if(item.first == color)
    {
      item.second = false;
      break;
    }
  }
}
