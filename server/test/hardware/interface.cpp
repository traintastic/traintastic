/**
 * server/test/hardware/interface.cpp
 *
 * This file is part of the traintastic test suite.
 *
 * Copyright (C) 2023-2025 Reinder Feenstra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <catch2/catch_template_test_macros.hpp>
#include "../src/world/world.hpp"
#include "../src/core/method.tpp"
#include "../src/core/objectproperty.tpp"
#include "../src/hardware/interface/interfacelist.hpp"
#include "../src/hardware/decoder/list/decoderlist.hpp"
#include "../src/hardware/input/input.hpp"
#include "../src/hardware/input/list/inputlist.hpp"
#include "../src/hardware/output/list/outputlist.hpp"
#include "../src/hardware/identification/identification.hpp"
#include "../src/hardware/identification/list/identificationlist.hpp"
#include "../src/vehicle/rail/locomotive.hpp"
#include "../src/vehicle/rail/railvehiclelist.hpp"
#include "../src/train/train.hpp"
#include "interfaces.hpp"

TEMPLATE_TEST_CASE("Assign decoder to another interface", "[interface]", INTERFACES_DECODER)
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());
  REQUIRE(worldWeak.lock()->interfaces->length == 0);
  REQUIRE(worldWeak.lock()->decoders->length == 0);

  std::weak_ptr<TestType> interfaceWeak1 = std::dynamic_pointer_cast<TestType>(world->interfaces->create(TestType::classId));
  REQUIRE_FALSE(interfaceWeak1.expired());
  REQUIRE(worldWeak.lock()->interfaces->length == 1);
  REQUIRE(worldWeak.lock()->decoders->length == 0);
  REQUIRE(interfaceWeak1.lock()->decoders->length == 0);

  std::weak_ptr<TestType> interfaceWeak2 = std::dynamic_pointer_cast<TestType>(world->interfaces->create(TestType::classId));
  REQUIRE_FALSE(interfaceWeak2.expired());
  REQUIRE(worldWeak.lock()->interfaces->length == 2);
  REQUIRE(worldWeak.lock()->decoders->length == 0);
  REQUIRE(interfaceWeak2.lock()->decoders->length == 0);

  std::weak_ptr<Locomotive> locomotiveWeak = std::dynamic_pointer_cast<Locomotive>(world->railVehicles->create(Locomotive::classId));
  REQUIRE_FALSE(locomotiveWeak.expired());
  REQUIRE(locomotiveWeak.lock()->decoder);
  REQUIRE_FALSE(locomotiveWeak.lock()->decoder->interface);

  std::weak_ptr<Decoder> decoderWeak = locomotiveWeak.lock()->decoder.value();
  REQUIRE_FALSE(decoderWeak.expired());
  REQUIRE(worldWeak.lock()->interfaces->length == 2);
  REQUIRE(worldWeak.lock()->decoders->length == 1);
  REQUIRE(interfaceWeak1.lock()->decoders->length == 0);
  REQUIRE(interfaceWeak2.lock()->decoders->length == 0);

  decoderWeak.lock()->interface = interfaceWeak1.lock();
  REQUIRE_FALSE(decoderWeak.expired());
  REQUIRE(decoderWeak.lock()->interface.value() == std::dynamic_pointer_cast<DecoderController>(interfaceWeak1.lock()));
  REQUIRE(worldWeak.lock()->interfaces->length == 2);
  REQUIRE(worldWeak.lock()->decoders->length == 1);
  REQUIRE(interfaceWeak1.lock()->decoders->length == 1);
  REQUIRE(interfaceWeak2.lock()->decoders->length == 0);

  decoderWeak.lock()->interface = interfaceWeak2.lock();
  REQUIRE_FALSE(decoderWeak.expired());
  REQUIRE(decoderWeak.lock()->interface.value() == std::dynamic_pointer_cast<DecoderController>(interfaceWeak2.lock()));
  REQUIRE(worldWeak.lock()->interfaces->length == 2);
  REQUIRE(worldWeak.lock()->decoders->length == 1);
  REQUIRE(interfaceWeak1.lock()->decoders->length == 0);
  REQUIRE(interfaceWeak2.lock()->decoders->length == 1);

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(interfaceWeak1.expired());
  REQUIRE(interfaceWeak2.expired());
  REQUIRE(locomotiveWeak.expired());
  REQUIRE(decoderWeak.expired());
}


#ifndef __APPLE__

TEMPLATE_TEST_CASE("Assign identification to another interface", "[interface]", INTERFACES_IDENTIFICATION)
{
  auto world = World::create();
  std::weak_ptr<World> worldWeak = world;
  REQUIRE_FALSE(worldWeak.expired());
  REQUIRE(worldWeak.lock()->interfaces->length == 0);
  REQUIRE(worldWeak.lock()->identifications->length == 0);

  std::weak_ptr<TestType> interfaceWeak1 = std::dynamic_pointer_cast<TestType>(world->interfaces->create(TestType::classId));
  REQUIRE_FALSE(interfaceWeak1.expired());
  REQUIRE(worldWeak.lock()->interfaces->length == 1);
  REQUIRE(worldWeak.lock()->identifications->length == 0);
  REQUIRE(interfaceWeak1.lock()->identifications->length == 0);

  std::weak_ptr<TestType> interfaceWeak2 = std::dynamic_pointer_cast<TestType>(world->interfaces->create(TestType::classId));
  REQUIRE_FALSE(interfaceWeak2.expired());
  REQUIRE(worldWeak.lock()->interfaces->length == 2);
  REQUIRE(worldWeak.lock()->identifications->length == 0);
  REQUIRE(interfaceWeak2.lock()->identifications->length == 0);

  std::weak_ptr<Identification> identificationWeak = interfaceWeak1.lock()->identifications->create();
  REQUIRE_FALSE(identificationWeak.expired());
  REQUIRE(identificationWeak.lock()->interface.value() == std::dynamic_pointer_cast<IdentificationController>(interfaceWeak1.lock()));
  REQUIRE(worldWeak.lock()->interfaces->length == 2);
  REQUIRE(worldWeak.lock()->identifications->length == 1);
  REQUIRE(interfaceWeak1.lock()->identifications->length == 1);
  REQUIRE(interfaceWeak2.lock()->identifications->length == 0);

  identificationWeak.lock()->interface = interfaceWeak2.lock();
  REQUIRE_FALSE(identificationWeak.expired());
  REQUIRE(identificationWeak.lock()->interface.value() == std::dynamic_pointer_cast<IdentificationController>(interfaceWeak2.lock()));
  REQUIRE(worldWeak.lock()->interfaces->length == 2);
  REQUIRE(worldWeak.lock()->identifications->length == 1);
  REQUIRE(interfaceWeak1.lock()->identifications->length == 0);
  REQUIRE(interfaceWeak2.lock()->identifications->length == 1);

  world.reset();
  REQUIRE(worldWeak.expired());
  REQUIRE(interfaceWeak1.expired());
  REQUIRE(interfaceWeak2.expired());
  REQUIRE(identificationWeak.expired());
}

#endif
