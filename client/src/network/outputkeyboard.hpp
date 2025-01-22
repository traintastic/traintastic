/**
 * client/src/network/outputkeyboard.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_OUTPUTKEYBOARD_HPP
#define TRAINTASTIC_CLIENT_NETWORK_OUTPUTKEYBOARD_HPP

#include "object.hpp"
#include <unordered_map>
#include <variant>
#include <traintastic/enum/tristate.hpp>
#include <traintastic/enum/outputpairvalue.hpp>
#include <traintastic/enum/outputtype.hpp>

class OutputKeyboard final : public Object
{
  Q_OBJECT

  public:
    struct OutputState
    {
      bool used = false;
      std::variant<std::monostate, TriState, OutputPairValue> value;
    };

  private:
    int m_requestId;
    std::unordered_map<uint32_t, OutputState> m_outputStates;
    OutputType m_outputType = static_cast<OutputType>(0);
    Event* m_outputUsedChanged = nullptr;
    Event* m_outputValueChanged = nullptr;

    void created() final;

  public:
    inline static const QString classIdPrefix = QStringLiteral("output_keyboard.");

    OutputKeyboard(std::shared_ptr<Connection> connection, Handle handle, const QString& classId_);
    ~OutputKeyboard() final;

    OutputType outputType() const { return m_outputType; }
    const OutputState& getOutputState(uint32_t address) const;

  signals:
    void outputStateChanged(uint32_t address);
};

#endif
