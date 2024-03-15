/**
 * server/src/hardware/protocol/z21/kernel.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_KERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_KERNEL_HPP

#include "../kernelbase.hpp"

#include <array>

#include "config.hpp"
#include "iohandler/iohandler.hpp"

class Decoder;
enum class DecoderChangeFlags;
class DecoderController;

namespace Z21 {

struct Message;
enum HardwareType : uint32_t;

class Kernel : public ::KernelBase
{
  protected:
    std::unique_ptr<IOHandler> m_ioHandler;

    Kernel(std::string logId_);
    virtual ~Kernel() = default;

    void setIOHandler(std::unique_ptr<IOHandler> handler);

    virtual void onStart() {}
    virtual void onStop() {}

  public:
    Kernel(const Kernel&) = delete;
    Kernel& operator =(const Kernel&) = delete;

    /**
     * @brief Start the kernel and IO handler
     */
    virtual void start();

    /**
     * @brief Stop the kernel and IO handler
     */
    virtual void stop();
};

}

#endif
