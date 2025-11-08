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

Marklin6050Interface::Marklin6050Interface(World& world, std::string_view idValue)
  : Interface(world, idValue)
{
  name = "Märklin 6050";

  // Example property setup for UI (if needed later)
  // Attributes::addDisplayName(propertyName, "Property Display Name");
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
  if (value)
  {
    setState(InterfaceState::Online);
  }
  else
  {
    setState(InterfaceState::Offline);
  }

  return true;
}

void Marklin6050Interface::updateEnabled()
{
  // Update UI property states based on online status or world mode
}
