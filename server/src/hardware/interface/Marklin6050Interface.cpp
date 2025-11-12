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

CREATE_IMPL(Marklin6050Interface)

Marklin6050Interface::Marklin6050Interface(World& world, std::string_view objId)
  : Interface(world, objId),
    serialPort(this, "serialPort", "", PropertyFlags{}) // default flags
{
    name = "Märklin 6050";

    // Connect change callback
    serialPort.changed().connect([this](const std::string& newPort){
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
    std::string port = serialPort.getValue(); // actual Property method

    if(value)
    {
        if(port.empty() || !Marklin6050::Serial::isValidPort(port))
        {
            value = false;
            return false;
        }

        if(!Marklin6050::Serial::testOpen(port))
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
    // Enable/disable serialPort based on online status
    serialPort.setEnabled(getState() != InterfaceState::Online);
}

void Marklin6050Interface::serialPortChanged(const std::string& newPort)
{
    if(getState() == InterfaceState::Online)
    {
        // If new port is invalid, go offline
        if(!Marklin6050::Serial::isValidPort(newPort) || !Marklin6050::Serial::testOpen(newPort))
        {
            bool val = false;
            setOnline(val, false);
        }
    }
}
