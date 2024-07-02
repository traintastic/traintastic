/**
 * server/src/hardware/output/map/outputmapitem.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAPITEM_HPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAPITEM_HPP

#include "../../../core/object.hpp"
#include "../../../core/objectvectorproperty.hpp"
#include "traintastic/enum/tristate.hpp"

class Output;
class OutputMap;
class OutputMapOutputAction;

class OutputMapItem : public Object
{
  friend class OutputMap;

  protected:
    Object& m_map;

    void worldEvent(WorldState state, WorldEvent event) override;

  public:
    enum class MatchResult
    {
      FullMatch = 0,     // Every action matches
      PartialMatch = 1,  // Some actions do not match, probably switch operation is not completed yet
      WildcardMatch = 2, // Some action are set to "None", they match any state but are ranked below others
      NoMatch = 3        // None of the actions matches
    };

    ObjectVectorProperty<OutputMapOutputAction> outputActions;

    OutputMapItem(Object& map);

    void execute();

    MatchResult matchesCurrentOutputState() const;
};

#endif
