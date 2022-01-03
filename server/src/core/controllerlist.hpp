/**
 * server/src/core/controllerlist.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CORE_CONTROLLERLIST_HPP
#define TRAINTASTIC_SERVER_CORE_CONTROLLERLIST_HPP

#include "controllerlistbase.hpp"

template<class T>
class ControllerList : public ControllerListBase
{
  public:
    ControllerList(Object& _parent, const std::string& parentPropertyName)
      : ControllerListBase(_parent, parentPropertyName)
    {
    }

    inline void add(const std::shared_ptr<T>& controller)
    {
      if(ObjectPtr object = std::dynamic_pointer_cast<Object>(controller))
        ControllerListBase::add(std::move(object));
      else
        assert(false);
    }

    inline void remove(const std::shared_ptr<T>& controller)
    {
      ControllerListBase::remove(std::dynamic_pointer_cast<Object>(controller));
    }
};

#endif
