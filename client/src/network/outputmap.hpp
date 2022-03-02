/**
 * client/src/network/outputmap.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_OUTPUTMAP_HPP
#define TRAINTASTIC_CLIENT_NETWORK_OUTPUTMAP_HPP

#include "object.hpp"
#include "objectptr.hpp"

class OutputMap final : public Object
{
  Q_OBJECT

  public:
    using Items = std::vector<ObjectPtr>;
    using Outputs = std::vector<ObjectPtr>;

  private:
    int m_getItemsRequestId;
    Items m_items;
    int m_getOutputsRequestId;
    Outputs m_outputs;

    void readOutputs(const Message& message);

  protected:
    void processMessage(const Message& message) final;

  public:
    inline static const QString classIdPrefix = QStringLiteral("output_map.");

    OutputMap(std::shared_ptr<Connection> connection, Handle handle, const QString& classId);
    ~OutputMap() final;

    const Items& items() const { return m_items; }
    void getItems();

    const Outputs& outputs() const { return m_outputs; }
    void getOutputs();

  signals:
    void itemsChanged();
    void outputsChanged();
};

#endif
