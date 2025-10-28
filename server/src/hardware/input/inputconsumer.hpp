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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INPUT_INPUTCONSUMER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INPUT_INPUTCONSUMER_HPP

#include <boost/asio/steady_timer.hpp>
#include <boost/signals2/connection.hpp>
#include "../../core/property.hpp"
#include "../../core/objectproperty.hpp"

enum class InputChannel : uint16_t;
enum class WorldState : uint32_t;
enum class WorldEvent : uint64_t;

class World;
class Object;
class InterfaceItems;
class Input;
class InputController;

class InputConsumer
{
private:
  Object& m_object;
  boost::asio::steady_timer m_inputFilterTimer;
  std::shared_ptr<Input> m_input;
  boost::signals2::connection m_inputDestroying;
  boost::signals2::connection m_inputValueChanged;

  void setInput(std::shared_ptr<Input> value);
  void releaseInput();

  void interfaceChanged();
  void channelChanged();

protected:
  void loaded();
  void worldEvent(WorldState worldState, WorldEvent worldEvent);

  void addInterfaceItems(InterfaceItems& items);

  const std::shared_ptr<Input>& input()
  {
    return m_input;
  }

  virtual void inputValueChanged(bool value, const std::shared_ptr<Input>& input) = 0;

public:
  static constexpr uint16_t delayMin = 0;
  static constexpr uint16_t delayMax = 20'000;
  static constexpr uint16_t delayStep = 50;
  static constexpr std::string_view delayUnit = "ms";

  ObjectProperty<InputController> interface;
  Property<InputChannel> channel;
  Property<uint32_t> address;
  Property<uint16_t> onDelay;
  Property<uint16_t> offDelay;

  InputConsumer(Object& object, const World& world);
  virtual ~InputConsumer();
};

#endif
