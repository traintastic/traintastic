#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_MARKLIN6050INTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_MARKLIN6050INTERFACE_HPP

#include "interface.hpp"
#include "../../core/serialdeviceproperty.hpp"
#include "../../core/objectproperty.hpp"
#include <memory>

class Marklin6050Interface final : public Interface
{
  CLASS_ID("interface.marklin6050")
  DEFAULT_ID("marklin6050")
  CREATE_DEF(Marklin6050Interface)

private:
  std::unique_ptr<class SerialConnection> m_connection;

  void addToWorld() final;
  void loaded() final;
  void destroying() final;

protected:
  bool setOnline(bool& value, bool simulation) final;

public:
  SerialDeviceProperty device;
  Property<uint32_t> baudrate;

  Marklin6050Interface(World& world, std::string_view _id);
  ~Marklin6050Interface() final;
};

#endif
