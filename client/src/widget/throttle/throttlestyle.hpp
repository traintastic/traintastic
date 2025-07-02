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

#ifndef TRAINTASTIC_CLIENT_WIDGET_THROTTLE_THROTTLESTYLE_HPP
#define TRAINTASTIC_CLIENT_WIDGET_THROTTLE_THROTTLESTYLE_HPP

#include <QColor>

struct ThrottleStyle
{
  inline static const QColor backgroundColor{0x10, 0x10, 0x10};
  inline static const QColor buttonColor{0xBB, 0x86, 0xFC};
  inline static const QColor buttonActiveColor{0x03, 0xDA, 0xC6};
  inline static const QColor buttonDisabledColor{0x80, 0x80, 0x80};
  inline static const QColor buttonEStopColor{0xCF, 0x66, 0x79};
  inline static const QColor buttonTextColor{0x00, 0x00, 0x00};
};

#endif
