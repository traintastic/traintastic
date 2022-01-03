/**
 * server/src/core/abstractevent.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_ABSTRACTEVENT_HPP
#define TRAINTASTIC_SERVER_CORE_ABSTRACTEVENT_HPP

#include "interfaceitem.hpp"
#include <list>
#include "eventflags.hpp"
#include "argument.hpp"

class AbstractEventHandler;

class AbstractEvent : public InterfaceItem
{
  private:
    const EventFlags m_flags;
    std::list<std::shared_ptr<AbstractEventHandler>> m_handlers;

  protected:
    void fire(const Arguments& args);

  public:
    using ArgumentInfo = std::pair<ValueType, std::string_view>;

    AbstractEvent(Object& object, const std::string& name, EventFlags m_flags);

    inline bool isScriptable() const { return (m_flags & EventFlags::Scriptable) == EventFlags::Scriptable; }

    inline bool isInternal() const final
    {
      return true;
    }

    inline EventFlags flags() const { return m_flags; }

    /// @todo C++20 -> std::span ??
    virtual std::pair<const ArgumentInfo*, size_t> argumentInfo() const = 0;

    void connect(std::shared_ptr<AbstractEventHandler> handler);
    bool disconnect(const std::shared_ptr<AbstractEventHandler>& handler);
};

#endif
