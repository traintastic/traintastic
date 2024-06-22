#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_EXTERNALOUTPUTCHANGEACTION_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_EXTERNALOUTPUTCHANGEACTION_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"

enum class ExternalOutputChangeAction : uint8_t
{
  DoNothing = 0,
  EmergencyStopTrain = 1,
  EmergencyStopWorld = 2,
  PowerOffWorld = 3,
};

TRAINTASTIC_ENUM(ExternalOutputChangeAction, "ext_output_change_action", 4,
{
  {ExternalOutputChangeAction::DoNothing, "do_nothing"},
  {ExternalOutputChangeAction::EmergencyStopTrain, "estop_train"},
  {ExternalOutputChangeAction::EmergencyStopWorld, "estop_world"},
  {ExternalOutputChangeAction::PowerOffWorld, "poweroff_world"},
});

inline constexpr std::array<ExternalOutputChangeAction, 4> extOutputChangeActionValues{{
  ExternalOutputChangeAction::DoNothing,
  ExternalOutputChangeAction::EmergencyStopTrain,
  ExternalOutputChangeAction::EmergencyStopWorld,
  ExternalOutputChangeAction::PowerOffWorld,
}};

#endif // TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_EXTERNALOUTPUTCHANGEACTION_HPP
