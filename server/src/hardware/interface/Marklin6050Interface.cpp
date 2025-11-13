#include "Marklin6050Interface.hpp"

#include "../../core/attributes.hpp"
#include "../../utils/displayname.hpp"       // needed for DisplayName::Serial::device
#include "../../world/world.hpp"
#include "../../core/serialdeviceproperty.hpp"
#include "../../hardware/protocol/Marklin6050Interface/serial_port_list.hpp"

CREATE_IMPL(Marklin6050Interface)

Marklin6050Interface::Marklin6050Interface(World& world, std::string_view objId)
    : Interface(world, objId),
      serialPort(this, "serialPort", "", PropertyFlags::ReadWrite | PropertyFlags::Store)
{
    name = "Märklin 6050";

    // Show display name and enabled state (disabled while online)
    Attributes::addDisplayName(serialPort, DisplayName::Serial::device);
    Attributes::addEnabled(serialPort, !online);

    // Make property visible in the interface UI and insert it before the notes entry,
    // so it appears alongside other interface options (same pattern as LocoNet)
    Attributes::addVisible(serialPort, true);
    m_interfaceItems.insertBefore(serialPort, notes);

    // Do NOT enumerate ports here — SerialDeviceProperty already uses the global
    // SerialPortList and will populate/update values automatically.
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

    if (!m_kernel)
        return;

    switch (event)
    {
        case WorldEvent::Stop:
            m_kernel->sendByte(96); // 0x60
            break;

        case WorldEvent::Run:
            m_kernel->sendByte(97); // 0x61
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
    std::string port = serialPort;

    if (value)
    {
        if (port.empty() || !Marklin6050::Serial::isValidPort(port))
        {
            value = false;
            return false;
        }

        if (!Marklin6050::Serial::testOpen(port))
        {
            value = false;
            return false;
        }

        m_kernel = std::make_unique<Marklin6050::Kernel>(port);
        if (!m_kernel->start())
        {
            m_kernel.reset();
            value = false;
            return false;
        }

        setState(InterfaceState::Online);
    }
    else
    {
        if (m_kernel)
        {
            m_kernel->stop();
            m_kernel.reset();
        }
        setState(InterfaceState::Offline);
    }

    updateEnabled();
    return true;
}


void Marklin6050Interface::updateEnabled()
{
    // Disable serialPort while online (same UX as LocoNet)
    Attributes::setEnabled(serialPort, !online);
}

void Marklin6050Interface::serialPortChanged(const std::string& newPort)
{
    // Use the interface's 'online' flag instead of accessing ObjectProperty<InterfaceStatus>
    if (online)
    {
        if (!Marklin6050::Serial::isValidPort(newPort) || !Marklin6050::Serial::testOpen(newPort))
        {
            bool val = false;
            setOnline(val, false);
        }
    }
}

