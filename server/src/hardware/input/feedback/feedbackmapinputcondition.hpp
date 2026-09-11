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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INPUT_FEEDBACK_FEEDBACKMAPINPUTCONDITION_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INPUT_FEEDBACK_FEEDBACKMAPINPUTCONDITION_HPP

#include "../../../core/object.hpp"
#include "../../../core/property.hpp"
#include <traintastic/enum/inputcondition.hpp>

class FeedbackMap;
class Input;
class World;

class FeedbackMapInputCondition : public Object
{
  friend class FeedbackMapItem;

  CLASS_ID("feedback_map_input_condition")

public:
  Property<InputCondition> condition;

  FeedbackMapInputCondition(FeedbackMap& feedbackMap, size_t inputIndex);

  std::string getObjectId() const final;

  bool matches() const;

protected:
  void worldEvent(WorldState state, WorldEvent event) override;

private:
  FeedbackMap& m_feedbackMap;
  const size_t m_inputIndex;
};

#endif
