/**
 * server/src/hardware/interface/Marklin6050Interface.cpp
 *
 * Dummy Märklin 6050 interface for Traintastic
 * © 2025
 */

#include "Marklin6050Interface.hpp"
#include "../../core/attributes.hpp"
#include "../../world/world.hpp"
#include "../../utils/displayname.hpp"

CREATE_IMPL(Marklin6050Interface)

Marklin6050Interface::Marklin6050Interface(World& world, std::string_view idValue)
    : Interface(world, idValue),
      serialPort(this, "serialPort", "", PropertyFlags{},
                 /* onChanged */ [this](const std::string& value) {
                     serialPortChanged(value);
                 })
{
    name = "Märklin 6050";

    // Populate serial port choices at construction
    auto availablePorts = Serial::getPortList();
    if (!availablePorts.empty())
        serialPort = availablePorts.front();
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
            // Automatically go online if desired
            break;

        case WorldEvent::PowerOff:
            break;

        default:
            break;
    }
}

bool Marklin6050Interface::setOnline(bool& value, bool /*simulation*/)
{
    if (value)
    {
        // Attempt to open selected serial port
        std::string port = serialPort.value();
        if (!Serial::isValidPort(port))
        {
            value = false;
            setState(InterfaceState::Offline);
            return false;
        }

        // Optionally test open
        if (!Serial::testOpen(port))
        {
            value = false;
            setState(InterfaceState::Offline);
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
    // Example: disable selection if online
    // serialPort.setEnabled(!isOnline());
}

void Marklin6050Interface::serialPortChanged(const std::string& newPort)
{
    // React to serial port changes, e.g., reconnect or log
    (void)newPort;
}

