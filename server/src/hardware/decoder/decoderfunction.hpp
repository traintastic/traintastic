/**
 * server/src/hardware/decoder/decoderfunction.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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

#ifndef SERVER_HARDWARE_DECODER_DECODERFUNCTION_HPP
#define SERVER_HARDWARE_DECODER_DECODERFUNCTION_HPP

#include "../../core/output.hpp"
#include "../../core/objectproperty.hpp"
#include "../commandstation/commandstation.hpp"

namespace Hardware {

class Decoder;

class DecoderFunction : public Output
{
  public://protected:
    Decoder* m_decoder;

  protected:
    bool setValue(bool& value) final;
    void valueChanged(bool) final;

  public:
    CLASS_ID("hardware.decoder_function")
    CREATE(DecoderFunction)

    static const std::shared_ptr<DecoderFunction> null;

    DecoderFunction(const std::weak_ptr<World>& world, const std::string& _id);

    Property<uint8_t> number;
    Property<std::string> name;
    Property<bool> momentary;
};

}

#endif
