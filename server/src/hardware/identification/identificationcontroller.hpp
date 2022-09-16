/**
 * server/src/hardware/identification/identificationcontroller.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_IDENTIFICATION_IDENTIFICATIONCONTROLLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_IDENTIFICATION_IDENTIFICATIONCONTROLLER_HPP

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>
#include <traintastic/enum/direction.hpp>
#include <traintastic/enum/identificationeventtype.hpp>
#include "../../core/objectproperty.hpp"
#include "../../enum/tristate.hpp"

#ifdef interface
  #undef interface // interface is defined in combaseapi.h
#endif

class IdObject;
class Identification;
class IdentificationMonitor;

class IdentificationList;
enum class IdentificationListColumn;

class IdentificationController
{
  public:
    struct IdentificationMapKey
    {
      uint32_t channel;
      uint32_t address;

      inline bool operator ==(const IdentificationMapKey other) const noexcept
      {
        return channel == other.channel && address == other.address;
      }
    };
    static_assert(sizeof(IdentificationMapKey) == sizeof(uint64_t));

    struct IdentificationMapKeyHash
    {
      std::size_t operator()(const IdentificationMapKey& value) const noexcept
      {
        return std::hash<uint64_t>{}(*reinterpret_cast<const uint64_t*>(&value));
      }
    };

    using IdentificationMap = std::unordered_map<IdentificationMapKey, std::shared_ptr<Identification>, IdentificationMapKeyHash>;

  private:
    IdObject& interface();

  protected:
    IdentificationMap m_identifications;
    std::unordered_map<uint32_t, std::weak_ptr<IdentificationMonitor>> m_identificationMonitors;

    IdentificationController(IdObject& interface);

    void addToWorld(IdentificationListColumn columns);
    void destroying();

  public:
    static constexpr std::vector<uint32_t>* noIdentificationChannels = nullptr;
    static constexpr uint32_t defaultIdentificationChannel = 0;

    ObjectProperty<IdentificationList> identifications;

    /**
     *
     */
    inline const IdentificationMap& identificationMap() const { return m_identifications; }

    /**
     *
     */
    virtual const std::vector<uint32_t>* identificationChannels() const { return noIdentificationChannels; }

    /**
     *
     */
    virtual const std::vector<std::string_view>* identificationChannelNames() const { return nullptr; }

    /**
     *
     */
    bool isIdentificationChannel(uint32_t channel) const;

    /**
     *
     */
    virtual std::pair<uint32_t, uint32_t> identificationAddressMinMax(uint32_t channel) const = 0;

    /**
     *
     */
    [[nodiscard]] virtual bool isIdentificationAddressAvailable(uint32_t channel, uint32_t address) const;

    /**
     * @brief Get the next unused identification address
     *
     * @return An usused address or #Identification::invalidAddress if no unused address is available.
     */
    uint32_t getUnusedIdentificationAddress(uint32_t channel) const;

    /**
     *
     * @return \c true if changed, \c false otherwise.
     */
    [[nodiscard]] virtual bool changeIdentificationChannelAddress(Identification& identification, uint32_t newChannel, uint32_t newAddress);

    /**
     *
     * @return \c true if added, \c false otherwise.
     */
    [[nodiscard]] bool addIdentification(Identification& identification);

    /**
     *
     * @return \c true if removed, \c false otherwise.
     */
    [[nodiscard]] bool removeIdentification(Identification& identification);

    /**
     * @brief Update the identification value
     *
     * This function should be called by the hardware layer whenever the identification value changes.
     *
     * @param[in] channel Identification channel
     * @param[in] address Identification address
     * @param[in] type ...
     * @param[in] type ...
     * @param[in] type ...
     * @param[in] type ...
     */
    virtual void identificationEvent(uint32_t channel, uint32_t address, IdentificationEventType type, uint16_t identifier, Direction direction, uint8_t category);
};

#endif
