/**
 * server/src/hardware/interface/Marklin6050Interface.hpp
 *
 * Dummy Märklin 6050 interface for Traintastic
 * Copyright (C) 2025
 */

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_MARKLIN6050INTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_MARKLIN6050INTERFACE_HPP

#include "interface.hpp"
#include "../../core/objectproperty.hpp"

/**
 * \brief Dummy interface for the Märklin 6050 hardware
 */
class Marklin6050Interface final : public Interface
{
  CLASS_ID("interface.marklin6050")
  DEFAULT_ID("marklin6050")
  CREATE_DEF(Marklin6050Interface)

private:
  void updateEnabled();

protected:
  void addToWorld() override;
  void loaded() override;
  void destroying() override;
  void worldEvent(WorldState state, WorldEvent event) override;
  bool setOnline(bool& value, bool simulation) override;

  // Custom method for handling changes to serial port selection
  void serialPortChanged(const std::string& newPort);
  void onlineChanged(bool value);

public:
  explicit Marklin6050Interface(World& world, std::string_view idValue);

  // === Properties ===
  Property<std::string> serialPort;
};

#endif
