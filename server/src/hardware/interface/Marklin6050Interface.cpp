#include "Marklin6050Interface.hpp"
#include "../../core/attributes.hpp"
#include "../../utils/displayname.hpp"
#include "../../world/world.hpp"
#include "../../utils/makearray.hpp"

CREATE_IMPL(Marklin6050Interface)

Marklin6050Interface::Marklin6050Interface(World& world, std::string_view objId)
  : Interface(world, objId),
    serialPort(this, "serialPort", "", PropertyFlags{},
               nullptr, 
               [this](std::string& newPort) { serialPortChanged(newPort); return true; })
{
    name = "Märklin 6050";
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
    // handle world events if needed
}

void Marklin6050Interface::onlineChanged(bool /*value*/)
{
    updateEnabled();
}

bool Marklin6050Interface::setOnline(bool& value, bool /*simulation*/)
{
    std::string port = serialPort; // read value via operator T()

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
    // Optionally update UI elements here
    // serialPort.setEnabled(...) if supported
}

void Marklin6050Interface::serialPortChanged(const std::string& newPort)
{
    if (status.valid()) // or status.get() != nullptr
    {
        InterfaceStatus* st = status.get(); // or whatever method returns raw pointer
        if (st->state == InterfaceState::Online)
        {
            if (!Marklin6050::Serial::isValidPort(newPort) || !Marklin6050::Serial::testOpen(newPort))
            {
                bool val = false;
                setOnline(val, false);
            }
        }
    }
}


