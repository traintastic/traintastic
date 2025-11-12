#include "Marklin6050Interface.hpp"
#include "../../core/attributes.hpp"
#include "../../utils/displayname.hpp"
#include "../../world/world.hpp"
#include "../../utils/makearray.hpp"
#include "../../core/objectproperty.tpp"
#include "../../core/serialdeviceproperty.hpp"
#include "../../hardware/protocol/Marklin6050Interface/serial_port_list.hpp"

CREATE_IMPL(Marklin6050Interface)

Marklin6050Interface::Marklin6050Interface(World& world, std::string_view objId)
  : Interface(world, objId),
    serialPort(this, "serialPort", "", PropertyFlags{}) // just create property
{
    name = "Märklin 6050";

    // Set display name and initial enabled state
    Attributes::addDisplayName(serialPort, DisplayName::Serial::device);
    Attributes::addEnabled(serialPort, !online);

    // Do NOT call listSerialPorts() here; will populate later in loaded()
}

void Marklin6050Interface::addToWorld()
{
    Interface::addToWorld();
}

void Marklin6050Interface::loaded()
{
    Interface::loaded();

    // Lazy population of serial ports (does not block constructor)
    auto ports = Marklin6050::Serial::listAvailablePorts();
    if (ports.empty())
        ports.push_back("No ports detected");

    Attributes::addValues(serialPort, ports);

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
    // Enable or disable the serial port property depending on online state
    Attributes::setEnabled(serialPort, !online);
}

void Marklin6050Interface::serialPortChanged(const std::string& newPort)
{
    if (status && status->state == InterfaceState::Online)
    {
        if (!Marklin6050::Serial::isValidPort(newPort) || !Marklin6050::Serial::testOpen(newPort))
        {
            bool val = false;
            setOnline(val, false);
        }
    }
}
