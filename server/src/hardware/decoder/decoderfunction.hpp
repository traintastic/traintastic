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

#include "../../core/idobject.hpp"
#include "../../core/objectproperty.hpp"
#include "../../enum/decoderfunctiontype.hpp"

class Decoder;

class DecoderFunction : public IdObject
{
  private:
    void typeChanged();

  protected:
    Decoder& m_decoder;

    void save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const override;
    void loaded() override;
    void worldEvent(WorldState state, WorldEvent event) final;

   // void addToWorld() final { Output::addToWorld(); }

  public:
    CLASS_ID("decoder_function")

    static const std::shared_ptr<DecoderFunction> null;
    static std::shared_ptr<DecoderFunction> create(Decoder& decoder, std::string_view _id);

    Property<uint8_t> number;
    Property<std::string> name;
    Property<DecoderFunctionType> type;
    Property<bool> value;

    DecoderFunction(Decoder& decoder, std::string_view _id);

    Decoder& decoder() { return m_decoder; }
};

#endif
