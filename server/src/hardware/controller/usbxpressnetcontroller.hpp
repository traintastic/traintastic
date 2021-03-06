/**
 * server/src/hardware/controller/usbxpressnetcontroller.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_CONTROLLER_USBXPRESSNETCONTROLLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_CONTROLLER_USBXPRESSNETCONTROLLER_HPP

#include "controller.hpp"
#include <usbxpressnet.h>
#include "../../enum/usbxpressnetcontrollermode.hpp"

class USBXpressNetController : public Controller
{
  protected:
    usbxpressnet_handle m_handle;

    bool setActive(bool& value) final;

    void emergencyStopChanged(bool value) final;
    void trackPowerChanged(bool value) final;
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags, uint32_t) final;

  public:
    CLASS_ID("controller.usb_xpressnet_controller")
    CREATE(USBXpressNetController)

    Property<std::string> serial;
    Property<USBXpressNetControllerMode> mode;

    USBXpressNetController(const std::weak_ptr<World>& world, std::string_view _id);
    ~USBXpressNetController() final;
};

#endif
