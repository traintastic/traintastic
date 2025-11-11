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
    serialPort(this, "serialPort", "", PropertyFlags::None)
{
  name = "Märklin 6050";

  // Optionally: populate default ports at load time
  auto availablePorts = Marklin6050::Serial::getPortList();
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
      // If desired, automatically go online here
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
    if (!Marklin6050::Serial::isValidPort(port))
    {
      value = false;
      setState(InterfaceState::Offline);
      return false;
    }

    // Optionally test open
    if (!Marklin6050::Serial::testOpen(port))
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
  // You can add conditional logic if needed
}

void Marklin6050Interface::serialPortChanged(const std::string& newPort)
{
  // For example, reconnect or log
  (void)newPort;
}
