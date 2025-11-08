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
  void addToWorld() final;
  void loaded() final;
  void destroying() final;
  void worldEvent(WorldState state, WorldEvent event) final;
  void onlineChanged(bool value) final;
  bool setOnline(bool& value, bool simulation) final;

public:
  Marklin6050Interface(World& world, std::string_view id);
};

#endif

