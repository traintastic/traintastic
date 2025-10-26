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

#include <boost/signals2/connection.hpp>
#include "../../core/property.hpp"
#include "../../core/objectproperty.hpp"

enum class InputChannel : uint16_t;
enum class WorldState : uint32_t;
enum class WorldEvent : uint64_t;

class World;
class Object;
class Input;
class InputController;

class InputConsumer
{
private:
  Object& m_object;
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

  const std::shared_ptr<Input>& input()
  {
    return m_input;
  }

  virtual void inputValueChanged(bool value, const std::shared_ptr<Input>& input) = 0;

public:
  ObjectProperty<InputController> interface;
  Property<InputChannel> channel;
  Property<uint32_t> address;

  InputConsumer(Object& object, const World& world);
  virtual ~InputConsumer();
};

#endif
