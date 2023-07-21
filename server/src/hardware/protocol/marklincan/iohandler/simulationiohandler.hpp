/**
 * server/src/hardware/protocol/marklincan/iohandler/simulationiohandler.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_IOHANDLER_SIMULATIONIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_IOHANDLER_SIMULATIONIOHANDLER_HPP

#include "iohandler.hpp"
#include <chrono>
#include <queue>
#include <boost/asio/steady_timer.hpp>
#include "../messages.hpp"

namespace MarklinCAN {

class SimulationIOHandler final : public IOHandler
{
  private:
    static constexpr auto defaultSwitchTime = std::chrono::milliseconds(1000);

    struct DelayedMessage
    {
      std::chrono::steady_clock::time_point time;
      Message message;
    };

    class DelayedMessageCompare
    {
      public:
        bool operator()(const DelayedMessage& a, const DelayedMessage& b)
        {
          return a.time >= b.time;
        }
    };

    std::chrono::milliseconds m_switchTime = defaultSwitchTime;
    std::priority_queue<DelayedMessage, std::vector<DelayedMessage>, DelayedMessageCompare> m_delayedMessages;
    boost::asio::steady_timer m_delayedMessageTimer;

    void reply(const Message& message);
    void reply(const Message& message, std::chrono::milliseconds delay);

    void restartDelayedMessageTimer();

  public:
    SimulationIOHandler(Kernel& kernel);

    void start() final {}
    void stop() final;

    bool send(const Message& message) final;
};

template<>
constexpr bool isSimulation<SimulationIOHandler>()
{
  return true;
}

}

#endif
