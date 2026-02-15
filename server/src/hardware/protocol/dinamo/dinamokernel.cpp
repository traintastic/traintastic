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

#include "dinamokernel.hpp"
#include "dinamomessages.hpp"
#include "simulator/dinamosimulator.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"
#include "../../../log/logmessageexception.hpp"
#include "../../../utils/setthreadname.hpp"

namespace Dinamo {

namespace
{
  constexpr Version protocolVersionMin{3, 2, 0, 0};
  constexpr Version protocolVersionMax{4, 0, 0, 0};

  constexpr bool isProtocolVersionSupported(const Version& version)
  {
    return version >= protocolVersionMin && version < protocolVersionMax;
  }

  constexpr bool isSystemSupported(const System& system)
  {
    return
      (system.type == SystemType::RM_C);
  }
}

Kernel::Kernel(std::string logId_, const Config& config, bool simulation)
  : KernelBase(std::move(logId_))
  , m_simulation{simulation}
  , m_config{config}
{
  assert(isEventLoopThread());
}

void Kernel::setConfig(const Config& config)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, newConfig=config]()
    {
      if((!m_config.setHFILevel && newConfig.setHFILevel) ||
          (newConfig.setHFILevel && m_config.hfiLevel != newConfig.hfiLevel))
      {
        send(SetHFILevel(newConfig.hfiLevel));
      }

      m_config = newConfig;
    });
}

void Kernel::start()
{
  assert(isEventLoopThread());
  assert(m_ioHandler);

  m_thread = std::thread(
    [this]()
    {
      setThreadName("dinamo");
      auto work = std::make_shared<boost::asio::io_context::work>(m_ioContext);
      m_ioContext.run();
    });

  m_ioContext.post(
    [this]()
    {
      try
      {
        m_ioHandler->start();
      }
      catch(const LogMessageException& e)
      {
        EventLoop::call(
          [this, e]()
          {
            Log::log(logId, e.message(), e.args());
            error();
          });
        return;
      }
    });
}

void Kernel::stop()
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this]()
    {
      m_ioHandler->stop();
    });

  m_ioContext.stop();

  m_thread.join();
}

void Kernel::started()
{
  assert(isKernelThread());

  send(ProtocolVersionRequest());

  send(SystemVersionRequest());

  if(m_config.setHFILevel)
  {
    send(SetHFILevel(m_config.hfiLevel));
  }

  ::KernelBase::started();
}

void Kernel::receive(std::span<const uint8_t> message, bool hold, bool fault)
{
  assert(isKernelThread());

  if(m_config.debugLogRXTX)
  {
    EventLoop::call(
      [this, msg=toString(message, hold, fault)]()
      {
        Log::log(logId, LogMessage::D2002_RX_X, msg);
      });
  }

  if(m_rxFault != fault)
  {
    m_rxFault = fault;
    if(m_rxFault && !m_txFault && onFault)
    {
      EventLoop::call(onFault);
    }
  }

  if(SystemCommand::check(message))
  {
    handleSystemCommand(message);
  }
  else if(InputEvent::check(message) || InputRequestOrResponse::check(message))
  {
    handleInput(message);
  }
}

void Kernel::setFault()
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this]()
    {
      if(!m_txFault)
      {
        m_txFault = true;
        send({}); // send null to make sure fault is sent asap
      }
    });
}

void Kernel::resetFault()
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this]()
    {
      if(m_txFault)
      {
        m_txFault = false;
        send(ResetFault());
      }
    });
}

void Kernel::requestInputState(std::vector<uint16_t> inputAddresses)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, addresses=std::move(inputAddresses)]()
    {
      for(auto address : addresses)
      {
        send(InputRequestOrResponse(address, false));
      }
    });
}

void Kernel::setOC32Aspect(uint16_t address, uint8_t aspect)
{
  assert(isEventLoopThread());

  if(aspect <= 0x7F) [[likely]]
  {
    m_ioContext.post(
      [this, address, aspect]()
      {
        send(Ox32((address >> 5) & 0x1F, address & 0x1F, Ox32::Command::SetAspect, aspect));
      });
  }
}

void Kernel::setBlockAnalog(uint8_t block, bool light, std::optional<Polarity> polarity)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, block, light, polarity]()
    {
      using enum BlockControl::Action;
      send(BlockControl(block, light ? SetAnalogLightOn : SetAnalogLightOff, polarity, true));
    });
}

void Kernel::setBlockAnalogSpeed(uint8_t block, uint8_t speed, std::optional<Dinamo::Polarity> polarity)
{
  m_ioContext.post(
    [this, block, speed, polarity]()
    {
      if(polarity)
      {
        send(BlockAnalogSetSpeedPolarity(block, speed, *polarity));
      }
      else
      {
        send(BlockAnalogSetSpeed(block, speed));
      }
    });
}

void Kernel::setBlockAnalogLight(uint8_t block, bool on)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, block, on]()
    {
      send(BlockAnalogSetLight(block, on));
    });
}

void Kernel::setBlockDCC(uint8_t block, std::optional<Polarity> polarity)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, block, polarity]()
    {
      using enum BlockControl::Action;
      send(BlockControl(block, SetDCCAndClear, polarity, true));
    });
}

void Kernel::setBlockDCCSpeedAndDirection(uint8_t block, uint16_t address, bool longAddress, bool emergencyStop, uint8_t speedStep, Direction direction)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, block, address, longAddress, emergencyStop, speedStep, direction]()
    {
      if(longAddress)
      {
        send(BlockDCCLongSpeedDirection(block, address, emergencyStop, speedStep, direction));
      }
      else
      {
        send(BlockDCCShortSpeedDirection(block, static_cast<uint8_t>(address), emergencyStop, speedStep, direction));
      }
    });
}

void Kernel::setBlockDCCFunctionF0F4(uint8_t block, uint16_t address, bool longAddress, bool f0, bool f1, bool f2, bool f3, bool f4)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, block, address, longAddress, f0, f1, f2, f3, f4]()
    {
      if(longAddress)
      {
        send(BlockDCCLongFunctionF0F4(block, address, f0, f1, f2, f3, f4));
      }
      else
      {
        send(BlockDCCShortFunctionF0F4(block, static_cast<uint8_t>(address), f0, f1, f2, f3, f4));
      }
    });
}

void Kernel::setBlockDCCFunctionF5F8(uint8_t block, uint16_t address, bool longAddress, bool f5, bool f6, bool f7, bool f8)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, block, address, longAddress, f5, f6, f7, f8]()
    {
      if(longAddress)
      {
        send(BlockDCCLongFunctionF5F8(block, address, f5, f6, f7, f8));
      }
      else
      {
        send(BlockDCCShortFunctionF5F8(block, static_cast<uint8_t>(address), f5, f6, f7, f8));
      }
    });
}

void Kernel::setBlockDCCFunctionF9F12(uint8_t block, uint16_t address, bool longAddress, bool f9, bool f10, bool f11, bool f12)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, block, address, longAddress, f9, f10, f11, f12]()
    {
      if(longAddress)
      {
        send(BlockDCCLongFunctionF9F12(block, address, f9, f10, f11, f12));
      }
      else
      {
        send(BlockDCCShortFunctionF9F12(block, static_cast<uint8_t>(address), f9, f10, f11, f12));
      }
    });
}

void Kernel::setBlockDCCFunctions(uint8_t block, uint16_t address, bool longAddress, uint32_t functions)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, block, address, longAddress, functions]()
    {
      if((functions & 0x1F) != 0)
      {
        const bool f0 = getBit<0>(functions);
        const bool f1 = getBit<1>(functions);
        const bool f2 = getBit<2>(functions);
        const bool f3 = getBit<3>(functions);
        const bool f4 = getBit<4>(functions);
        if(longAddress)
        {
          send(BlockDCCLongFunctionF0F4(block, address, f0, f1, f2, f3, f4));
        }
        else
        {
          send(BlockDCCShortFunctionF0F4(block, static_cast<uint8_t>(address), f0, f1, f2, f3, f4));
        }
      }
      if((functions & (0xF << 5)) != 0)
      {
        const bool f5 = getBit<5>(functions);
        const bool f6 = getBit<6>(functions);
        const bool f7 = getBit<7>(functions);
        const bool f8 = getBit<8>(functions);
        if(longAddress)
        {
          send(BlockDCCLongFunctionF5F8(block, address, f5, f6, f7, f8));
        }
        else
        {
          send(BlockDCCShortFunctionF5F8(block, static_cast<uint8_t>(address), f5, f6, f7, f8));
        }
      }
      if((functions & (0xF << 9)) != 0)
      {
        const bool f9 = getBit<9>(functions);
        const bool f10 = getBit<10>(functions);
        const bool f11 = getBit<11>(functions);
        const bool f12 = getBit<12>(functions);
        if(longAddress)
        {
          send(BlockDCCLongFunctionF9F12(block, address, f9, f10, f11, f12));
        }
        else
        {
          send(BlockDCCShortFunctionF9F12(block, static_cast<uint8_t>(address), f9, f10, f11, f12));
        }
      }
    });
}

void Kernel::linkBlock(uint8_t destinationBlock, uint8_t sourceBlock, bool invertPolarity)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, destinationBlock, sourceBlock, invertPolarity]()
    {
      using enum BlockControl::Action;
      send(BlockLink(destinationBlock, sourceBlock, invertPolarity));
    });
}

void Kernel::unlinkBlockUp(uint8_t block)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, block]()
    {
      send(BlockUnlink(block, true));
    });
}

void Kernel::unlinkBlockDown(uint8_t block)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, block]()
    {
      send(BlockUnlink(block, false));
    });
}

void Kernel::resetBlock(uint8_t block)
{
  assert(isEventLoopThread());

  m_ioContext.post(
    [this, block]()
    {
      using enum BlockControl::Action;
      send(BlockControl(block, Clear, std::nullopt, false));
    });
}

void Kernel::setIOHandler(std::unique_ptr<IOHandler> handler)
{
  assert(isEventLoopThread());
  assert(handler);
  assert(!m_ioHandler);
  m_ioHandler = std::move(handler);
}

template<typename T>
void Kernel::send(const T& message)
{
  static_assert(std::is_base_of_v<Message, T>);
  assert(isKernelThread());
  const auto* p = reinterpret_cast<const uint8_t*>(&message);
  send({p, p + sizeof(message)});
}

void Kernel::send(std::span<const uint8_t> message)
{
  assert(isKernelThread());

  const bool hold = false;

  if(m_config.debugLogRXTX)
  {
    EventLoop::call(
      [this, msg=toString(message, hold, m_txFault)]()
      {
        Log::log(logId, LogMessage::D2001_TX_X, msg);
      });
  }

  if(auto ec = m_ioHandler->send(message, hold, m_txFault); ec)
  {
    (void)ec; // FIXME: handle error
  }
}

void Kernel::handleSystemCommand(std::span<const uint8_t> message)
{
  assert(isKernelThread());

  if(ProtocolVersionResponse::check(message))
  {
    const auto& protocolVersionResponse = *reinterpret_cast<const ProtocolVersionResponse*>(message.data());

    m_protocolVersion.major = protocolVersionResponse.major();
    m_protocolVersion.minor = protocolVersionResponse.minor();
    m_protocolVersion.subRelease = protocolVersionResponse.subRelease();
    m_protocolVersion.bugFix = protocolVersionResponse.bugFix();

    if(isProtocolVersionSupported(m_protocolVersion))
    {
      EventLoop::call(
        [this, version=m_protocolVersion]()
        {
          Log::log(logId, LogMessage::I2007_DINAMO_PROTOCOL_VERSION_X, version.toString());
        });
    }
    else
    {
      EventLoop::call(
        [this, version=m_protocolVersion]()
        {
          Log::log(logId, LogMessage::E2028_DINAMO_PROTOCOL_VERSION_X_NOT_SUPPORTED, version.toString());
          error();
        });
    }
  }
  else if(SystemVersionResponse::check(message))
  {
    const auto& systemVersionResponse = *reinterpret_cast<const SystemVersionResponse*>(message.data());

    m_system.type = systemVersionResponse.type;
    m_system.version.major = systemVersionResponse.major();
    m_system.version.minor = systemVersionResponse.minor();
    m_system.version.subRelease = systemVersionResponse.subRelease();
    m_system.version.bugFix = systemVersionResponse.bugFix();

    if(isSystemSupported(m_system))
    {
      EventLoop::call(
        [this, type=m_system.type, version=m_system.version]()
        {
          Log::log(logId, LogMessage::I2008_DINAMO_SYSTEM_X_VX, toString(type), version.toString());
        });
    }
    else
    {
      EventLoop::call(
        [this, type=m_system.type, version=m_system.version]()
        {
          Log::log(logId, LogMessage::E2029_DINAMO_PROTOCOL_X_VX_NOT_SUPPORTED, toString(type), version.toString());
          error();
        });
    }
  }
}

void Kernel::handleInput(std::span<const uint8_t> message)
{
  assert(isKernelThread());

  const auto& input = *reinterpret_cast<const InputMessage*>(message.data());
  EventLoop::call(
    [this, address=input.address(), value=input.value()]()
    {
      if(onInputChanged) [[likely]]
      {
        onInputChanged(address, value);
      }
    });
}

}
