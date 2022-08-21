/**
 * server/src/os/unix/signals.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#include "signals.hpp"
#include <csignal>
#include "../../log/log.hpp"
#include "../../traintastic/traintastic.hpp"

static void signalHandler(int signum)
{
  switch(signum)
  {
    case SIGINT:
    case SIGQUIT:
    {
      signal(SIGINT, SIG_DFL);
      signal(SIGQUIT, SIG_DFL);

      Log::log(*Traintastic::instance, LogMessage::N1001_RECEIVED_SIGNAL_X, std::string_view{strsignal(signum)});
      Traintastic::instance->exit();
      break;
    }
  }
}

namespace Unix {

void setupSignalHandlers()
{
  signal(SIGINT, signalHandler);
  signal(SIGQUIT, signalHandler);
}

}
