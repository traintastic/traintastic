/**
 * server/src/hardware/output/map/signaloutputmap.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_SIGNALOUTPUTMAP_HPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_SIGNALOUTPUTMAP_HPP

#include "outputmapbase.hpp"
#include "signaloutputmapitem.hpp"

class SignalOutputMap : public OutputMapBase<SignalAspect, SignalOutputMapItem>
{
  CLASS_ID("output_map.signal")

  public:
    SignalOutputMap(Object& _parent, std::string_view parentPropertyName, std::initializer_list<SignalAspect> aspects, DefaultOutputActionGetter defaultOutputActionGetter);

    template<std::size_t SpanExtent>
    SignalOutputMap(Object& _parent, std::string_view parentPropertyName, const tcb::span<const SignalAspect, SpanExtent> &aspects, DefaultOutputActionGetter defaultOutputActionGetter) :
      OutputMapBase(_parent, parentPropertyName, aspects, defaultOutputActionGetter)
    {
    }
};

#endif
