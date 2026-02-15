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

#ifndef TRAINTASTIC_SERVER_HARDWARE_TRACKDRIVER_TRACKDRIVERCONSUMER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_TRACKDRIVER_TRACKDRIVERCONSUMER_HPP

#include <boost/signals2/connection.hpp>
#include "../../core/property.hpp"
#include "../../core/objectproperty.hpp"

enum class WorldState : uint32_t;
enum class WorldEvent : uint64_t;

class World;
class Object;
class InterfaceItems;
class TrackDriver;
class TrackDriverController;

class TrackDriverConsumer
{
public:
  ObjectProperty<TrackDriverController> interface;
  Property<uint32_t> address;
  Property<bool> invertPolarity;

  TrackDriverConsumer(Object& object, const World& world);
  virtual ~TrackDriverConsumer();

  const std::shared_ptr<TrackDriver>& trackDriver()
  {
    return m_trackDriver;
  }

protected:
  void loaded();
  void worldEvent(WorldState worldState, WorldEvent worldEvent);

  void addInterfaceItems(InterfaceItems& items);

private:
  Object& m_object;
  std::shared_ptr<TrackDriver> m_trackDriver;
  boost::signals2::connection m_trackDriverDestroying;

  void setTrackDriver(std::shared_ptr<TrackDriver> trackDriver);
  void releaseTrackDriver();

  void interfaceChanged();
};

#endif
