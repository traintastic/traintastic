/**
 * server/src/core/objectlist.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_OBJECTLIST_HPP
#define TRAINTASTIC_SERVER_CORE_OBJECTLIST_HPP

#include "abstractobjectlist.hpp"
#include <cassert>
#include "idobject.hpp"
#include "subobject.hpp"

template<typename T>
class ObjectListTableModel;

template<typename T>
class ObjectList : public AbstractObjectList
{
  friend class ObjectListTableModel<T>;

  public:
    using Items = std::vector<std::shared_ptr<T>>;
    using const_iterator = typename Items::const_iterator;

  protected:
    Items m_items;
    std::unordered_map<Object*, boost::signals2::scoped_connection> m_propertyChanged;
    std::vector<ObjectListTableModel<T>*> m_models;

    void deleteMethodHandler(const std::shared_ptr<T>& object)
    {
      if(containsObject(object))
      {
#ifndef NDEBUG
        std::weak_ptr<T> weak = object;
#endif
        object->destroy(); // NOTE: object might not be valid after this call or point to another object (if it was a reference to the modified list)
        assert(weak.expired() || !containsObject(weak.lock()));
      }
    }

    std::vector<ObjectPtr> getItems() const
    {
      std::vector<ObjectPtr> items;
      items.reserve(m_items.size());
      for(auto& item : m_items)
        items.emplace_back(std::move(std::static_pointer_cast<Object>(item)));
      return items;
    }

    void setItems(const std::vector<ObjectPtr>& items)
    {
      m_propertyChanged.clear();
      m_items.clear();
      m_items.reserve(items.size());
      for(auto& item : items)
        if(std::shared_ptr<T> t = std::dynamic_pointer_cast<T>(item))
          addObject(std::move(t));
      rowCountChanged();
    }

    virtual void objectAdded(const std::shared_ptr<T>& /*object*/) {}
    virtual void objectRemoved(const std::shared_ptr<T>& /*object*/) {}

    virtual bool isListedProperty(std::string_view name) = 0;

    virtual void propertyChanged(BaseProperty& property)
    {
      if(!m_models.empty() && isListedProperty(property.name()))
      {
        ObjectPtr obj = property.object().shared_from_this();
        const uint32_t rows = static_cast<uint32_t>(m_items.size());
        for(uint32_t row = 0; row < rows; row++)
          if(m_items[row] == obj)
          {
            for(auto& model : m_models)
              model->propertyChanged(property, row);
            break;
          }
      }
    }

    void rowCountChanged()
    {
      const auto size = m_items.size();
      length.setValueInternal(static_cast<uint32_t>(size));
      for(auto& model : m_models)
        model->setRowCount(static_cast<uint32_t>(size));
    }

    void rowsChanged(uint32_t first, uint32_t last)
    {
      for(auto& model : m_models)
      {
        model->rowsChanged(first, last);
      }
    }

  public:
    ObjectList(Object& _parent, std::string_view parentPropertyName) :
      AbstractObjectList{_parent, parentPropertyName}
    {
      static_assert(std::is_base_of_v<Object, T>);
    }

    inline const_iterator begin() const noexcept { return m_items.begin(); }
    inline const_iterator end() const noexcept { return m_items.end(); }
    inline const std::shared_ptr<T>& front() const noexcept { return m_items.front(); }
    inline std::shared_ptr<T>& front() noexcept { return m_items.front(); }
    inline bool empty() const noexcept { return m_items.empty(); }

    ObjectPtr getObject(uint32_t index) final
    {
      assert(index < m_items.size());
      return std::static_pointer_cast<Object>(m_items[index]);
    }

    const std::shared_ptr<T>& operator[](uint32_t index)
    {
      assert(index < m_items.size());
      return m_items[index];
    }

    bool containsObject(const std::shared_ptr<T>& object)
    {
      return std::find(m_items.begin(), m_items.end(), object) != m_items.end();
    }

    void addObject(std::shared_ptr<T> object)
    {
      m_propertyChanged.emplace(object.get(), object->propertyChanged.connect(std::bind(&ObjectList<T>::propertyChanged, this, std::placeholders::_1)));
      m_items.emplace_back(std::move(object));
      objectAdded(m_items.back());
      rowCountChanged();
    }

    void removeObject(const std::shared_ptr<T>& object)
    {
      auto it = std::find(m_items.begin(), m_items.end(), object);
      if(it != m_items.end())
      {
        m_propertyChanged[object.get()].disconnect();
        m_propertyChanged.erase(object.get());
        m_items.erase(it);
        objectRemoved(object);
        rowCountChanged();

        uint32_t row = std::distance(m_items.begin(), it);
        for(auto& model : m_models)
        {
          model->rowRemovedHack(row);
        }
      }
    }
};

#endif
