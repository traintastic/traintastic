/**
 * server/src/hardware/interface/hsi88.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#include "hsi88.hpp"
#include "../input/input.hpp"
#include "../input/list/inputlisttablemodel.hpp"
#include "../../core/attributes.hpp"
#include "../../core/eventloop.hpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/serialport.hpp"
#include "../../utils/setthreadname.hpp"
#include "../../utils/tohex.hpp"
#include "../../world/world.hpp"

constexpr auto inputListColumns = InputListColumn::Id | InputListColumn::Name | InputListColumn::Channel | InputListColumn::Address;

HSI88Interface::HSI88Interface(World& world, std::string_view _id)
  : Interface(world, _id)
  , InputController(static_cast<IdObject&>(*this))
  , m_ioContext{1}
  , m_serialPort{m_ioContext}
  , device{this, "device", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , modulesLeft{this, "modules_left", 2, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](uint8_t /*value*/)
      {
        updateModulesMax();
      }}
  , modulesMiddle{this, "modules_middle", 2, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](uint8_t /*value*/)
      {
        updateModulesMax();
      }}
  , modulesRight{this, "modules_right", 2, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](uint8_t /*value*/)
      {
        updateModulesMax();
      }}
  , debugLogRXTX{this, "debug_log_rx_tx", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](bool value)
      {
        m_debugLogRXTX = value;
      }}
{
  name = "HSI-88";

  const bool editable = contains(m_world.state, WorldState::Edit);

  Attributes::addEnabled(device, !online && editable);
  m_interfaceItems.insertBefore(device, notes);

  Attributes::addEnabled(modulesLeft, !online && editable);
  Attributes::addMinMax(modulesLeft, modulesMin, modulesMax);
  m_interfaceItems.insertBefore(modulesLeft, notes);

  Attributes::addEnabled(modulesMiddle, !online && editable);
  Attributes::addMinMax(modulesMiddle, modulesMin, modulesMax);
  m_interfaceItems.insertBefore(modulesMiddle, notes);

  Attributes::addEnabled(modulesRight, !online && editable);
  Attributes::addMinMax(modulesRight, modulesMin, modulesMax);
  m_interfaceItems.insertBefore(modulesRight, notes);

  m_interfaceItems.insertBefore(inputs, notes);

  Attributes::addDisplayName(debugLogRXTX, DisplayName::Hardware::debugLogRXTX);
  Attributes::addEnabled(debugLogRXTX, editable);
  m_interfaceItems.insertBefore(debugLogRXTX, notes);

  updateModulesMax();
}

std::pair<uint32_t, uint32_t> HSI88Interface::inputAddressMinMax(uint32_t channel) const
{
  (void)channel; // silence unused warning

  assert(
    channel == InputChannel::left ||
    channel == InputChannel::middle ||
    channel == InputChannel::right);

  return {inputAddressMin, inputAddressMax};
}

void HSI88Interface::inputSimulateChange(uint32_t channel, uint32_t address)
{
  //! \todo add simulation support
  (void)channel;
  (void)address;
}

void HSI88Interface::addToWorld()
{
  Interface::addToWorld();
  InputController::addToWorld(inputListColumns);
}

void HSI88Interface::destroying()
{
  InputController::destroying();
  Interface::destroying();
}

void HSI88Interface::loaded()
{
  Interface::loaded();

  if(modulesLeft > modulesMax)
    modulesLeft.setValueInternal(modulesMax);
  if(modulesMiddle > modulesMax)
    modulesMiddle.setValueInternal(modulesMax);
  if(modulesRight > modulesMax)
    modulesRight.setValueInternal(modulesMax);

  m_debugLogRXTX = debugLogRXTX;

  updateModulesMax();
}

void HSI88Interface::worldEvent(WorldState state, WorldEvent event)
{
  Interface::worldEvent(state, event);

  switch(event)
  {
    case WorldEvent::EditDisabled:
    case WorldEvent::EditEnabled:
    {
      const bool editable = contains(state, WorldState::Edit);
      Attributes::setEnabled({device, modulesLeft, modulesMiddle, modulesRight}, !online && editable);
      Attributes::setEnabled(debugLogRXTX, editable);
      break;
    }
    default:
      break;
  }
}

bool HSI88Interface::setOnline(bool& value, bool simulation)
{
  if(value)
  {
    if(modulesLeft + modulesMiddle + modulesRight > modulesTotal)
    {
      Log::log(*this, LogMessage::E2020_TOTAL_NUMBER_OF_MODULES_MAY_NOT_EXCEED_X, modulesTotal);
      return false;
    }

    m_simulation = simulation;

    if(simulation)
    {
      Log::log(*this, LogMessage::N2001_SIMULATION_NOT_SUPPORTED);
      return false;
    }

    {
      m_thread = std::thread(
        [this]()
        {
          setThreadName("hsi88");
          auto work = std::make_shared<boost::asio::io_context::work>(m_ioContext);
          m_ioContext.restart();
          m_ioContext.run();
        });

      m_ioContext.post(
        [this, dev=device.value(), ml=modulesLeft.value(), mm=modulesMiddle.value(), mr=modulesRight.value()]()
        {
          try
          {
            SerialPort::open(m_serialPort, dev, 9'600, 8, SerialParity::None, SerialStopBits::One, SerialFlowControl::Hardware);
          }
          catch(const LogMessageException& e)
          {
            Log::log(*this, e.message(), e.args());
            //! \todo interface status -> error
            return;
          }

          // reset buffers:
          m_readBufferOffset = 0;
          m_writeBuffer[0] = '\r';
          m_writeBufferOffset = 1;
          while(!m_sendQueue.empty())
            m_sendQueue.pop();
          m_waitingForReply = false;

          read();

          send(std::string{versionInquiry});
          send(std::string{terminalModeOff});
          const std::array<char, 5> registerModules = {'s', static_cast<char>(ml), static_cast<char>(mm), static_cast<char>(mr), '\r'};
          send({registerModules.data(), registerModules.size()});
        });
    }

    Attributes::setEnabled({device, modulesLeft, modulesMiddle, modulesRight}, false);
  }
  else
  {
    m_ioContext.stop();
    m_thread.join();
    m_serialPort.close();

    Attributes::setEnabled({device, modulesLeft, modulesMiddle, modulesRight}, contains(m_world.state, WorldState::Edit));
  }

  return true;
}

void HSI88Interface::read()
{
  assert(isHSI88Thread());

  m_serialPort.async_read_some(boost::asio::buffer(m_readBuffer.data() + m_readBufferOffset, m_readBuffer.size() - m_readBufferOffset),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        bytesTransferred += m_readBufferOffset;

        char* pos = m_readBuffer.data();
        char* end = pos + bytesTransferred;

        while(pos < end)
        {
          char* it = std::find(pos, end, '\r');
          if(it == end)
            break;

          if(it > pos)
            receive({pos, static_cast<size_t>(it - pos)});

          bytesTransferred -= it - pos + 1;
          pos = it + 1;
        }

        if(bytesTransferred != 0)
          std::memmove(m_readBuffer.data(), pos, bytesTransferred);
        m_readBufferOffset = bytesTransferred;

        read();
      }
      else if(ec != boost::asio::error::operation_aborted)
      {
        Log::log(*this, LogMessage::E2002_SERIAL_READ_FAILED_X, ec);
        //! \todo interface status -> error
      }
    });
}

void HSI88Interface::write()
{
  assert(isHSI88Thread());

  m_serialPort.async_write_some(boost::asio::buffer(m_writeBuffer.data(), m_writeBufferOffset),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        if(bytesTransferred < m_writeBufferOffset)
        {
          m_writeBufferOffset -= bytesTransferred;
          memmove(m_writeBuffer.data(), m_writeBuffer.data() + bytesTransferred, m_writeBufferOffset);
          write();
        }
        else
          m_writeBufferOffset = 0;
      }
      else if(ec != boost::asio::error::operation_aborted)
      {
        Log::log(*this, LogMessage::E2001_SERIAL_WRITE_FAILED_X, ec);
        //! \todo interface status -> error
      }
    });
}

void HSI88Interface::receive(std::string_view message)
{
  assert(isHSI88Thread());

  if(m_debugLogRXTX)
    Log::log(*this, LogMessage::D2002_RX_X, toHex(message));

  switch(message[0])
  {
    case 'i':
    {
      const uint8_t moduleCount = message[1];
      for(uint8_t i = 0; i < moduleCount; i++)
      {
        const uint8_t module = message[2 + 3 * i];
        const uint16_t bits = static_cast<uint16_t>(message[3 + 3 * i]) << 8 | message[4 + 3 * i];
        for(uint8_t j = 0; j < inputsPerModule; j++)
        {
          const TriState value = (bits & (static_cast<uint16_t>(1) << j)) ? TriState::True : TriState::False;
          const uint32_t index = (module - 1) * inputsPerModule + j;

          if(m_inputValues[index] != value)
          {
            m_inputValues[index] = value;

            EventLoop::call(
              [this, index, value]()
              {
                const auto moduleIndex = index / inputsPerModule;
                if(moduleIndex < modulesLeft.value())
                  updateInputValue(InputChannel::left, inputAddressMin + index, value);
                else if(moduleIndex < (modulesLeft.value() + modulesMiddle.value()))
                  updateInputValue(InputChannel::middle, inputAddressMin + index - modulesLeft.value() * inputsPerModule, value);
                else
                  updateInputValue(InputChannel::right, inputAddressMin + index - (modulesLeft.value() + modulesMiddle.value()) * inputsPerModule, value);
              });
          }
        }
      }
      break;
    }
    case 's':
      if(m_waitingForReply && !m_sendQueue.empty() && m_sendQueue.front()[0] == 's')
      {
        m_waitingForReply = false;
        m_sendQueue.pop();
        sendNext();
      }
      m_inputValues.resize(message[1] * inputsPerModule);
      std::fill(m_inputValues.begin(), m_inputValues.end(), TriState::Undefined);
      break;

    case 't':
      if(m_waitingForReply && !m_sendQueue.empty() && m_sendQueue.front()[0] == 't')
      {
        m_waitingForReply = false;
        m_sendQueue.pop();
        sendNext();
      }
      break;

    case 'v':
    case 'V':
      if(m_waitingForReply && !m_sendQueue.empty() && m_sendQueue.front() == versionInquiry)
      {
        m_waitingForReply = false;
        m_sendQueue.pop();
        sendNext();
      }
      Log::log(*this, LogMessage::I2004_HSI_88_X, message);
      break;
  }
}

void HSI88Interface::send(std::string message)
{
  assert(isHSI88Thread());

  m_sendQueue.emplace(std::move(message));

  if(!m_waitingForReply)
    sendNext();
}

void HSI88Interface::sendNext()
{
  assert(isHSI88Thread());

  if(m_sendQueue.empty())
    return;

  const auto& message = m_sendQueue.front();
  memcpy(m_writeBuffer.data() + m_writeBufferOffset, message.data(), message.size());
  m_writeBufferOffset += message.size();

  if(m_debugLogRXTX)
    Log::log(*this, LogMessage::D2001_TX_X, toHex(message));

  write();

  m_waitingForReply = true;
}

void HSI88Interface::updateModulesMax()
{
  const uint8_t sum = modulesLeft + modulesMiddle + modulesRight;
  const uint8_t unused = sum < modulesTotal ? modulesTotal - sum : 0;

  Attributes::setMax<uint8_t>(modulesLeft, modulesLeft + unused);
  Attributes::setMax<uint8_t>(modulesMiddle, modulesMiddle + unused);
  Attributes::setMax<uint8_t>(modulesRight, modulesRight + unused);
}
