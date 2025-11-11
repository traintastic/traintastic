/**
 * server/src/hardware/interface/Marklin6050Interface.cpp
 *
 * Dummy Märklin 6050 interface for Traintastic
 * Copyright (C) 2025
 */

#include "Marklin6050Interface.hpp"
#include "../../core/attributes.hpp"
#include "../../utils/displayname.hpp"
#include "../../world/world.hpp"
#include "../../utils/makearray.hpp"
#include "../protocol/Marklin6050Interface/serial.hpp" // Our cross-platform serial helper

CREATE_IMPL(Marklin6050Interface)

Marklin6050Interface::Marklin6050Interface(World& world, std::string_view id)
    : Interface(world, id),
      serialPort(this, "serialPort", "", PropertyFlags{}) // default flags
{
    name = "Märklin 6050";

    // Populate available serial ports dynamically
    auto availablePorts = SerialPort::getPortList();
    serialPort.setChoices(availablePorts);

    // Hook for changes
    serialPort.afterChange().connect([this](const std::string& newPort) {
        serialPortChanged(newPort);
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
    auto port = serialPort.getValue();

    if (value)
    {
        if (port.empty())
        {
            // Cannot go online without selecting a port
            value = false;
            return false;
        }

        if (!SerialPort::isValidPort(port))
        {
            value = false;
            return false;
        }

        if (!SerialPort::testOpen(port))
        {
            value = false;
            return false;
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
    // Enable/disable property based on online status
    serialPort.setEnabled(!isOnline());
}

void Marklin6050Interface::serialPortChanged(const std::string& /*newPort*/)
{
    // Re-open or test the port if the interface is online
    if (isOnline())
    {
        setOnline(true, false);
    }
}
