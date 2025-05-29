/**
 * server/src/throttle/hardwarethrottle.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023,2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_THROTTLE_HARDWARETHROTTLE_HPP
#define TRAINTASTIC_SERVER_THROTTLE_HARDWARETHROTTLE_HPP

#include "throttle.hpp"
#include "throttlecontroller.hpp"

class HardwareThrottle : public Throttle
{
  CLASS_ID("throttle.hardware")

  private:
    ThrottleController& throttleController();

  protected:
    void destroying() override;
    void addToList() override;

  public:
    static std::shared_ptr<HardwareThrottle> create(std::shared_ptr<ThrottleController> controller, World& world);
    static std::shared_ptr<HardwareThrottle> create(std::shared_ptr<ThrottleController> controller, World& world, std::string_view _id);

    ObjectProperty<ThrottleController> interface;

    HardwareThrottle(std::shared_ptr<ThrottleController> controller, World& world, std::string_view logId);

    AcquireResult acquire(DecoderProtocol protocol, uint16_t address, bool steal = false);
};

#endif
