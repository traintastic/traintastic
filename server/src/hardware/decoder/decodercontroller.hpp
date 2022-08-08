/**
 * server/src/hardware/decoder/decodercontroller.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_DECODER_DECODERCONTROLLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_DECODER_DECODERCONTROLLER_HPP

#include <cstdint>
#include <vector>
#include <memory>
#include "../../core/objectproperty.hpp"

class IdObject;
class Decoder;
enum class DecoderChangeFlags;
enum class DecoderProtocol : uint8_t;
class DecoderList;
enum class DecoderListColumn;

class DecoderController
{
  public:
    using DecoderVector = std::vector<std::shared_ptr<Decoder>>;

  private:
    DecoderVector m_decoders;

    IdObject& interface();

  protected:
    DecoderController(IdObject& interface, DecoderListColumn columns);

    void addToWorld();
    void destroying();

    DecoderVector::iterator findDecoder(const Decoder& decoder);
    DecoderVector::iterator findDecoder(DecoderProtocol protocol, uint16_t address, bool dccLongAddress = false);

    /// \brief restore speed of all decoders that are not (emergency) stopped
    void restoreDecoderSpeed();

  public:
    ObjectProperty<DecoderList> decoders;

    [[nodiscard]] bool addDecoder(Decoder& decoder);
    [[nodiscard]] bool removeDecoder(Decoder& decoder);

    const std::shared_ptr<Decoder>& getDecoder(DecoderProtocol protocol, uint16_t address, bool dccLongAddress = false, bool fallbackToProtocolAuto = false);

    virtual void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) = 0;
};

#endif
