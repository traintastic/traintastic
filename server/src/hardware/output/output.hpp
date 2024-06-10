/**
 * server/src/hardware/output/output.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2022,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_OUTPUT_HPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_OUTPUT_HPP

#include "../../core/object.hpp"
#include <set>
#include <traintastic/enum/outputchannel.hpp>
#include <traintastic/enum/outputtype.hpp>
#include "../../core/property.hpp"
#include "../../core/objectproperty.hpp"
#include "../../core/event.hpp"

#ifdef interface
  #undef interface // interface is defined in combaseapi.h
#endif

class OutputController;

class Output : public Object
{
  friend class OutputController;

  private:
    std::set<std::shared_ptr<Object>> m_usedBy; //!< Objects that use the output.

  protected:
    Output(std::shared_ptr<OutputController> outputController, OutputChannel channel_, OutputType type_);

  public:
    static constexpr uint32_t invalidAddress = std::numeric_limits<uint32_t>::max();

    ObjectProperty<OutputController> interface;
    Property<OutputChannel> channel;
    Property<OutputType> type;
    Event<const std::shared_ptr<Output>&> onValueChangedGeneric;

    /**
     * \brief Unique identifier for the output within the channel.
     *
     * \return Unique identifier, can be any number/mask.
     */
    virtual uint32_t id() const = 0;

    std::string getObjectId() const final;
};

#endif
