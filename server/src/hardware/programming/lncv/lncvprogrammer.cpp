/**
 * server/src/hardware/programming/lncv/lncvprogrammer.cpp
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

#include "lncvprogrammer.hpp"
#include "lncvprogrammingcontroller.hpp"
#include "../../../core/method.tpp"

LNCVProgrammer::LNCVProgrammer(LNCVProgrammingController& controller)
  : m_controller{controller}
  , start{*this, "start", MethodFlags::NoScript,
      [this](uint16_t moduleId, uint16_t moduleAddress)
      {
        return m_controller.startLNCVProgramming(moduleId, moduleAddress);
      }}
  , read{*this, "read",
      [this](uint16_t lncv)
      {
        return m_controller.readLNCV(lncv);
      }}
  , write{*this, "write",
      [this](uint16_t lncv, uint16_t value)
      {
        return m_controller.writeLNCV(lncv, value);
      }}
  , stop{*this, "stop",
      [this]()
      {
        return m_controller.stopLNCVProgramming();
      }}
  , onReadResponse{*this, "on_read_response", EventFlags::Public}
{
  if(!m_controller.attachLNCVProgrammer(*this))
    throw std::runtime_error("lncv_programmer:programmer_not_available");

  m_interfaceItems.add(start);
  m_interfaceItems.add(read);
  m_interfaceItems.add(write);
  m_interfaceItems.add(stop);
  m_interfaceItems.add(onReadResponse);
}

LNCVProgrammer::~LNCVProgrammer()
{
  m_controller.detachLNCVProgrammer(*this);
}

void LNCVProgrammer::readResponse(bool success, uint16_t lncv, uint16_t value)
{
  fireEvent(onReadResponse, success, lncv, value);
}
