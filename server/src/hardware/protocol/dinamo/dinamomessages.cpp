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

#include "dinamomessages.hpp"
#include <format>
#include "../../../utils/tohex.hpp"

namespace Dinamo {

std::string toString(std::span<const uint8_t> message, bool hold, bool fault)
{
  std::string s;

  if(message.empty())
  {
    s = "Null";
  }
  else if(SystemCommand::check(message))
  {
    if(ResetFault::check(message))
    {
      s = "ResetFault";
    }
    else if(SetHFILevel::check(message))
    {
      const auto& msg = *reinterpret_cast<const SetHFILevel*>(message.data());
      s = std::format("SetHFILevel level={}", msg.level);
    }
    else if(ProtocolVersionRequest::check(message))
    {
      s = "ProtocolVersionRequest";
    }
    else if(ProtocolVersionResponse::check(message))
    {
      const auto& msg = *reinterpret_cast<const ProtocolVersionResponse*>(message.data());
      s = std::format("ProtocolVersionResponse major={} minor={} subRelease={} bugFix={}", msg.major(), msg.minor(), msg.subRelease(), msg.bugFix());
    }
    else if(SystemVersionRequest::check(message))
    {
      s = "SystemVersionRequest";
    }
    else if(SystemVersionResponse::check(message))
    {
      const auto& msg = *reinterpret_cast<const SystemVersionResponse*>(message.data());
      s = std::format("SystemVersionResponse type={} major={} minor={} subRelease={} bugFix={}", static_cast<uint8_t>(msg.type), msg.major(), msg.minor(), msg.subRelease(), msg.bugFix());
    }
  }
  else if(SetOutput::check(message))
  {
    // FIXME: const auto& msg = *reinterpret_cast<const SetOutput*>(message.data());
    s = std::format("SetOutput");
  }
  else if(SetOutputWithPulseDuration::check(message))
  {
    // FIXME: const auto& msg = *reinterpret_cast<const SetOutputWithPulseDuration*>(message.data());
    s = std::format("SetOutputWithPulseDuration");
  }
  else if(Ox32::check(message))
  {
    const auto& msg = *reinterpret_cast<const Ox32*>(message.data());
    s = std::format("Ox32 module={} output={} command={} parameter={} ", msg.module(), msg.output(), toString(msg.command), msg.parameter);
  }
  else if(BlockAnalogSetSpeed::check(message))
  {
    const auto& msg = *reinterpret_cast<const BlockAnalogSetSpeed*>(message.data());
    s = std::format("BlockAnalogSetSpeed speed={}", msg.speed());
  }
  else if(BlockAnalogSetSpeedPolarity::check(message))
  {
    const auto& msg = *reinterpret_cast<const BlockAnalogSetSpeedPolarity*>(message.data());
    s = std::format("BlockAnalogSetSpeedPolarity speed={} polarity={}",
      msg.speed(),
      toString(msg.polarity()));
  }
  else if(BlockAnalogSetLight::check(message))
  {
    const auto& msg = *reinterpret_cast<const BlockAnalogSetLight*>(message.data());
    s = std::format("BlockAnalogSetLight light={}", msg.light());
  }
  else if(BlockDCCSpeedDirection::check(message))
  {
    const bool isShort = BlockDCCShortSpeedDirection::check(message);
    const bool isLong = BlockDCCLongSpeedDirection::check(message);
    if(isShort != isLong) [[likely]]
    {
      const auto& msg = *reinterpret_cast<const BlockDCCSpeedDirection*>(message.data());
      s = std::format("{} block={} address={} speed={} direction={}",
        isShort ? "BlockDCCShortSpeedDirection" : "BlockDCCLongSpeedDirection",
        msg.block(),
        isShort ? static_cast<const BlockDCCShortSpeedDirection&>(msg).address() : static_cast<const BlockDCCLongSpeedDirection&>(msg).address(),
        msg.emergencyStop() ? "estop" : std::to_string(msg.speed()),
        msg.direction() == Direction::Forward ? "forward" : "reverse");
    }
  }
  else if(BlockDCCSpeedDirectionPolarity::check(message))
  {
    const bool isShort = BlockDCCShortSpeedDirectionPolarity::check(message);
    const bool isLong = BlockDCCLongSpeedDirectionPolarity::check(message);
    if(isShort != isLong) [[likely]]
    {
      const auto& msg = *reinterpret_cast<const BlockDCCSpeedDirectionPolarity*>(message.data());
      s = std::format("{} block={} address={} speed={} direction={} polarity={}",
        isShort ? "BlockDCCShortSpeedDirection" : "BlockDCCLongSpeedDirection",
        msg.block(),
        isShort ? static_cast<const BlockDCCShortSpeedDirectionPolarity&>(msg).address() : static_cast<const BlockDCCLongSpeedDirectionPolarity&>(msg).address(),
        msg.emergencyStop() ? "estop" : std::to_string(msg.speed()),
        msg.direction() == Direction::Forward ? "forward" : "reverse",
        toString(msg.polarity()));
    }
  }
  else if(BlockDCCFunctionF0F4::check(message))
  {
    const bool isShort = BlockDCCShortFunctionF0F4::check(message);
    const bool isLong = BlockDCCLongFunctionF0F4::check(message);
    if(isShort != isLong) [[likely]]
    {
      const auto& msg = *reinterpret_cast<const BlockDCCFunctionF0F4*>(message.data());
      s = std::format("{} block={} address={} f0={} f1={} f2={} f3={} f4={}",
        isShort ? "BlockDCCShortFunctionF0F4" : "BlockDCCLongFunctionF0F4",
        msg.block(),
        isShort ? static_cast<const BlockDCCShortFunctionF0F4&>(msg).address() : static_cast<const BlockDCCLongFunctionF0F4&>(msg).address(),
        msg.f0(),
        msg.f1(),
        msg.f2(),
        msg.f3(),
        msg.f4());
    }
  }
  else if(BlockDCCFunctionF5F8::check(message))
  {
    const bool isShort = BlockDCCShortFunctionF5F8::check(message);
    const bool isLong = BlockDCCLongFunctionF5F8::check(message);
    if(isShort != isLong) [[likely]]
    {
      const auto& msg = *reinterpret_cast<const BlockDCCFunctionF5F8*>(message.data());
      s = std::format("{} block={} address={} f5={} f6={} f7={} f8={}",
        isShort ? "BlockDCCShortFunctionF5F8" : "BlockDCCLongFunctionF5F8",
        msg.block(),
        isShort ? static_cast<const BlockDCCShortFunctionF5F8&>(msg).address() : static_cast<const BlockDCCLongFunctionF5F8&>(msg).address(),
        msg.f5(),
        msg.f6(),
        msg.f7(),
        msg.f8());
    }
  }
  else if(BlockDCCFunctionF9F12::check(message))
  {
    const bool isShort = BlockDCCShortFunctionF9F12::check(message);
    const bool isLong = BlockDCCLongFunctionF9F12::check(message);
    if(isShort != isLong) [[likely]]
    {
      const auto& msg = *reinterpret_cast<const BlockDCCFunctionF9F12*>(message.data());
      s = std::format("{} block={} address={} f9={} f10={} f11={} f12={}",
        isShort ? "BlockDCCShortFunctionF9F12" : "BlockDCCLongFunctionF9F12",
        msg.block(),
        isShort ? static_cast<const BlockDCCShortFunctionF9F12&>(msg).address() : static_cast<const BlockDCCLongFunctionF9F12&>(msg).address(),
        msg.f9(),
        msg.f10(),
        msg.f11(),
        msg.f12());
    }
  }
  else if(BlockAlarm::check(message))
  {
    const auto& msg = *reinterpret_cast<const BlockAlarm*>(message.data());
    s = std::format("BlockAlarm block={} shortCircuit={}", msg.block(), msg.shortCircuit());
  }
  else if(BlockLink::check(message))
  {
    const auto& msg = *reinterpret_cast<const BlockLink*>(message.data());
    s = std::format("BlockLink block={} source={} polarity={} {}",
      msg.block(),
      msg.sourceBlock(),
      msg.invertPolarity() ? "invert" : "normal",
      msg.link() ? "link" : "copy");
  }
  else if(BlockUnlink::check(message))
  {
    const auto& msg = *reinterpret_cast<const BlockUnlink*>(message.data());
    s = std::format("BlockUnlink block={} {} zero={}",
      msg.block(),
      msg.unlinkUp() ? "up" : "down",
      msg.zero());
  }
  else if(BlockKickStart::check(message))
  {
    const auto& msg = *reinterpret_cast<const BlockKickStart*>(message.data());
    s = std::format("BlockKickStart block={} value={}", msg.block(), msg.value);
  }
  else if(BlockControl::check(message))
  {
    const auto& msg = *reinterpret_cast<const BlockControl*>(message.data());
    s = std::format("BlockControl block={}", msg.block());
    // FIXME
  }
  else if(InputEvent::check(message))
  {
    const auto& msg = *reinterpret_cast<const InputEvent*>(message.data());
    s = std::format("InputEvent address={} value={}", msg.address(), msg.value());
  }
  else if(InputRequestOrResponse::check(message))
  {
    const auto& msg = *reinterpret_cast<const InputRequestOrResponse*>(message.data());
    s = std::format("InputRequestOrResponse address={} value={}", msg.address(), msg.value());
  }

  // raw data:
  s.append(std::format(" [{}{}|{}]",
    hold ? "H" : "_",
    fault ? "F" : "_",
    toHex(message.data(), message.size(), true)));

  return s;
}

std::string toString(Ox32::Command cmd)
{
  switch(cmd)
  {
    using enum Ox32::Command;

    case SetAspect:
      return "SetAspect";

    default:
      return std::to_string(static_cast<uint8_t>(cmd));
  }
}

}