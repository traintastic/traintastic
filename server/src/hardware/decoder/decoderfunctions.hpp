/**
 * server/src/hardware/decoder/decoderfunctions.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2023,2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_DECODER_DECODERFUNCTIONS_HPP
#define TRAINTASTIC_SERVER_HARDWARE_DECODER_DECODERFUNCTIONS_HPP

#include "../../core/subobject.hpp"
#include "../../core/objectvectorproperty.hpp"
#include "../../core/method.hpp"
#include "decoderfunction.hpp"

class DecoderFunctions : public SubObject
{
  protected:
    void load(WorldLoader& loader, const nlohmann::json& data) final;
    void worldEvent(WorldState state, WorldEvent event) final;

  public:
    CLASS_ID("decoder_functions")

    ObjectVectorProperty<DecoderFunction> items;
    Method<void()> create;
    Method<void(const std::shared_ptr<DecoderFunction>&)> delete_;
    Method<void(const std::shared_ptr<DecoderFunction>&)> moveUp;
    Method<void(const std::shared_ptr<DecoderFunction>&)> moveDown;

    using const_iterator = ObjectVectorProperty<DecoderFunction>::const_iterator;

    DecoderFunctions(Object& _parent, std::string_view parentPropertyName);

    inline const_iterator begin() const { return items.begin(); }
    inline const_iterator end() const { return items.end(); }

    inline bool empty() const
    {
      return items.empty();
    }
};

#endif
