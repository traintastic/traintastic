#include "marklin6050interface.hpp"
#include "../../core/attributes.hpp"
#include "../../utils/displayname.hpp"
#include "../../log/log.hpp"
#include "../../core/eventloop.hpp"

CREATE_IMPL(Marklin6050Interface)

Marklin6050Interface::Marklin6050Interface(World& world, std::string_view _id)
  : Interface(world, _id)
  , device{this, "device", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
  , baudrate{this, "baudrate", 2400, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  name = "Märklin 6050";

  Attributes::addDisplayName(device, DisplayName::Serial::device);
  Attributes::addEnabled(device, !online);
  m_interfaceItems.insertBefore(device, notes);

  Attributes::addDisplayName(baudrate, DisplayName::Serial::baudrate);
  Attributes::addEnabled(baudrate, !online);
  m_interfaceItems.insertBefore(baudrate, notes);
}

Marklin6050Interface::~Marklin6050Interface() = default;

bool Marklin6050Interface::setOnline(bool& value, bool simulation)
{
  if (value) {
    Log::info(*this, "Opening Märklin 6050 serial connection on {} at {} baud", device.value(), baudrate.value());
    // TODO: Create serial connection with baudrate.value()
  } else {
    // TODO: Close serial connection
  }
  return true;
}

void Marklin6050Interface::addToWorld()
{
  Interface::addToWorld();
}

void Marklin6050Interface::loaded()
{
  Interface::loaded();
}

void Marklin6050Interface::destroying()
{
  Interface::destroying();
}



