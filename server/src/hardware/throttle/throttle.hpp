/**
 * server/src/hardware/throttle/throttle.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_THROTTLE_THROTTLE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_THROTTLE_THROTTLE_HPP

#include "../../core/idobject.hpp"
#include <traintastic/enum/direction.hpp>
#include "../../core/property.hpp"
#include "../../core/objectproperty.hpp"
#include "../../core/objectvectorproperty.hpp"
#include "../../core/method.hpp"

class Decoder;
enum class DecoderProtocol : uint8_t;
class ThrottleFunction;
class Train;

class Throttle : public IdObject
{
  friend class ThrottleFunction;

  DEFAULT_ID("throttle")

  public:
    enum class AcquireResult
    {
      Success,
      FailedNonExisting,
      FailedInUse,
    };

  private:
    std::shared_ptr<Decoder> m_decoder;

  protected:
    Throttle(World& world, std::string_view _id);

    void destroying() override;
    void addToWorld() override;

    AcquireResult acquire(std::shared_ptr<Decoder> decoder, bool steal = false);

  public:
    static constexpr float throttleMin = 0;
    static constexpr float throttleStop = throttleMin;
    static constexpr float throttleMax = 1;

    boost::signals2::signal<void()> released;

    Property<std::string> name;
    Property<Direction> direction;
    Property<float> throttle;
    ObjectProperty<Train> train;
    Method<bool()> emergencyStop;
    Method<bool(bool)> stop;
    Method<bool(bool)> faster;
    Method<bool(bool)> slower;
    Method<bool(Direction)> setDirection;

#ifndef NDEBUG
    ~Throttle() override;
#endif

    bool acquired() const;
    std::error_code acquire(const std::shared_ptr<Train>& acquireTrain, bool steal = false);
    void release(bool stopIt = true);

    const std::shared_ptr<Decoder>& decoder() const // TODO: remove once WiThrottle is migrated to train control
    {
      return m_decoder;
    }
};

#endif
