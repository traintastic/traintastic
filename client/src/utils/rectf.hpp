/**
 * client/src/utils/rectf.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_UTILS_RECTF_HPP
#define TRAINTASTIC_CLIENT_UTILS_RECTF_HPP

#include <QRectF>

constexpr QPointF topCenter(const QRectF& r)
{
  return QPointF{r.center().x(), r.top()};
}

constexpr QPointF bottomCenter(const QRectF& r)
{
  return QPointF{r.center().x(), r.bottom()};
}

constexpr QPointF centerLeft(const QRectF& r)
{
  return QPointF{r.left(), r.center().y()};
}

constexpr QPointF centerRight(const QRectF& r)
{
  return QPointF{r.right(), r.center().y()};
}

#endif
