/**
 * server/src/hardware/interface/interfacelist.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_INTERFACELIST_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_INTERFACELIST_HPP

#include "../../core/objectlist.hpp"
#include "../../core/method.hpp"
#include "interface.hpp"

class InterfaceList final : public ObjectList<Interface>
{
  private:
    std::unordered_map<Object*, boost::signals2::connection> m_statusPropertyChanged;

    void statusPropertyChanged(BaseProperty& property);

  protected:
    void worldEvent(WorldState state, WorldEvent event) final;
    bool isListedProperty(std::string_view name) final;

    void objectAdded(const std::shared_ptr<Interface>& object) final;
    void objectRemoved(const std::shared_ptr<Interface>& object) final;

  public:
    CLASS_ID("list.interface")

    Method<std::shared_ptr<Interface>(std::string_view)> create;
    Method<void(const std::shared_ptr<Interface>&)> delete_;

    InterfaceList(Object& _parent, std::string_view parentPropertyName);
    ~InterfaceList() final;

    TableModelPtr getModel() final;
};

#endif
