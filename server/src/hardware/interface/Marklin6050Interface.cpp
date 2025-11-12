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
#include "../../hardware/protocol/Marklin6050Interface/serial.hpp"

CREATE_IMPL(Marklin6050Interface)

Marklin6050Interface::Marklin6050Interface(World& world, std::string_view objId)
  : Interface(world, objId),
    serialPort(this, "serialPort", "", PropertyFlags{}) // default flags
{
    name = "Märklin 6050";

    // Populate serial port choices
    auto ports = Marklin6050::Serial::getPortList();
    Attributes::addChoices(serialPort, [ports]() { return ports; });

    // Connect change callback
    serialPort.afterChange().connect([this](const std::string& newPort){
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

    switch(event)
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
    if(value)
    {
        std::string port = serialPort.get();

        if(port.empty() || !Marklin6050::Serial::isValidPort(port))
        {
            value = false; // cannot go online without a valid port
            return false;
        }

        if(!Marklin6050::Serial::testOpen(port))
        {
            value = false; // cannot open port
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
    // Enable/disable the serialPort property depending on online state
    serialPort.setEnabled(state != InterfaceState::Online);
}

void Marklin6050Interface::serialPortChanged(const std::string& newPort)
{
    // Called when the serial port property is changed
    if(state == InterfaceState::Online)
    {
        // Re-test the port
        if(!Marklin6050::Serial::isValidPort(newPort) || !Marklin6050::Serial::testOpen(newPort))
        {
            setOnline(false, false);
        }
    }
}
