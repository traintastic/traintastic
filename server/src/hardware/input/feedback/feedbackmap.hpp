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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INPUT_FEEDBACK_FEEDBACKMAP_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INPUT_FEEDBACK_FEEDBACKMAP_HPP

#include "../../../core/subobject.hpp"
#include "../../../core/method.hpp"
#include "../../../core/objectproperty.hpp"
#include "../../../core/objectvectorproperty.hpp"
#include "../../../core/property.hpp"
#include "../../../core/vectorproperty.hpp"
#include "../inputlocation.hpp"

class Input;
class InputController;
enum class InputChannel : uint16_t;
class FeedbackMapItem;

class FeedbackMap : public SubObject
{
public:
  using Items = std::vector<std::shared_ptr<FeedbackMapItem>>;
  using InputConnectionPair = std::pair<std::shared_ptr<Input>, boost::signals2::connection>;
  using Inputs = std::vector<InputConnectionPair>;

  ObjectProperty<Object> parentObject; // UI needs access to parent object
  ObjectProperty<InputController> interface;
  Property<InputChannel> channel;
  Property<uint32_t> node;
  VectorProperty<uint32_t> addresses;
  ObjectVectorProperty<FeedbackMapItem> items;
  Method<void()> addAddress;
  Method<void()> removeAddress;

  FeedbackMap(Object& _parent, std::string_view parentPropertyName);
  ~FeedbackMap() override;

  const std::shared_ptr<Input>& input(size_t index) const
  {
    assert(index < m_inputs.size());
    return m_inputs[index].first;
  }

protected:
  enum class MatchResult
  {
    Unknown,
    None,
    Match,
    Conflict,
  };

  static constexpr size_t invalidMatchIndex = std::numeric_limits<size_t>::max();

  Inputs m_inputs;

  void load(WorldLoader& loader, const nlohmann::json& data) override;
  void loaded() override;
  void worldEvent(WorldState state, WorldEvent event) override;

  void interfaceChanged();
  void channelChanged();
  void addressesSizeChanged();
  void updateInputConditions();
  void updateEnabled();

  uint32_t getUnusedAddress() const;

  virtual void matchResultChanged(MatchResult result, size_t index) = 0;

private:
  static constexpr size_t addressesSizeMin = 1;
  static constexpr size_t addressesSizeMax = 8;

  boost::signals2::scoped_connection m_interfaceDestroying;
  MatchResult m_lastMatchResult = MatchResult::Unknown;
  size_t m_lastMatchIndex = 0;

  void addInput(InputChannel ch, const InputLocation& location);
  void addInput(InputChannel ch, const InputLocation& location, InputController& inputController);
  InputConnectionPair getInput(InputChannel ch, const InputLocation& location, InputController& inputController);
  void releaseInput(InputConnectionPair& inputConnPair);
  void releaseInputs(Inputs& inputs);

  void inputValueChanged(bool value, const std::shared_ptr<Input>& input);

  void updateAddressDisplayName();
};

#endif
