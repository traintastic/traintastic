/**
 * server/src/hardware/protocol/Marklin6050Interface/settings6023.hpp
 *
 * Copyright (C) 2025
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6050_SETTINGS6023_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6050_SETTINGS6023_HPP

#include "../../../core/subobject.hpp"
#include "../../../core/property.hpp"
#include "config.hpp"

namespace Marklin6050 {

class Settings6023 final : public SubObject
{
    CLASS_ID("marklin6023_settings")

protected:
    void loaded() final;

public:
    Property<unsigned int> s88amount;
    Property<unsigned int> s88interval;
    Property<unsigned int> redundancy;

    Settings6023(Object& _parent, std::string_view parentPropertyName);

    Config config() const;
    void updateEnabled(bool online);
};

} // namespace Marklin6050

#endif
