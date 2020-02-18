/**
 * server/src/core/object.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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

#ifndef SERVER_CORE_OBJECT_HPP
#define SERVER_CORE_OBJECT_HPP

#include "objectptr.hpp"
#include <boost/signals2/signal.hpp>
#include "interfaceitems.hpp"
#include <enum/traintasticmode.hpp>

#define CLASS_ID(id) \
  public: \
    static constexpr std::string_view classId = id; \
    const std::string_view& getClassId() const override { return classId; }

class AbstractMethod;
class AbstractProperty;
class AbstractAttribute;

class Object : public std::enable_shared_from_this<Object>
{
  friend class World;

  protected:
    InterfaceItems m_interfaceItems;

    //void log(Console::Level level, const std::string& id, const std::string& message) const;
    //inline void logError(const std::string& id, const std::string& message) const { log(Console::Level::Error, message); }

    virtual void modeChanged(TraintasticMode mode);

  public:
    boost::signals2::signal<void (AbstractProperty&)> propertyChanged;
    boost::signals2::signal<void (AbstractAttribute&)> attributeChanged;

    Object();
    virtual ~Object();

    template <typename Derived>
    std::shared_ptr<Derived> shared_ptr()
    {
      return std::static_pointer_cast<Derived>(shared_from_this());
    }

    virtual const std::string_view& getClassId() const = 0;
    //virtual const std::string& getId() const = 0;

    const InterfaceItems& interfaceItems() const { return m_interfaceItems; }

    InterfaceItem* getItem(const std::string& name);
    //AbstractMethod* getMethod(const std::string& name);
    AbstractProperty* getProperty(const std::string& name);
};

#endif
