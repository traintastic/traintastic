/**
 * server/src/core/handlelist.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CORE_HANDLELIST_HPP
#define TRAINTASTIC_SERVER_CORE_HANDLELIST_HPP

#include <cstdint>
#include <cassert>
#include <unordered_map>

template<typename Thandle, typename Titem>
class HandleList
{
  public:
    using Handle = Thandle;

  protected:
    Handle m_nextHandle;
    std::unordered_map<Handle, Titem> m_handleToItem;
    std::unordered_map<Titem, Handle> m_itemToHandle;
    std::unordered_map<Handle, uint32_t> m_handleCounter;

  public:
    static const Handle invalidHandle = 0;

    HandleList() :
      m_nextHandle{invalidHandle + 1}
    {
    }

    Titem getItem(Handle handle) const
    {
      auto it = m_handleToItem.find(handle);
      return it != m_handleToItem.end() ? it->second : nullptr;
    }

    uint32_t getCounter(Handle handle)
    {
      auto it = m_handleCounter.find(handle);
      return it != m_handleCounter.end() ? it->second : 0;
    }

    Handle addItem(const Titem& item)
    {
      if(!item)
        return invalidHandle;

      auto it = m_itemToHandle.find(item);
      if(it == m_itemToHandle.end())
      {
        Handle handle = m_nextHandle;
        m_handleToItem.emplace(handle, item);
        m_handleCounter.emplace(handle, 1);
        m_itemToHandle.emplace(item, handle);
        assert(m_handleToItem.size() == m_itemToHandle.size());
        if(++m_nextHandle == invalidHandle)
          m_nextHandle++;
        return handle;
      }
      else
      {
        m_handleCounter[it->second]++;
        return it->second;
      }
    }

    Handle getHandle(const Titem& item)
    {
      if(item)
      {
        auto it = m_itemToHandle.find(item);
        if(it != m_itemToHandle.end())
        {
          m_handleCounter[it->second]++;
          return it->second;
        }
      }
      return invalidHandle;
    }

    void removeHandle(const Thandle& handle)
    {
      auto it = m_handleToItem.find(handle);
      if(it != m_handleToItem.end())
      {
        m_itemToHandle.erase(it->second);
        m_handleToItem.erase(it);
        m_handleCounter.erase(handle);
        assert(m_handleToItem.size() == m_itemToHandle.size());
      }
    }

    void removeItem(const Titem& item)
    {
      auto it = m_itemToHandle.find(item);
      if(it != m_itemToHandle.end())
      {
        m_handleToItem.erase(it->second);
        m_handleCounter.erase(it->second);
        m_itemToHandle.erase(it);
        assert(m_handleToItem.size() == m_itemToHandle.size());
      }
    }

    void clear()
    {
      m_handleToItem.clear();
      m_itemToHandle.clear();
      assert(m_handleToItem.size() == m_itemToHandle.size());
    }
};

#endif
