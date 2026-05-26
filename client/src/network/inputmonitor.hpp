/**
 * client/src/network/inputmonitor.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_INPUTMONITOR_HPP
#define TRAINTASTIC_CLIENT_NETWORK_INPUTMONITOR_HPP

#include "object.hpp"
#include <unordered_map>
#include <traintastic/enum/tristate.hpp>

class InputMonitor final : public Object
{
  Q_OBJECT

  public:
    struct InputState
    {
      bool used = false;
      TriState value = TriState::Undefined;
    };

  private:
    int m_requestId;
    std::unordered_map<uint32_t, InputState> m_inputStates;
    Method* m_simulateInputChange = nullptr;
    Event* m_inputUsedChanged = nullptr;
    Event* m_inputValueChanged = nullptr;

    void created() final;

  public:
    inline static const QString classId = QStringLiteral("input_monitor");

    InputMonitor(const std::shared_ptr<Connection>& connection, Handle handle, const QString& classId_);
    ~InputMonitor() final;

    const InputState& getInputState(uint32_t address) const;

    void simulateInputChange(uint32_t address);

  signals:
    void inputStateChanged(uint32_t address);
};

#endif
