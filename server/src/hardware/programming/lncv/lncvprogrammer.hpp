/**
 * server/src/hardware/programming/lncv/lncvprogrammer.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROGRAMMING_LNCV_LNCVPROGRAMMER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROGRAMMING_LNCV_LNCVPROGRAMMER_HPP

#include "../../../core/object.hpp"
#include "../../../core/method.hpp"
#include "../../../core/event.hpp"

class LNCVProgrammingController;

/**
 * \brief LNCV programmer
 */
class LNCVProgrammer final : public Object
{
  CLASS_ID("lncv_programmer");

  private:
    LNCVProgrammingController& m_controller;

  public:
    Method<bool(uint16_t, uint16_t)> start;
    Method<bool(uint16_t)> read;
    Method<bool(uint16_t, uint16_t)> write;
    Method<bool()> stop;
    Event<bool, uint16_t, uint16_t> onReadResponse;

    LNCVProgrammer(LNCVProgrammingController& controller);
    ~LNCVProgrammer() final;

    std::string getObjectId() const final { assert(false); return {}; }

    void readResponse(bool success, uint16_t lncv, uint16_t value);
};

#endif
