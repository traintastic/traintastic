#include "Marklin6050Interface.hpp"

#include "../../core/attributes.hpp"
#include "../../utils/displayname.hpp"   
#include "../../world/world.hpp"
#include "../../core/serialdeviceproperty.hpp"
#include "../../hardware/protocol/Marklin6050Interface/serial_port_list.hpp"

CREATE_IMPL(Marklin6050Interface)

Marklin6050Interface::Marklin6050Interface(World& world, std::string_view objId)
    : Interface(world, objId),
      serialPort(this, "serialPort", "", PropertyFlags::ReadWrite | PropertyFlags::Store),
      baudrate(this, "baudrate", 2400, PropertyFlags::ReadWrite | PropertyFlags::Store), // default 2400
      centralUnitVersion(this, "centralUnitVersion", 0, PropertyFlags::ReadWrite | PropertyFlags::Store) 
{
    name = "Märklin 6050";

    Attributes::addDisplayName(serialPort, DisplayName::Serial::device);
    Attributes::addEnabled(serialPort, !online);
    Attributes::addVisible(serialPort, true);
    m_interfaceItems.insertBefore(serialPort, notes);

    Attributes::addDisplayName(baudrate, DisplayName::Serial::baudrate);
    Attributes::addEnabled(baudrate, !online);
    Attributes::addVisible(baudrate, true);
    m_interfaceItems.insertBefore(baudrate, notes);
    Attributes::addValues(baudrate, std::vector<unsigned int>{
    1200,
    2400,
    4800,
    9600,
    19200,
    38400,
    57600,
    115200
    });
    
    static const std::vector<unsigned int> options = {
    6020, 6021, 6022, 6023,
    6223,
    6027, 6029, 6030, 6032
};
static const std::vector<std::string_view> labels = {
    "6020", "6021", "6022", "6023",
    "6223",
    "6027", "6029", "6030", "6032"
};

Attributes::addCategory(centralUnitVersion, "Märklin 6050");
Attributes::addDisplayName(centralUnitVersion, "Central Unit Version");
Attributes::addHelp(centralUnitVersion, "CUversion");

Attributes::addEnabled(centralUnitVersion, true);
Attributes::addVisible(centralUnitVersion, true);
m_interfaceItems.insertBefore(centralUnitVersion, notes);

Attributes::addValues(centralUnitVersion, options);
Attributes::addAliases(centralUnitVersion, &options, &labels);


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
            m_kernel->sendByte(97); // 0x61
            break;

        case WorldEvent::Run:
            m_kernel->sendByte(96); // 0x60
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
        
        m_kernel = std::make_unique<Marklin6050::Kernel>(port, baudrate.value());
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

