/**
 * server/src/hardware/decoder/decoderlist.hpp
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


#ifndef SERVER_HARDWARE_DECODER_DECODERLIST_HPP
#define SERVER_HARDWARE_DECODER_DECODERLIST_HPP

#include "../../core/objectlist.hpp"
#include "decoderlist.hpp"
#include "decoder.hpp"

class DecoderList : public ObjectList<Hardware::Decoder>
{
  protected:
    bool isListedProperty(const std::string& name) final;

  public:
    CLASS_ID("decoder_list")

    DecoderList(Object& parent, const std::string& parentPropertyName);

    TableModelPtr getModel() final;
};

#endif
