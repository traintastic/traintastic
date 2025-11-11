/**
 * server/src/hardware/protocol/Marklin6050Interface/Marklin6050Interface.cpp
 *
 * Dummy Märklin 6050 interface for Traintastic
 * Copyright (C) 2025
 */

#include "Marklin6050Interface.hpp"
#include "../../core/attributes.hpp"
#include "../../utils/displayname.hpp"
#include "../../world/world.hpp"
#include "../../utils/makearray.hpp"
#include <vector>
#include <string>

CREATE_IMPL(Marklin6050Interface)

Marklin6050Interface::Marklin6050Interface(World& world, std::string_view id)
    : Interface(world, id),
      serialPort(this, "serialPort", "", PropertyFlags{})  // default-constructed PropertyFlags
{
    name = "Märklin 6050";

    // Fill serial port choices dynamically
    auto availablePorts = Serial::getPortList();  // cross-platform serial port enumeration
    Attributes::addChoices(serialPort, availablePorts);

    serialPort.afterChange().connect([this](const std::string& value) {
        serialPortChanged(value);
    });
}

void Marklin6050Interface::addToWorld()
{
    Interface::addToWorld();
}

void Marklin6050Interface::loaded()
{
    Interface::loaded();
    updateEnabled();
}

void Marklin6050Interface::destroying()
{
    Interface::destroying();
}

void Marklin6050Interface::worldEvent(WorldState state, WorldEvent event)
{
    Interface::worldEvent(state, event);

    switch (event)
    {
        case WorldEvent::PowerOn:
            break;
        case WorldEvent::PowerOff:
            break;
        default:
            break;
    }
}

void Marklin6050Interface::onlineChanged(bool /*value*/)
{
    updateEnabled();
}

bool Marklin6050Interface::setOnline(bool& value, bool /*simulation*/)
{
    const std::string port = serialPort.get();

    if (value)
    {
        if (!port.empty() && Serial::isValidPort(port))
        {
            if (!Serial::testOpen(port))
            {
                value = false; // cannot open port
                setState(InterfaceState::Offline);
                return false;
            }
        }
        setState(InterfaceState::Online);
    }
    else
    {
        setState(InterfaceState::Offline);
    }

    updateEnabled();
    return true;
}

void Marklin6050Interface::updateEnabled()
{
    // Enable/disable the serial port property depending on online status
    serialPort.setEnabled(!isOnline());
}

void Marklin6050Interface::serialPortChanged(const std::string& /*newPort*/)
{
    // If interface is online, handle reconnect or reconfiguration here
    if (isOnline())
    {
        setOnline(false, false);
        setOnline(true, false);
    }
}
