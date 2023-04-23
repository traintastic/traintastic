/**
 * server/src/core/object.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CORE_OBJECT_HPP
#define TRAINTASTIC_SERVER_CORE_OBJECT_HPP

#include "objectptr.hpp"
#include <boost/signals2/signal.hpp>
#include <nlohmann/json.hpp>
#include "interfaceitems.hpp"
#include "argument.hpp"
#include <traintastic/enum/worldevent.hpp>
#include <traintastic/set/worldstate.hpp>

#define CLASS_ID(id) \
  public: \
    static constexpr std::string_view classId = id; \
    std::string_view getClassId() const override { return classId; }

template<class... Args> class Event;
class AbstractMethod;
class BaseProperty;
class AbstractProperty;
class AbstractObjectProperty;
class AbstractVectorProperty;
class AbstractAttribute;
class AbstractEvent;
class WorldLoader;
class WorldSaver;

class Object : public std::enable_shared_from_this<Object>
{
  friend class World;
  friend class WorldLoader;
  friend class WorldSaver;

  private:
    static nlohmann::json toJSON(WorldSaver& saver, const BaseProperty& baseProperty);
    static void loadJSON(WorldLoader& loader, BaseProperty& baseProperty, const nlohmann::json& value);

    bool m_dying; // TODO: atomic??

  protected:
    InterfaceItems m_interfaceItems;

    template<class... Args>
    inline void fireEvent(Event<Args...>& event, Args... args)
    {
      event.fire(std::forward<Args>(args)...);
    }

    virtual void destroying() {}
    virtual void load(WorldLoader& loader, const nlohmann::json& data);
    virtual void save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const;
    virtual void loaded();
    virtual void worldEvent(WorldState state, WorldEvent event);

  public:
    Object(const Object&) = delete;
    Object& operator =(const Object&) = delete;

    boost::signals2::signal<void (Object&)> onDestroying;
    boost::signals2::signal<void (BaseProperty&)> propertyChanged;
    boost::signals2::signal<void (AbstractAttribute&)> attributeChanged;
    boost::signals2::signal<void (const AbstractEvent&, const Arguments&)> onEventFired;

    Object();
    virtual ~Object() = default;

    inline bool dying() const noexcept { return m_dying; }
    void destroy();

    template <typename Derived>
    inline std::shared_ptr<const Derived> shared_ptr_c() const
    {
      return std::static_pointer_cast<const Derived>(shared_from_this());
    }

    template <typename Derived>
    std::shared_ptr<Derived> shared_ptr()
    {
      return std::static_pointer_cast<Derived>(shared_from_this());
    }

    virtual std::string_view getClassId() const = 0;
    virtual std::string getObjectId() const = 0;

    const InterfaceItems& interfaceItems() const { return m_interfaceItems; }

    const InterfaceItem* getItem(std::string_view name) const;
    InterfaceItem* getItem(std::string_view name);
    const AbstractMethod* getMethod(std::string_view name) const;
    AbstractMethod* getMethod(std::string_view name);
    const AbstractProperty* getProperty(std::string_view name) const;
    AbstractProperty* getProperty(std::string_view name);
    const AbstractObjectProperty* getObjectProperty(std::string_view name) const;
    AbstractObjectProperty* getObjectProperty(std::string_view name);
    const AbstractVectorProperty* getVectorProperty(std::string_view name) const;
    AbstractVectorProperty* getVectorProperty(std::string_view name);
};

#endif
