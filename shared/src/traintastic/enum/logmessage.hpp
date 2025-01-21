/**
 * shared/src/enum/logmessage.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2024 Reinder Feenstra
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

#ifdef QT_CORE_LIB
  #include <QString>
#endif

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
 * 3xxx = trains
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
#ifdef ENABLE_LOG_DEBUG
  D0000_X = LogMessageOffset::debug,
#endif
  D1001_RESUME_X_MULTIPLIER_X = LogMessageOffset::debug + 1001,
  D1002_TICK_X_ERROR_X_US = LogMessageOffset::debug + 1002,
  D1003_FREEZE_X = LogMessageOffset::debug + 1003,
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
  I1001_TRAINTASTIC_VX = LogMessageOffset::info + 1001,
  I1002_SETTING_FILE_NOT_FOUND_USING_DEFAULTS = LogMessageOffset::info + 1002,
  I1003_NEW_CONNECTION = LogMessageOffset::info + 1003,
  I1004_CONNECTION_LOST = LogMessageOffset::info + 1004,
  I1005_BUILDING_WORLD_INDEX = LogMessageOffset::info + 1005,
  I1006_X = LogMessageOffset::info + 1006, //!< boost version
  I1007_X = LogMessageOffset::info + 1007, //!< nlohmann::json version
  I1008_X = LogMessageOffset::info + 1008, //!< LibArchive version
  I1009_ZLIB_X = LogMessageOffset::info + 1009, //!< zlib version
  I2001_UNKNOWN_LOCO_ADDRESS_X = LogMessageOffset::info + 2001,
  I2002_HARDWARE_TYPE_X = LogMessageOffset::info + 2002,
  I2003_FIRMWARE_VERSION_X = LogMessageOffset::info + 2003,
  I2004_HSI_88_X = LogMessageOffset::info + 2004,
  I2005_X = LogMessageOffset::info + 2005,
  I9001_STOPPED_SCRIPT = LogMessageOffset::info + 9001,
  I9002_X = LogMessageOffset::info + 9002, //!< Lua version
  I9003_CLEARED_PERSISTENT_VARIABLES = LogMessageOffset::info + 9003,
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
  N1023_SIMULATION_DISABLED = LogMessageOffset::notice + 1023,
  N1024_SIMULATION_ENABLED = LogMessageOffset::notice + 1024,
  N1025_EXPORTED_WORLD_SUCCESSFULLY = LogMessageOffset::notice + 1025,
  N1026_IMPORTED_WORLD_SUCCESSFULLY = LogMessageOffset::notice + 1026,
  N1027_LOADED_WORLD_X = LogMessageOffset::notice + 1027,
  N1028_CLOSED_WORLD = LogMessageOffset::notice + 1028,
  N2001_SIMULATION_NOT_SUPPORTED = LogMessageOffset::notice + 2001,
  N2002_NO_RESPONSE_FROM_LNCV_MODULE_X_WITH_ADDRESS_X = LogMessageOffset::notice + 2002,
  N2003_STOPPED_SENDING_FAST_CLOCK_SYNC = LogMessageOffset::notice + 2003,
  N2004_STARTING_PCAP_FILE_LOG_X = LogMessageOffset::notice + 2004,
  N2005_STARTING_PCAP_LOG_PIPE_X = LogMessageOffset::notice + 2005,
  N2006_LISTEN_ONLY_MODE_ACTIVATED = LogMessageOffset::notice + 2006,
  N2007_LISTEN_ONLY_MODE_DEACTIVATED = LogMessageOffset::notice + 2007,
  N3001_ASSIGNED_TRAIN_X_TO_BLOCK_X = LogMessageOffset::notice + 3001,
  N3002_REMOVED_TRAIN_X_FROM_BLOCK_X = LogMessageOffset::notice + 3002,
  N3003_TURNOUT_RESET_TO_RESERVED_POSITION = LogMessageOffset::notice + 3003,
  N3004_SIGNAL_RESET_TO_RESERVED_ASPECT = LogMessageOffset::notice + 3004,
  N9001_STARTING_SCRIPT = LogMessageOffset::notice + 9001,
  N9999_X = LogMessageOffset::notice + 9999,

  // Warning:
  W1001_DISCOVERY_DISABLED_ONLY_ALLOWED_ON_PORT_X = LogMessageOffset::warning + 1001,
  W1002_SETTING_X_DOESNT_EXIST = LogMessageOffset::warning + 1002,
  W1003_READING_WORLD_X_FAILED_LIBARCHIVE_ERROR_X_X = LogMessageOffset::warning + 1003,
  W2001_RECEIVED_MALFORMED_DATA_DROPPED_X_BYTES = LogMessageOffset::warning + 2001,
  W2002_COMMAND_STATION_DOESNT_SUPPORT_FUNCTIONS_ABOVE_FX = LogMessageOffset::warning + 2002,
  W2003_RECEIVED_MALFORMED_DATA_DROPPED_X_BYTES_X = LogMessageOffset::warning + 2003,
  W2004_INPUT_ADDRESS_X_IS_INVALID = LogMessageOffset::warning + 2004,
  W2005_OUTPUT_ADDRESS_X_IS_INVALID = LogMessageOffset::warning + 2005,
  W2006_COMMAND_STATION_DOES_NOT_SUPPORT_LOCO_SLOT_X = LogMessageOffset::warning + 2006,
  W2007_COMMAND_STATION_DOES_NOT_SUPPORT_THE_FAST_CLOCK_SLOT = LogMessageOffset::warning + 2007,
  W2018_TIMEOUT_NO_ECHO_WITHIN_X_MS = LogMessageOffset::warning + 2018,
  W2019_Z21_BROADCAST_FLAG_MISMATCH = LogMessageOffset::warning + 2019,
  W2020_DCCEXT_RCN213_IS_NOT_SUPPORTED = LogMessageOffset::warning + 2020,
  W3001_NX_BUTTON_CONNECTED_TO_TWO_BLOCKS = LogMessageOffset::warning + 3001,
  W3002_NX_BUTTON_NOT_CONNECTED_TO_ANY_BLOCK = LogMessageOffset::warning + 3002,
  W3003_LOCKED_TURNOUT_CHANGED = LogMessageOffset::warning + 3003,
  W3004_LOCKED_SIGNAL_CHANGED = LogMessageOffset::warning + 3004,
  W9001_EXECUTION_TOOK_X_US = LogMessageOffset::warning + 9001,
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
  E2012_FUNCTION_NUMBER_ALREADY_IN_USE = LogMessageOffset::error + 2012,
  E2013_SERIAL_PORT_SET_BAUDRATE_FAILED_X = LogMessageOffset::error + 2013,
  E2014_SERIAL_PORT_SET_DATA_BITS_FAILED_X = LogMessageOffset::error + 2014,
  E2015_SERIAL_PORT_SET_STOP_BITS_FAILED_X = LogMessageOffset::error + 2015,
  E2016_SERIAL_PORT_SET_PARITY_FAILED_X = LogMessageOffset::error + 2016,
  E2017_SERIAL_PORT_SET_FLOW_CONTROL_FAILED_X = LogMessageOffset::error + 2017,
  E2018_TIMEOUT_NO_ECHO_WITHIN_X_MS = LogMessageOffset::error + 2018,
  E2019_TIMEOUT_NO_RESPONSE_WITHIN_X_MS = LogMessageOffset::error + 2019,
  E2020_TOTAL_NUMBER_OF_MODULES_MAY_NOT_EXCEED_X  = LogMessageOffset::error + 2020,
  E2021_STARTING_PCAP_LOG_FAILED_X = LogMessageOffset::error + 2021,
  E2022_SOCKET_CREATE_FAILED_X = LogMessageOffset::error + 2022,
  E2023_SOCKET_IOCTL_FAILED_X = LogMessageOffset::error + 2023,
  E2024_UNKNOWN_LOCOMOTIVE_MFX_UID_X = LogMessageOffset::error + 2024,
  E3001_CANT_DELETE_RAIL_VEHICLE_WHEN_IN_ACTIVE_TRAIN = LogMessageOffset::error + 3001,
  E3002_CANT_DELETE_ACTIVE_TRAIN = LogMessageOffset::error + 3002,
  E3003_TRAIN_STOPPED_ON_TURNOUT_X_CHANGED = LogMessageOffset::error + 3003,
  E3004_TRAIN_STOPPED_ON_SIGNAL_X_CHANGED = LogMessageOffset::error + 3004,
  E3005_CANT_REMOVE_TRAIN_TRAIN_MUST_BE_STOPPED_FIRST = LogMessageOffset::error + 3005,
  E3006_CANT_REMOVE_TRAIN_TRAIN_CAN_ONLY_BE_REMOVED_FROM_HEAD_OR_TAIL_BLOCK = LogMessageOffset::error + 3006,
  E3007_WORLD_STOPPED_ON_TURNOUT_X_CHANGED = LogMessageOffset::error + 3007,
  E3008_WORLD_STOPPED_ON_SIGNAL_X_CHANGED = LogMessageOffset::error + 3008,
  E3009_WORLD_POWER_OFF_ON_TURNOUT_X_CHANGED = LogMessageOffset::error + 3009,
  E3010_WORLD_POWER_OFF_ON_SIGNAL_X_CHANGED = LogMessageOffset::error + 3010,
  E9001_X_DURING_EXECUTION_OF_X_EVENT_HANDLER = LogMessageOffset::error + 9001,
  E9999_X = LogMessageOffset::error + 9999,

  // Critical:
  C1001_LOADING_WORLD_FAILED_X = LogMessageOffset::critical + 1001,
  C1002_CREATING_CONNECTION_FAILED_X = LogMessageOffset::critical + 1002,
  C1003_CANT_WRITE_TO_SETTINGS_FILE_X = LogMessageOffset::critical + 1003,
  C1004_READING_WORLD_FAILED_X_X = LogMessageOffset::critical + 1004,
  C1005_SAVING_WORLD_FAILED_X = LogMessageOffset::critical + 1005,
  C1006_CREATING_WORLD_BACKUP_FAILED_X = LogMessageOffset::critical + 1006,
  C1007_CREATING_WORLD_BACKUP_DIRECTORY_FAILED_X = LogMessageOffset::critical + 1007,
  C1008_CREATING_BACKUP_DIRECTORY_FAILED_X = LogMessageOffset::critical + 1008,
  C1009_CREATING_SETTING_BACKUP_FAILED_X = LogMessageOffset::critical + 1009,
  C1010_EXPORTING_WORLD_FAILED_X = LogMessageOffset::critical + 1010,
  C1011_IMPORTING_WORLD_FAILED_X = LogMessageOffset::critical + 1011,
  C1012_UNKNOWN_CLASS_X_CANT_RECREATE_OBJECT_X = LogMessageOffset::critical + 1012,
  C1013_CANT_LOAD_WORLD_SAVED_WITH_NEWER_VERSION_REQUIRES_AT_LEAST_X = LogMessageOffset::critical + 1013,
  C1014_INVALID_COMMAND = LogMessageOffset::critical + 1014,
  C1015_UNKNOWN_OBJECT = LogMessageOffset::critical + 1015,
  C1016_UNKNOWN_PROPERTY = LogMessageOffset::critical + 1016,
  C1017_INVALID_INDICES = LogMessageOffset::critical + 1017,
  C1018_EXCEPTION_X = LogMessageOffset::critical + 1018,
  C1019_OBJECT_NOT_A_TABLE = LogMessageOffset::critical + 1019,
  C2001_ADDRESS_ALREADY_USED_AT_X = LogMessageOffset::critical + 2001,
  C2004_CANT_GET_FREE_SLOT = LogMessageOffset::critical + 2004,
  C2005_SOCKETCAN_IS_ONLY_AVAILABLE_ON_LINUX = LogMessageOffset::critical + 2005,
  C9999_X = LogMessageOffset::critical + 9999,

  // Fatal:
  F1001_OPENING_TCP_SOCKET_FAILED_X = LogMessageOffset::fatal + 1001,
  F1002_TCP_SOCKET_ADDRESS_REUSE_FAILED_X = LogMessageOffset::fatal + 1002,
  F1003_BINDING_TCP_SOCKET_FAILED_X = LogMessageOffset::fatal + 1003,
  F1004_TCP_SOCKET_LISTEN_FAILED_X = LogMessageOffset::fatal + 1004,
  F1005_OPENING_UDP_SOCKET_FAILED_X = LogMessageOffset::fatal + 1005,
  F1006_UDP_SOCKET_ADDRESS_REUSE_FAILED_X = LogMessageOffset::fatal + 1006,
  F1007_BINDING_UDP_SOCKET_FAILED_X = LogMessageOffset::fatal + 1007,
  F1008_EVENTLOOP_CRASHED_X = LogMessageOffset::fatal + 1008,
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

#ifdef QT_CORE_LIB
inline QString logMessageCode(LogMessage message)
{
  return QString(logMessageChar(message)).append(QString::number(logMessageNumber(message)).rightJustified(4, '0'));
}
#endif

#endif
