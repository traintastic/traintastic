/**
 * client/src/network/outputkeyboard.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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
#include <traintastic/enum/tristate.hpp>

class OutputKeyboard final : public Object
{
  Q_OBJECT

  private:
    int m_requestId;

  public:
    inline static const QString classId = QStringLiteral("output_keyboard");

    OutputKeyboard(std::shared_ptr<Connection> connection, Handle handle, const QString& classId);
    ~OutputKeyboard() final;

    void refresh();

  public slots:
    void outputSetValue(uint32_t address, bool value);

  signals:
    void outputIdChanged(uint32_t address, QString id);
    void outputValueChanged(uint32_t address, TriState value);
};

#endif
