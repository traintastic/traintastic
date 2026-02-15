/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "trackdrivercontroller.hpp"
#include "trackdriver.hpp"
#include "../../core/attributes.hpp"
#include "../../core/controllerlist.hpp"
#include "../../core/idobject.hpp"
#include "../../core/objectproperty.tpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../world/world.hpp"

TrackDriverController::TrackDriverController(IdObject& /*interface*/)
{
}

bool TrackDriverController::isTrackDriverAvailable(uint32_t address) const
{
  return
    inRange(address, trackDriverAddressMinMax()) &&
    m_trackDrivers.find(address) == m_trackDrivers.end();
}

std::optional<uint32_t> TrackDriverController::getUnusedTrackDriverAddress() const
{
  const auto end = m_trackDrivers.cend();
  const auto range = trackDriverAddressMinMax();
  for(uint32_t address = range.first; address < range.second; address++)
  {
    if(m_trackDrivers.find(address) == end)
    {
      return address;
    }
  }
  return std::nullopt;
}

std::shared_ptr<TrackDriver> TrackDriverController::getTrackDriver(uint32_t address, Object& usedBy)
{
  if(!inRange(address, trackDriverAddressMinMax()))
  {
    return {};
  }

  // Check if already exists:
  if(auto it = m_trackDrivers.find(address); it != m_trackDrivers.end())
  {
    it->second->m_usedBy.emplace(usedBy.shared_from_this());
    return it->second;
  }

  // Create new track driver:
  auto trackDriver = std::make_shared<TrackDriver>(shared_ptr(), address);
  trackDriver->m_usedBy.emplace(usedBy.shared_from_this());
  m_trackDrivers.emplace(address, trackDriver);

  return trackDriver;
}

void TrackDriverController::releaseTrackDriver(TrackDriver& trackDriver, Object& usedBy)
{
  auto trackDriverShared = trackDriver.shared_ptr<TrackDriver>();
  trackDriver.m_usedBy.erase(usedBy.shared_from_this());
  if(trackDriver.m_usedBy.empty())
  {
    const auto address = trackDriver.address.value();

    m_trackDrivers.erase(address);
    trackDriverShared->destroy();
    trackDriverShared.reset();
  }
}

void TrackDriverController::addToWorld()
{
  auto& object = interface();
  auto& world = object.world();
  world.trackDriverControllers->add(std::dynamic_pointer_cast<TrackDriverController>(object.shared_from_this()));
  world.enableFeature(WorldFeature::TrackDriverSystem);
}

void TrackDriverController::destroying()
{
  auto& object = interface();
  auto& world = object.world();
  while(!m_trackDrivers.empty())
  {
    const auto& trackDriver = m_trackDrivers.begin()->second;
    assert(trackDriver->interface.value() == std::dynamic_pointer_cast<TrackDriverController>(object.shared_from_this()));
    trackDriver->interface.setValueInternal(nullptr);
    trackDriver->destroy(); // notify consumers we're dying
  }
  world.trackDriverControllers->remove(std::dynamic_pointer_cast<TrackDriverController>(object.shared_from_this()));
  if(world.trackDriverControllers->length == 0)
  {
    world.disableFeature(WorldFeature::TrackDriverSystem);
  }
}

IdObject& TrackDriverController::interface()
{
  auto* object = dynamic_cast<IdObject*>(this);
  assert(object);
  return *object;
}

std::shared_ptr<TrackDriverController> TrackDriverController::shared_ptr()
{
  auto self = std::dynamic_pointer_cast<TrackDriverController>(interface().shared_from_this());
  assert(self);
  return self;
}
