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

Marklin6050Interface::Marklin6050Interface(World& world, std::string_view objectId)
  : Interface(world, objectId),
    serialPort(this, "serialPort", "", PropertyFlags::None)
{
    name = "Märklin 6050";

    // Populate serial port choices manually (platform-independent)
    std::vector<std::string> ports = Marklin6050::Serial::getPortList();
    // Could later create a UI helper for choices
    serialPortChanged(serialPort.getValue()); // initial check

    // Connect property change
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
    if(value)
    {
        std::string port = serialPort.getValue();
        if(!Marklin6050::Serial::isValidPort(port) || !Marklin6050::Serial::testOpen(port))
        {
            value = false;
        }
    }

    if(value)
    {
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
    // Disable serial port selection when online
    serialPort.setEnabled(status != InterfaceState::Online);
}

void Marklin6050Interface::serialPortChanged(const std::string& newPort)
{
    if(status == InterfaceState::Online)
    {
        if(!Marklin6050::Serial::isValidPort(newPort) || !Marklin6050::Serial::testOpen(newPort))
        {
            bool val = false;
            setOnline(val, false);
        }
    }
}
