/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_THROTTLE_AUTOTHROTTLE_HPP
#define TRAINTASTIC_SERVER_THROTTLE_AUTOTHROTTLE_HPP

#include "throttle.hpp"
#include <traintastic/enum/autothrottlereason.hpp>
#include <traintastic/enum/autothrottlestate.hpp>

class BlockPath;
class Train;

class AutoThrottle : public Throttle
{
  CLASS_ID("throttle.auto")

public:
  static std::shared_ptr<AutoThrottle> assign(const std::shared_ptr<Train>& train, bool steal, std::error_code& ec);

  Property<AutoThrottleState> state;
  Property<AutoThrottleReason> reason;

protected:
  AutoThrottle(World& world, std::string_view objectId);

  void trainChanged() override;

private:
  std::vector<boost::signals2::scoped_connection> m_events;

  void changeState(AutoThrottleState newState, AutoThrottleReason newReason = AutoThrottleReason::None);

  void evaluate();
  void evaluateIdle();
  void evaluateDepart();
  void evaluateDrive();
  void evaluateArrive();

  void stop();
};

#endif
