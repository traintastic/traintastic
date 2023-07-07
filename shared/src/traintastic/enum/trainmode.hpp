/**
 * shared/src/traintastic/enum/trainmode.hpp
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_TRAINMODE_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_TRAINMODE_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"

enum class TrainMode : uint8_t
{
  ManualUnprotected = 1, //!< User or throttle controls the train, Traintastic doesn't monitor anything.
  ManualProtected = 2, //!< User or throttle controls the train, Traintastic monitors and interfears if required.
  Automatic = 3, //!< Traintastic controls the train.
};

TRAINTASTIC_ENUM(TrainMode, "train_mode", 3,
{
  {TrainMode::ManualUnprotected, "manual_unprotected"},
  {TrainMode::ManualProtected, "manual_protected"},
  {TrainMode::Automatic, "automatic"}
});

constexpr std::array<TrainMode, 3> trainModeValues
{
  TrainMode::ManualUnprotected,
  TrainMode::ManualProtected,
  TrainMode::Automatic,
};

#endif
