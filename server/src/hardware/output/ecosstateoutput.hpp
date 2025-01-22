/**
 * server/src/hardware/output/ecosstateoutput.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_ECOSSTATEOUTPUT_HPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_ECOSSTATEOUTPUT_HPP

#include "output.hpp"
#include "../../core/method.hpp"
#include "../../core/event.hpp"

class ECoSStateOutput final : public Output
{
  friend class OutputController;

  CLASS_ID("output.ecos_state")

  protected:
    void updateValue(uint8_t newValue);

  public:
    Property<uint16_t> ecosObjectId;
    Property<uint8_t> value;
    Method<bool(uint8_t)> setValue;
    Event<uint8_t, const std::shared_ptr<ECoSStateOutput>&> onValueChanged;

    ECoSStateOutput(std::shared_ptr<OutputController> outputController, OutputChannel channel_, uint16_t ecosObjectId_);

    uint32_t id() const final
    {
      return ecosObjectId.value();
    }
};

#endif
