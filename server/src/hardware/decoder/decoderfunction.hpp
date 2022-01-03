/**
 * server/src/hardware/decoder/decoderfunction.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_DECODER_DECODERFUNCTION_HPP
#define TRAINTASTIC_SERVER_HARDWARE_DECODER_DECODERFUNCTION_HPP

#include "../../core/object.hpp"
#include "../../core/property.hpp"
#include "../../enum/decoderfunctiontype.hpp"
#include "../../enum/decoderfunctionfunction.hpp"

class Decoder;

class DecoderFunction : public Object
{
  private:
    void typeChanged();

  protected:
    Decoder& m_decoder;

    void loaded() override;
    void worldEvent(WorldState state, WorldEvent event) final;

  public:
    CLASS_ID("decoder_function")

    static const std::shared_ptr<DecoderFunction> null;

    Property<uint8_t> number;
    Property<std::string> name;
    Property<DecoderFunctionType> type;
    Property<DecoderFunctionFunction> function;
    Property<bool> value;

    DecoderFunction(Decoder& decoder, uint8_t _number);

    std::string getObjectId() const final;

    const Decoder& decoder() const { return m_decoder; }
    Decoder& decoder() { return m_decoder; }
};

#endif
