/**
 * shared/src/enum/logmessage.hpp
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_LOGMESSAGE_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_LOGMESSAGE_HPP

#include <cstdint>

struct LogMessageOffset
{
  static constexpr uint32_t blockSize = 1'000'000;
  static constexpr uint32_t debug = 0 * blockSize;
  static constexpr uint32_t info = 1 * blockSize;
  static constexpr uint32_t notice = 2 * blockSize;
  static constexpr uint32_t warning = 3 * blockSize;
  static constexpr uint32_t error = 4 * blockSize;
  static constexpr uint32_t critical = 5 * blockSize;
  static constexpr uint32_t fatal = 6 * blockSize;
};

/**
 * 1xxx = general
 * 2xxx = hardware
 * 3xxx
 * 4xxx
 * 5xxx
 * 6xxx
 * 7xxx
 * 8xxx
 * 9xxx = lua scripting
 */
enum class LogMessage : uint32_t
{
  // Debug:
  D2001_TX_X = LogMessageOffset::debug + 2001,
  D2002_RX_X = LogMessageOffset::debug + 2002,
  D2003_UNKNOWN_XHEADER_0XX = LogMessageOffset::debug + 2003,
  D2004_X_TX_X = LogMessageOffset::debug + 2004,
  D2005_X_RX_X = LogMessageOffset::debug + 2005,
  D2006_UNKNOWN_MESSAGE_X = LogMessageOffset::debug + 2006,
  D2007_INPUT_X_IS_X = LogMessageOffset::debug + 2007,
  D2008_OUTPUT_X_IS_X = LogMessageOffset::debug + 2008,
  D2009_SLOT_X_IS_X = LogMessageOffset::debug + 2009,
  D2010_SLOT_X_IS_FREE = LogMessageOffset::debug + 2010,
  D9999_X = LogMessageOffset::debug + 9999,

  // Info:
  I1001_TRAINTASTIC_VX_X = LogMessageOffset::info + 1001,
  I1002_SETTING_FILE_NOT_FOUND_USING_DEFAULTS = LogMessageOffset::info + 1002,
  I1003_CLIENT_CONNECTED = LogMessageOffset::info + 1003,
  I1004_CONNECTION_LOST = LogMessageOffset::info + 1004,
  I1005_BUILDING_WORLD_INDEX = LogMessageOffset::info + 1005,
  I2001_UNKNOWN_LOCO_ADDRESS_X = LogMessageOffset::info + 2001,
  I9999_X = LogMessageOffset::info + 9999,

  // Notice:
  N1001_RECEIVED_SIGNAL_X = LogMessageOffset::notice + 1001,
  N1002_CREATED_NEW_WORLD = LogMessageOffset::notice + 1002,
  N1003_RESTARTING = LogMessageOffset::notice + 1003,
  N1004_SHUTTING_DOWN = LogMessageOffset::notice + 1004,
  N1005_DISCOVERY_ENABLED = LogMessageOffset::notice + 1005,
  N1006_DISCOVERY_DISABLED = LogMessageOffset::notice + 1006,
  N1007_LISTENING_AT_X_X = LogMessageOffset::notice + 1007,
  N1008_LOADED_SETTINGS = LogMessageOffset::notice + 1008,
  N1009_SAVED_SETTINGS = LogMessageOffset::notice + 1009,
  N1010_EDIT_MODE_ENABLED = LogMessageOffset::notice + 1010,
  N1011_EDIT_MODE_DISABLED = LogMessageOffset::notice + 1011,
  N1012_COMMUNICATION_ENABLED = LogMessageOffset::notice + 1012,
  N1013_COMMUNICATION_DISABLED = LogMessageOffset::notice + 1013,
  N1014_POWER_ON = LogMessageOffset::notice + 1014,
  N1015_POWER_OFF = LogMessageOffset::notice + 1015,
  N1016_RUNNING = LogMessageOffset::notice + 1016,
  N1017_STOPPED = LogMessageOffset::notice + 1017,
  N1018_MUTE_ENABLED = LogMessageOffset::notice + 1018,
  N1019_MUTE_DISABLED = LogMessageOffset::notice + 1019,
  N1020_SMOKE_ENABLED = LogMessageOffset::notice + 1020,
  N1021_SMOKE_DISABLED = LogMessageOffset::notice + 1021,
  N1022_SAVED_WORLD_X = LogMessageOffset::notice + 1022,
  N9999_X = LogMessageOffset::notice + 9999,

  // Warning:
  W1001_DISCOVERY_DISABLED_ONLY_ALLOWED_ON_PORT_X = LogMessageOffset::warning + 1001,
  W1002_SETTING_X_DOESNT_EXIST = LogMessageOffset::warning + 1002,
  W2001_RECEIVED_MALFORMED_DATA_DROPPED_X_BYTES = LogMessageOffset::warning + 2001,
  W2002_COMMAND_STATION_DOESNT_SUPPORT_FUNCTIONS_ABOVE_FX = LogMessageOffset::warning + 2001,
  W2003_COMMAND_STATION_DOESNT_SUPPORT_X_SPEEDSTEPS_USING_X = LogMessageOffset::warning + 2002,
  W9999_X = LogMessageOffset::warning + 9999,

  // Error:
  E1001_INVALID_WORLD_UUID_X = LogMessageOffset::error + 1001,
  E1002_WORLD_X_DOESNT_EXIST = LogMessageOffset::error + 1002,
  E1003_UDP_RECEIVE_ERROR_X = LogMessageOffset::error + 1003,
  E1004_TCP_ACCEPT_ERROR_X = LogMessageOffset::error + 1004,
  E1005_SOCKET_SHUTDOWN_FAILED_X = LogMessageOffset::error + 1005,
  E1006_SOCKET_WRITE_FAILED_X = LogMessageOffset::error + 1006,
  E1007_SOCKET_READ_FAILED_X = LogMessageOffset::error + 1007,
  E1008_SOCKET_ACCEPTOR_CANCEL_FAILED_X = LogMessageOffset::error + 1008,
  E2001_SERIAL_WRITE_FAILED_X = LogMessageOffset::error + 2001,
  E2002_SERIAL_READ_FAILED_X = LogMessageOffset::error + 2002,
  E2003_MAKE_ADDRESS_FAILED_X = LogMessageOffset::error + 2003,
  E2004_SOCKET_OPEN_FAILED_X = LogMessageOffset::error + 2004,
  E2005_SOCKET_CONNECT_FAILED_X = LogMessageOffset::error + 2005,
  E2006_SOCKET_BIND_FAILED_X = LogMessageOffset::error + 2006,
  E2007_SOCKET_WRITE_FAILED_X = LogMessageOffset::error + 2007,
  E2008_SOCKET_READ_FAILED_X = LogMessageOffset::error + 2008,
  E2009_SOCKET_RECEIVE_FAILED_X = LogMessageOffset::error + 2009,
  E2010_SERIAL_PORT_OPEN_FAILED_X = LogMessageOffset::error + 2010,
  E2011_SOCKET_SEND_FAILED_X = LogMessageOffset::error + 2011,
  E9999_X = LogMessageOffset::error + 9999,

  // Critical:
  C1001_LOADING_WORLD_FAILED_X = LogMessageOffset::critical + 1001,
  C1002_CREATING_CLIENT_FAILED_X = LogMessageOffset::critical + 1002,
  C1003_CANT_WRITE_TO_SETTINGS_FILE_X = LogMessageOffset::critical + 1003,
  C1004_READING_WORLD_FAILED_X_X = LogMessageOffset::critical + 1004,
  C1005_SAVING_WORLD_FAILED_X = LogMessageOffset::critical + 1005,
  C2001_ADDRESS_ALREADY_USED_AT_X = LogMessageOffset::critical + 2001,
  C2002_DCCPLUSPLUS_ONLY_SUPPORTS_THE_DCC_PROTOCOL = LogMessageOffset::critical + 2001,
  C2003_DCCPLUSPLUS_DOESNT_SUPPORT_DCC_LONG_ADDRESSES_BELOW_128 = LogMessageOffset::critical + 2002,
  C9999_X = LogMessageOffset::critical + 9999,

  // Fatal:
  F1001_OPENING_TCP_SOCKET_FAILED_X = LogMessageOffset::fatal + 1001,
  F1002_TCP_SOCKET_ADDRESS_REUSE_FAILED_X = LogMessageOffset::fatal + 1002,
  F1003_BINDING_TCP_SOCKET_FAILED_X = LogMessageOffset::fatal + 1003,
  F1004_TCP_SOCKET_LISTEN_FAILED_X = LogMessageOffset::fatal + 1004,
  F1005_OPENING_UDP_SOCKET_FAILED_X = LogMessageOffset::fatal + 1005,
  F1006_UDP_SOCKET_ADDRESS_REUSE_FAILED_X = LogMessageOffset::fatal + 1006,
  F1007_BINDING_UDP_SOCKET_FAILED_X = LogMessageOffset::fatal + 1007,
  F9001_CREATING_LUA_STATE_FAILED = LogMessageOffset::fatal + 9001,
  F9002_RUNNING_SCRIPT_FAILED_X = LogMessageOffset::fatal + 9002,
  F9003_CALLING_FUNCTION_FAILED_X = LogMessageOffset::fatal + 9003,
  F9999_X = LogMessageOffset::fatal + 9999,
};

constexpr bool isDebugLogMessage(LogMessage message)
{
  return
    (static_cast<std::underlying_type_t<decltype(message)>>(message) > LogMessageOffset::debug) &&
    (static_cast<std::underlying_type_t<decltype(message)>>(message) < LogMessageOffset::debug + LogMessageOffset::blockSize);
}

constexpr bool isInfoLogMessage(LogMessage message)
{
  return
    (static_cast<std::underlying_type_t<decltype(message)>>(message) > LogMessageOffset::info) &&
    (static_cast<std::underlying_type_t<decltype(message)>>(message) < LogMessageOffset::info + LogMessageOffset::blockSize);
}

constexpr bool isNoticeLogMessage(LogMessage message)
{
  return
    (static_cast<std::underlying_type_t<decltype(message)>>(message) > LogMessageOffset::notice) &&
    (static_cast<std::underlying_type_t<decltype(message)>>(message) < LogMessageOffset::notice + LogMessageOffset::blockSize);
}

constexpr bool isWarningLogMessage(LogMessage message)
{
  return
    (static_cast<std::underlying_type_t<decltype(message)>>(message) > LogMessageOffset::warning) &&
    (static_cast<std::underlying_type_t<decltype(message)>>(message) < LogMessageOffset::warning + LogMessageOffset::blockSize);
}

constexpr bool isErrorLogMessage(LogMessage message)
{
  return
    (static_cast<std::underlying_type_t<decltype(message)>>(message) > LogMessageOffset::error) &&
    (static_cast<std::underlying_type_t<decltype(message)>>(message) < LogMessageOffset::error + LogMessageOffset::blockSize);
}

constexpr bool isCriticalLogMessage(LogMessage message)
{
  return
    (static_cast<std::underlying_type_t<decltype(message)>>(message) > LogMessageOffset::critical) &&
    (static_cast<std::underlying_type_t<decltype(message)>>(message) < LogMessageOffset::critical + LogMessageOffset::blockSize);
}

constexpr bool isFatalLogMessage(LogMessage message)
{
  return
    (static_cast<std::underlying_type_t<decltype(message)>>(message) > LogMessageOffset::fatal) &&
    (static_cast<std::underlying_type_t<decltype(message)>>(message) < LogMessageOffset::fatal + LogMessageOffset::blockSize);
}

constexpr char logMessageChar(LogMessage message)
{
  constexpr char chars[7] = {'D', 'I', 'N', 'W', 'E', 'C', 'F'};
  return chars[static_cast<std::underlying_type_t<decltype(message)>>(message) / LogMessageOffset::blockSize];
}

constexpr uint32_t logMessageNumber(LogMessage message)
{
  return static_cast<std::underlying_type_t<decltype(message)>>(message) % LogMessageOffset::blockSize;
}

#endif
