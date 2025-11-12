#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_MARKLIN6050INTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_MARKLIN6050INTERFACE_HPP

#include "interface.hpp"
#include "../../core/objectproperty.hpp"
#include "../../hardware/protocol/Marklin6050Interface/serial.hpp"

class Marklin6050Interface : public Interface
{
  CLASS_ID("interface.marklin6050")
  DEFAULT_ID("marklin6050")
  CREATE_DEF(Marklin6050Interface)

private:
  Property<std::string> serialPort;

  void updateEnabled();
  void serialPortChanged(const std::string& newPort);

protected:
  void addToWorld() final;
  void loaded() final;
  void destroying() final;
  void worldEvent(WorldState state, WorldEvent event) final;
  void onlineChanged(bool value);  // remove 'final'

  bool setOnline(bool& value, bool simulation) final;

public:
  Marklin6050Interface(World& world, std::string_view id);
};

#endif
