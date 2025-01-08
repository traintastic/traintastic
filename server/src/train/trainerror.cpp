/**
 * server/src/train/trainerror.cpp
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

#include "trainerror.hpp"

namespace {

struct TrainErrorCategory : std::error_category
{
  const char* name() const noexcept final;
  std::string message(int ev) const final;
};

const char* TrainErrorCategory::name() const noexcept
{
  return "train";
}

std::string TrainErrorCategory::message(int ev) const
{
  switch(static_cast<TrainError>(ev))
  {
    case TrainError::InvalidThrottle:
      return "invalid throttle";

    case TrainError::AlreadyAcquired:
      return "already acquired";

    case TrainError::CanNotActivateTrain:
      return "can't activate train";

    case TrainError::TrainMustBeStoppedToChangeDirection:
      return "train must be stopped to change direction";
  }
  return "(unrecognized error)";
}

const TrainErrorCategory trainErrorCategory{};

}

std::error_code make_error_code(TrainError ec)
{
  return {static_cast<int>(ec), trainErrorCategory};
}
