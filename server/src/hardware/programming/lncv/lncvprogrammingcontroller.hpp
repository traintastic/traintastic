/**
 * server/src/hardware/programming/lncv/lncvprogrammingcontroller.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROGRAMMING_LNCV_LNCVPROGRAMMINGCONTROLLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROGRAMMING_LNCV_LNCVPROGRAMMINGCONTROLLER_HPP

#include <cstdint>

class LNCVProgrammer;

/**
 * \brief LNCV programming controller
 */
class LNCVProgrammingController
{
  private:
    LNCVProgrammer* m_programmer = nullptr;

  protected:
    void addToWorld();
    void destroying();

    LNCVProgrammer* lncvProgrammer() { return m_programmer; }

  public:
    static constexpr uint16_t moduleAddressAny = 65535;

    [[nodiscard]] bool attachLNCVProgrammer(LNCVProgrammer& programmer);
    void detachLNCVProgrammer(LNCVProgrammer& programmer);

    [[nodiscard]] virtual bool startLNCVProgramming(uint16_t moduleId, uint16_t moduleAddress) = 0;
    [[nodiscard]] virtual bool readLNCV(uint16_t lncv) = 0;
    [[nodiscard]] virtual bool writeLNCV(uint16_t lncv, uint16_t value) = 0;
    [[nodiscard]] virtual bool stopLNCVProgramming() = 0;
};

#endif
