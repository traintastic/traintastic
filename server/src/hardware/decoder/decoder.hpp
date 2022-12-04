/**
 * server/src/hardware/decoder/decoder.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_DECODER_DECODER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_DECODER_DECODER_HPP

#include <type_traits>
#include "../../core/idobject.hpp"
#include "../../core/objectproperty.hpp"
#include "../../enum/decoderprotocol.hpp"
#include "../../enum/direction.hpp"
#include "decodercontroller.hpp"
#include "decoderfunctions.hpp"

#ifdef interface
  #undef interface // interface is defined in combaseapi.h
#endif

enum class DecoderChangeFlags;
class DecoderFunction;
class Throttle;

class Decoder : public IdObject
{
  friend class DecoderFunction;

  private:
    bool m_worldMute;
    bool m_worldNoSmoke;
    std::shared_ptr<Throttle> m_driver;

  protected:
    void loaded() final;
    void destroying() override;
    void worldEvent(WorldState state, WorldEvent event) final;
    void updateEditable();
    void updateEditable(bool editable);
    void changed(DecoderChangeFlags changes, uint32_t functionNumber = 0);

  public:
    CLASS_ID("decoder")
    CREATE(Decoder)

    static constexpr uint8_t speedStepsAuto = 0;
    static constexpr float throttleMin = 0;
    static constexpr float throttleStop = throttleMin;
    static constexpr float throttleMax = 1;

    inline static uint8_t throttleToSpeedStep(float throttle, uint8_t speedStepMax)
    {
      return static_cast<uint8_t>(std::lround(std::clamp(throttle, throttleMin, throttleMax) * speedStepMax));
    }

    inline static float speedStepToThrottle(uint8_t speedStep, uint8_t speedStepMax)
    {
      if(speedStepMax != 0)
        return static_cast<float>(std::clamp<uint8_t>(speedStep, 0, speedStepMax)) / speedStepMax;
      else
        return 0;
    }

    static const std::shared_ptr<Decoder> null;

    Property<std::string> name;
    ObjectProperty<DecoderController> interface;
    Property<DecoderProtocol> protocol;
    Property<uint16_t> address;
    Property<bool> longAddress;
    Property<bool> emergencyStop;
    Property<Direction> direction;
    Method<void()> toggleDirection;
    Property<uint8_t> speedSteps;
    Property<float> throttle;
    ObjectProperty<DecoderFunctions> functions;
    Property<std::string> notes;

    boost::signals2::signal<void (Decoder&, DecoderChangeFlags, uint32_t)> decoderChanged;

    Decoder(World& world, std::string_view _id);

    void addToWorld() final;
    bool hasFunction(uint32_t number) const;
    std::shared_ptr<const DecoderFunction> getFunction(uint32_t number) const;
    const std::shared_ptr<DecoderFunction>& getFunction(uint32_t number);
    std::shared_ptr<const DecoderFunction> getFunction(DecoderFunctionFunction function) const;
    const std::shared_ptr<DecoderFunction>& getFunction(DecoderFunctionFunction function);
    bool getFunctionValue(uint32_t number) const;
    bool getFunctionValue(const std::shared_ptr<const DecoderFunction>& function) const;
    void setFunctionValue(uint32_t number, bool value);

    bool acquire(Throttle& driver, bool steal = false);
    void release(Throttle& driver);
};

#endif
