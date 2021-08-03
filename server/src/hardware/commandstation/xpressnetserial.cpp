/**
 * hardware/commandstation/xpressnetserial.cpp
 *
 * This file is part of the traintastic source code
 *
 * Copyright (C) 2019-2021 Reinder Feenstra <reinderfeenstra@gmail.com>
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

#include "xpressnetserial.hpp"
#include "../../world/world.hpp"
#include "../../core/traintastic.hpp"
#include "../../core/eventloop.hpp"
#include "../../core/attributes.hpp"
#include "../../log/log.hpp"

XpressNetSerial::XpressNetSerial(const std::weak_ptr<World>& world, std::string_view _id) :
  SerialCommandStation(world, _id),
  interface{this, "interface", XpressNetSerialInterface::LenzLI100, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](XpressNetSerialInterface value)
    {
      switch(value)
      {
        case XpressNetSerialInterface::Custom:
          break;

        case XpressNetSerialInterface::LenzLI100:
        case XpressNetSerialInterface::RoSoftS88XPressNetLI:
          baudrate.setValueInternal(9600);
          flowControl.setValueInternal(SerialFlowControl::Hardware);
          break;

        case XpressNetSerialInterface::LenzLI100F:
          baudrate.setValueInternal(19200);
          flowControl.setValueInternal(SerialFlowControl::Hardware);
          break;

        case XpressNetSerialInterface::LenzLI101F:
          baudrate.setValueInternal(19200);
          flowControl.setValueInternal(SerialFlowControl::Hardware);
          break;
      }
      updateEnabled();
      updateVisible();
    }},
  xpressnet{this, "xpressnet", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject},
  s88StartAddress{this, "s88_start_address", XpressNet::RoSoftS88XpressNetLI::S88StartAddress::startAddressDefault, PropertyFlags::ReadWrite | PropertyFlags::Store},
  s88ModuleCount{this, "s88_module_count", XpressNet::RoSoftS88XpressNetLI::S88ModuleCount::moduleCountDefault, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  name = "XpressNet (serial)";
  xpressnet.setValueInternal(std::make_shared<XpressNet::XpressNet>(*this, xpressnet.name(), std::bind(&XpressNetSerial::send, this, std::placeholders::_1)));

  Attributes::addValues(interface, XpressNetSerialInterfaceValues);
  Attributes::addEnabled(interface, !online);
  m_interfaceItems.insertBefore(interface, baudrate);
  m_interfaceItems.insertBefore(xpressnet, notes);
  Attributes::addMinMax(s88StartAddress, XpressNet::RoSoftS88XpressNetLI::S88StartAddress::startAddressMin, XpressNet::RoSoftS88XpressNetLI::S88StartAddress::startAddressMax);
  Attributes::addEnabled(s88StartAddress, !online);
  Attributes::addVisible(s88StartAddress, false);
  m_interfaceItems.insertBefore(s88StartAddress, notes);
  Attributes::addMinMax(s88ModuleCount, XpressNet::RoSoftS88XpressNetLI::S88ModuleCount::moduleCountMin, XpressNet::RoSoftS88XpressNetLI::S88ModuleCount::moduleCountMax);
  Attributes::addEnabled(s88ModuleCount, !online);
  Attributes::addVisible(s88ModuleCount, false);
  m_interfaceItems.insertBefore(s88ModuleCount, notes);

  updateEnabled();
  updateVisible();
}

void XpressNetSerial::loaded()
{
  SerialCommandStation::loaded();
  updateEnabled();
  updateVisible();
}

void XpressNetSerial::worldEvent(WorldState state, WorldEvent event)
{
  SerialCommandStation::worldEvent(state, event);
  if(event == WorldEvent::EditEnabled || event == WorldEvent::EditDisabled)
    updateEnabled();
}

void XpressNetSerial::emergencyStopChanged(bool value)
{
  CommandStation::emergencyStopChanged(value);

  if(online)
    xpressnet->emergencyStopChanged(value);
}

void XpressNetSerial::powerOnChanged(bool value)
{
  CommandStation::powerOnChanged(value);

  if(online)
    xpressnet->powerOnChanged(value);
}

void XpressNetSerial::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  CommandStation::decoderChanged(decoder, changes, functionNumber);

  if(online)
    xpressnet->decoderChanged(decoder, changes, functionNumber);
}

bool XpressNetSerial::send(const XpressNet::Message& msg)
{
  assert(XpressNet::isChecksumValid(msg));
  if(!m_serialPort.is_open())
    return false;
  boost::system::error_code ec;
  m_serialPort.write_some(boost::asio::buffer(static_cast<const void*>(&msg), msg.size()), ec); // TODO async
  if(ec)
  {
    Log::log(*this, LogMessage::E2001_SERIAL_WRITE_FAILED_X, ec);
    return false;
  }
  return true;
}

void XpressNetSerial::stop()
{
  SerialCommandStation::stop();
  updateEnabled();
}

void XpressNetSerial::started()
{
  SerialCommandStation::started();

  updateEnabled();

  switch(interface.value())
  {
    case XpressNetSerialInterface::RoSoftS88XPressNetLI:
    {
      send(XpressNet::RoSoftS88XpressNetLI::S88StartAddress(s88StartAddress));
      send(XpressNet::RoSoftS88XpressNetLI::S88ModuleCount(s88ModuleCount));
      break;
    }
    default:
      break;
  }
}

void XpressNetSerial::read()
{
  m_serialPort.async_read_some(boost::asio::buffer(m_readBuffer.data() + m_readBufferOffset, m_readBuffer.size() - m_readBufferOffset),
    [this](const boost::system::error_code& ec, std::size_t bytesTransferred)
    {
      if(!ec)
      {
        const uint8_t* pos = m_readBuffer.data();
        bytesTransferred += m_readBufferOffset;

        while(bytesTransferred > 1)
        {
          const XpressNet::Message* message = reinterpret_cast<const XpressNet::Message*>(pos);

          size_t drop = 0;
          while(message->size() <= bytesTransferred && !XpressNet::isChecksumValid(*message) && drop < bytesTransferred)
          {
            drop++;
            pos++;
            bytesTransferred--;
            message = reinterpret_cast<const XpressNet::Message*>(pos);
          }

          if(drop != 0)
          {
            EventLoop::call(
              [this, drop]()
              {
                Log::log(*this, LogMessage::W2001_RECEIVED_MALFORMED_DATA_DROPPED_X_BYTES, drop);
              });
          }

          if(message->size() <= bytesTransferred)
          {
            bool handled = false;

            if(message->header == 0xF2)
            {
              switch(*(pos + 1))
              {
                case 0xF1:
                {
                  using XpressNet::RoSoftS88XpressNetLI::S88StartAddress;
                  EventLoop::call(
                    [this, msg = *static_cast<const S88StartAddress*>(message)]()
                    {
                      assert(msg.startAddress >= S88StartAddress::startAddressMin && msg.startAddress <= S88StartAddress::startAddressMax);
                      s88StartAddress.setValueInternal(msg.startAddress);
                    });
                  handled = true;
                  break;
                }
                case 0xF2:
                {
                  using XpressNet::RoSoftS88XpressNetLI::S88ModuleCount;
                  EventLoop::call(
                    [this, msg = *static_cast<const S88ModuleCount*>(message)]()
                    {
                      assert(msg.moduleCount >= S88ModuleCount::moduleCountMin && msg.moduleCount <= S88ModuleCount::moduleCountMax);
                      s88ModuleCount.setValueInternal(msg.moduleCount);
                    });
                  handled = true;
                  break;
                }
              }
            }

            if(!handled)
              xpressnet->receive(*message);
            pos += message->size();
            bytesTransferred -= message->size();
          }
          else
            break;
        }

        if(bytesTransferred != 0)
          memmove(m_readBuffer.data(), pos, bytesTransferred);
        m_readBufferOffset = bytesTransferred;

        read();
      }
      else
        EventLoop::call(
          [this, ec]()
          {
            Log::log(*this, LogMessage::E2002_SERIAL_READ_FAILED_X, ec);
            online = false;
          });
    });
}

void XpressNetSerial::updateEnabled()
{
  auto w = m_world.lock();
  const bool enabled = w && contains(w->state.value(), WorldState::Edit) && !online;

  Attributes::setEnabled(interface, enabled);
  Attributes::setEnabled(s88StartAddress, enabled);
  Attributes::setEnabled(s88ModuleCount, enabled);
}

void XpressNetSerial::updateVisible()
{
  const bool visible = interface == XpressNetSerialInterface::RoSoftS88XPressNetLI;
  Attributes::setVisible(s88StartAddress, visible);
  Attributes::setVisible(s88ModuleCount, visible);
}
