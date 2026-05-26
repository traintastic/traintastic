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

#ifndef TRAINTASTIC_SERVER_THROTTLE_CLIENTTHROTTLE_HPP
#define TRAINTASTIC_SERVER_THROTTLE_CLIENTTHROTTLE_HPP

#include "throttle.hpp"

class ClientThrottle : public Throttle
{
  CLASS_ID("throttle.client")

private:
  Method<int32_t(const std::shared_ptr<Train>&, bool)> m_acquire;
  Method<void(bool)> m_release;
  Method<bool()> m_emergencyStop;
  Method<bool(Direction)> m_setDirection;
  Method<bool(double, SpeedUnit, bool)> m_setSpeed;
  Method<bool(bool)> m_faster;
  Method<bool(bool)> m_slower;

public:
  static std::shared_ptr<ClientThrottle> create(World& world);

  ClientThrottle(World& world, std::string_view objectId);
};

#endif
