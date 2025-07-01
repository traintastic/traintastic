/**
 * server/src/throttle/throttle.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022,2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_THROTTLE_THROTTLE_HPP
#define TRAINTASTIC_SERVER_THROTTLE_THROTTLE_HPP

#include "../core/nonpersistentobject.hpp"
#include <unordered_set>
#include <traintastic/enum/direction.hpp>
#include <traintastic/enum/speedunit.hpp>
#include "../core/event.hpp"
#include "../core/property.hpp"
#include "../core/objectproperty.hpp"
#include "../core/objectvectorproperty.hpp"
#include "../core/method.hpp"
#include "../utils/stringequal.hpp"
#include "../utils/stringhash.hpp"

class Decoder;
enum class DecoderProtocol : uint8_t;
class Train;
class World;

class Throttle : public NonPersistentObject
{
  public:
    enum class AcquireResult
    {
      Success,
      FailedNonExisting,
      FailedInUse,
    };

  private:
    static std::unordered_set<std::string, StringHash, StringEqual> s_logIds;

  protected:
    static std::string_view getUniqueLogId(std::string_view prefix = "throttle");

    World& m_world;
    const std::string_view m_logId;

    Throttle(World& world, std::string_view logId);

    void destroying() override;
    virtual void addToList();

  public:
    Property<std::string> name;
    ObjectProperty<Train> train;
    Event<const std::shared_ptr<Throttle>&, const std::shared_ptr<Train>&> onAcquire;
    Event<const std::shared_ptr<Throttle>&> onRelease;

    ~Throttle() override;

    bool acquired() const;
    std::error_code acquire(const std::shared_ptr<Train>& acquireTrain, bool steal = false);
    void release(bool stopIt = true);

    bool emergencyStop();
    bool setDirection(Direction value);
    bool setSpeed(double value, SpeedUnit unit);
    bool setTargetSpeed(double value, SpeedUnit unit);
    bool slower(bool immediate);
    bool faster(bool immediate);
};

#endif
