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

#include "errorcode.hpp"

namespace {

struct ErrorCodeCategory : std::error_category
{
  const char* name() const noexcept final;
  std::string message(int ev) const final;
};

const char* ErrorCodeCategory::name() const noexcept
{
  return "traintastic";
}

std::string ErrorCodeCategory::message(int ev) const
{
  switch(static_cast<ErrorCode>(ev))
  {
    case ErrorCode::InvalidThrottle:
      return "invalid throttle";

    case ErrorCode::AlreadyAcquired:
      return "already acquired";

    case ErrorCode::CanNotActivateTrain:
      return "can't activate train";

    case ErrorCode::TrainMustBeStoppedToChangeDirection:
      return "train must be stopped to change direction";

    case ErrorCode::UnknownDecoderAddress:
      return "unknown decoder address";

    case ErrorCode::DecoderNotAssignedToAVehicle:
      return "decoder not assigned to a vehicle";

    case ErrorCode::VehicleNotAssignedToATrain:
      return "vehicle not assigned to a train";
  }
  return "(unrecognized error)";
}

const ErrorCodeCategory errorCodeCategory{};

}

std::error_code make_error_code(ErrorCode ec)
{
  return {static_cast<int>(ec), errorCodeCategory};
}
