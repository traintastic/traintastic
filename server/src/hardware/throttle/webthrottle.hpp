/**
 * server/src/hardware/throttle/webthrottle.hpp
 *
 * This file is part of the traintastic source code.
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_THROTTLE_WEBTHROTTLE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_THROTTLE_WEBTHROTTLE_HPP

#include "throttle.hpp"
#include "throttlecontroller.hpp"

class WebThrottle : public Throttle
{
  CLASS_ID("throttle.web")
  DEFAULT_ID("webthrottle")

protected:
  void load(WorldLoader& loader, const nlohmann::json& data) override;
  void save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const override;

public:
  static std::shared_ptr<WebThrottle> create(World& world);
  static std::shared_ptr<WebThrottle> create(World& world, std::string_view objectId);

  WebThrottle(World& world, std::string_view objectId);

  using Throttle::acquire;
};

#endif
