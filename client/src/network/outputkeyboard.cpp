/**
 * client/src/network/outputkeyboard.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2024 Reinder Feenstra
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

#include "outputkeyboard.hpp"
#include "connection.hpp"
#include "event.hpp"
#include "abstractproperty.hpp"

OutputKeyboard::OutputKeyboard(std::shared_ptr<Connection> connection, Handle handle, const QString& classId_)
  : Object(std::move(connection), handle, classId_)
  , m_requestId{Connection::invalidRequestId}
{
  {
    auto request = Message::newRequest(Message::Command::OutputKeyboardGetOutputInfo);
    request->write(m_handle);
    m_connection->send(request,
      [this](const std::shared_ptr<Message> message)
      {
        m_requestId = Connection::invalidRequestId;
        assert(message);
        uint32_t count = message->read<uint32_t>();
        while(count > 0)
        {
          const uint32_t address = message->read<uint32_t>();
          auto& outputState = m_outputStates[address];
          outputState.used = message->read<bool>();
          switch(m_outputType)
          {
            case OutputType::Single:
              outputState.value = message->read<TriState>();
              break;

            case OutputType::Pair:
              outputState.value = message->read<OutputPairValue>();
              break;

            case OutputType::Aspect: /*[[unlikely]]*/
            case OutputType::ECoSState: /*[[unlikely]]*/
              assert(false);
              break;
          }
          emit outputStateChanged(address);
          count--;
        }
      });
    m_requestId = request->requestId();
  }


}

OutputKeyboard::~OutputKeyboard()
{
  if(m_requestId != Connection::invalidRequestId)
  {
    m_connection->cancelRequest(m_requestId);
  }
}

const OutputKeyboard::OutputState& OutputKeyboard::getOutputState(uint32_t address) const
{
  static constexpr OutputState invalid;

  if(auto it = m_outputStates.find(address); it != m_outputStates.end())
  {
    return it->second;
  }

  return invalid;
}

void OutputKeyboard::created()
{
  m_outputUsedChanged = getEvent("output_used_changed");
  m_outputValueChanged = getEvent("output_value_changed");
  m_outputType = getProperty("output_type")->toEnum<OutputType>();

  connect(m_outputUsedChanged, &Event::fired,
    [this](QVariantList arguments)
    {
      assert(arguments.size() == 2);
      const uint32_t address = arguments[0].toUInt();
      m_outputStates[address].used = arguments[1].toBool();
      emit outputStateChanged(address);
    });

  connect(m_outputValueChanged, &Event::fired,
    [this](QVariantList arguments)
    {
      assert(arguments.size() == 2);
      const uint32_t address = arguments[0].toUInt();
      switch(m_outputType)
      {
        case OutputType::Single:
          m_outputStates[address].value = static_cast<TriState>(arguments[1].value<std::underlying_type_t<TriState>>());
          break;

        case OutputType::Pair:
          m_outputStates[address].value = static_cast<OutputPairValue>(arguments[1].value<std::underlying_type_t<OutputPairValue>>());
          break;

        case OutputType::Aspect: /*[[unlikely]]*/
        case OutputType::ECoSState: /*[[unlikely]]*/
          assert(false);
          break;
      }
      emit outputStateChanged(address);
    });
}
