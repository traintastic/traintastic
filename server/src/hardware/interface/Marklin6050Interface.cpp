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

Marklin6050Interface::Marklin6050Interface(World& world, std::string_view idValue)
  : Interface(world, idValue),
    serialPort{*this, "serialPort", ""}
{
  name = "Märklin 6050";

  // Setup attributes (used by the Traintastic editor)
  Attributes::addDisplayName(serialPort, "Serial Port");
  Attributes::addChoices(serialPort, Serial::getPortList);

  // Connect property change signal to handler
  serialPort.afterChange().connect([this](const std::string& value) {
    serialPortChanged(value);
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
      // Future hardware communication when powered on
      break;

    case WorldEvent::PowerOff:
      // Future hardware shutdown sequence
      break;

    default:
      break;
  }
}

bool Marklin6050Interface::setOnline(bool& value, bool /*simulation*/)
{
  if (value)
  {
    // Attempt to connect to serial port if available
    if (!serialPort().empty())
    {
      // For now just mark as online
      setState(InterfaceState::Online);
    }
    else
    {
      // Stay offline if no serial port is configured
      value = false;
      setState(InterfaceState::Offline);
    }
  }
  else
  {
    setState(InterfaceState::Offline);
  }

  return true;
}

void Marklin6050Interface::onlineChanged(bool /*value*/)
{
  updateEnabled();
}

void Marklin6050Interface::updateEnabled()
{
  // Disable serialPort editing when online
  serialPort.setEnabled(!isOnline());
}

void Marklin6050Interface::serialPortChanged(const std::string& newPort)
{
  // Called when serial port property changes
  if (isOnline())
  {
    // Reconnection logic could go here
  }
}
