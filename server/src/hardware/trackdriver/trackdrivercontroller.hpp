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

#ifndef TRAINTASTIC_SERVER_HARDWARE_TRACKDRIVER_TRACKDRIVERCONTROLLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_TRACKDRIVER_TRACKDRIVERCONTROLLER_HPP

#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>

#ifdef interface
  #undef interface // interface is defined in combaseapi.h
#endif

class Object;
class IdObject;
class TrackDriver;
class Train;
enum class BlockTrainDirection : uint8_t;

class TrackDriverController
{
public:
  using TrackDriverMap = std::unordered_map<uint32_t, std::shared_ptr<TrackDriver>>;

  virtual std::pair<uint32_t, uint32_t> trackDriverAddressMinMax() const = 0;
  [[nodiscard]] virtual bool isTrackDriverAvailable(uint32_t address) const;
  std::optional<uint32_t> getUnusedTrackDriverAddress() const;
  std::shared_ptr<TrackDriver> getTrackDriver(uint32_t address, Object& usedBy);
  void releaseTrackDriver(TrackDriver& trackDriver, Object& usedBy);

  virtual void trackDriverTrainAdded(uint32_t address, bool invertPolarity, const Train& train, BlockTrainDirection direction) = 0;
  virtual void trackDriverTrainFlipped(uint32_t address, const Train& train, BlockTrainDirection direction) = 0;
  virtual void trackDriverTrainRemoved(uint32_t address, const Train& train) = 0;

protected:
  TrackDriverMap m_trackDrivers;

  TrackDriverController(IdObject& interface);

  void addToWorld();
  void destroying();

private:
  TrackDriverController(const TrackDriverController&) = delete;
  TrackDriverController& operator=(const TrackDriverController&) = delete;

  std::shared_ptr<TrackDriverController> shared_ptr();
  IdObject& interface();
};

#endif
