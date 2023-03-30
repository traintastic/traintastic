/**
 * server/src/hardware/identification/identificationcontroller.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#include "identificationcontroller.hpp"
#include "identification.hpp"
#include "list/identificationlist.hpp"
#include "list/identificationlisttablemodel.hpp"
#include "../../core/attributes.hpp"
#include "../../core/objectproperty.tpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../world/world.hpp"

IdentificationController::IdentificationController(IdObject& interface)
  : identifications{&interface, "identifications", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
{
  Attributes::addDisplayName(identifications, DisplayName::Hardware::identifications);
}

bool IdentificationController::isIdentificationChannel(uint32_t channel) const
{
  const auto* channels = identificationChannels();
  if(!channels || channels->empty())
    return channel == defaultIdentificationChannel;

  auto it = std::find(channels->begin(), channels->end(), channel);
  assert(it == channels->end() || *it != defaultIdentificationChannel);
  return it != channels->end();
}

bool IdentificationController::isIdentificationAddressAvailable(uint32_t channel, uint32_t address) const
{
  assert(isIdentificationChannel(channel));
  return
    inRange(address, identificationAddressMinMax(channel)) &&
    m_identifications.find({channel, address}) == m_identifications.end();
}

uint32_t IdentificationController::getUnusedIdentificationAddress(uint32_t channel) const
{
  assert(isIdentificationChannel(channel));
  const auto end = m_identifications.cend();
  const auto range = identificationAddressMinMax(channel);
  for(uint32_t address = range.first; address < range.second; address++)
    if(m_identifications.find({channel, address}) == end)
      return address;
  return Identification::invalidAddress;
}

bool IdentificationController::changeIdentificationChannelAddress(Identification& identification, uint32_t newChannel, uint32_t newAddress)
{
  assert(identification.interface.value().get() == this);
  assert(isIdentificationChannel(newChannel));

  if(!isIdentificationAddressAvailable(newChannel, newAddress))
    return false;

  auto node = m_identifications.extract({identification.channel, identification.address});
  node.key() = {newChannel, newAddress};
  m_identifications.insert(std::move(node));

  return true;
}

bool IdentificationController::addIdentification(Identification& identification)
{
  if(isIdentificationChannel(identification.channel) && isIdentificationAddressAvailable(identification.channel, identification.address))
  {
    m_identifications.insert({{identification.channel, identification.address}, identification.shared_ptr<Identification>()});
    identifications->addObject(identification.shared_ptr<Identification>());
    return true;
  }
  return false;
}

bool IdentificationController::removeIdentification(Identification& identification)
{
  assert(identification.interface.value().get() == this);
  auto it = m_identifications.find({identification.channel, identification.address});
  if(it != m_identifications.end() && it->second.get() == &identification)
  {
    m_identifications.erase(it);
    identifications->removeObject(identification.shared_ptr<Identification>());
    return true;
  }
  return false;
}

void IdentificationController::identificationEvent(uint32_t channel, uint32_t address, IdentificationEventType type, uint16_t identifier, Direction direction, uint8_t category)
{
  if(auto it = m_identifications.find({channel, address}); it != m_identifications.end())
    it->second->fireEvent(type, identifier, direction, category);
}

void IdentificationController::addToWorld(IdentificationListColumn columns)
{
  auto& object = interface();
  identifications.setValueInternal(std::make_shared<IdentificationList>(object, identifications.name(), columns));
  object.world().identificationControllers->add(std::dynamic_pointer_cast<IdentificationController>(object.shared_from_this()));
}

void IdentificationController::destroying()
{
  auto& object = interface();
  while(!identifications->empty())
  {
    const auto& identification = identifications->front();
    assert(identification->interface.value() == std::dynamic_pointer_cast<IdentificationController>(object.shared_from_this()));
    identification->interface = nullptr; // removes object form the list
  }
  object.world().identificationControllers->remove(std::dynamic_pointer_cast<IdentificationController>(object.shared_from_this()));
}

IdObject& IdentificationController::interface()
{
  auto* object = dynamic_cast<IdObject*>(this);
  assert(object);
  return *object;
}
