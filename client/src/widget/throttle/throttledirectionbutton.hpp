/**
 * client/src/widget/throttle/throttledirectionbutton.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_WIDGET_THROTTLE_THROTTLEDIRECTIONBUTTON_HPP
#define TRAINTASTIC_CLIENT_WIDGET_THROTTLE_THROTTLEDIRECTIONBUTTON_HPP

#include "abstractthrottlebutton.hpp"
#include <traintastic/enum/direction.hpp>

class AbstractProperty;

class ThrottleDirectionButton : public AbstractThrottleButton
{
  Q_OBJECT

  private:
    const Direction m_direction;
    AbstractProperty* m_directionProperty;

  private slots:
    void directionChanged();

  public:
    ThrottleDirectionButton(ObjectPtr object, Direction direction, QWidget* parent = nullptr);

    void click() final;
};

#endif
