/**
 * server/src/hardware/output/map/outputmapoutputaction.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAPOUTPUTACTION_HPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_MAP_OUTPUTMAPOUTPUTACTION_HPP

#include "../../../core/object.hpp"
#include "../output.hpp"
#include "traintastic/enum/tristate.hpp"

class OutputMap;
class World;

class OutputMapOutputAction : public Object
{
  friend class OutputMapItem;

  private:
    OutputMap& m_parent;
    const size_t m_outputIndex;

  protected:
    World& world();
    Output& output();
    const Output& output() const;

  public:
    OutputMapOutputAction(OutputMap& _parent, size_t outputIndex);

    std::string getObjectId() const final;

    virtual void execute() = 0;

    virtual TriState matchesCurrentOutputState() const = 0;
};

#endif
