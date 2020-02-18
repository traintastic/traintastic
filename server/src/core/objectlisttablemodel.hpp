/**
 * server/src/core/objectlisttablemodel.hpp
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

#ifndef SERVER_CORE_OBJECTLISTTABLEMODEL_HPP
#define SERVER_CORE_OBJECTLISTTABLEMODEL_HPP

#include "tablemodel.hpp"
#include "objectlist.hpp"

template<typename T>
class ObjectListTableModel : public TableModel
{
  friend class ObjectList<T>;

  protected:
    ObjectList<T>& m_list;

    const T& getItem(uint32_t row) const { return *m_list.m_items[row]; }
    //T& getItem(uint32_t row) { return *m_list.m_items[row]; }
    virtual void propertyChanged(AbstractProperty& property, uint32_t row) = 0;

  public:
    ObjectListTableModel(ObjectList<T>& list) :
      TableModel(),
      m_list{list}
    {
      m_list.m_models.push_back(this);
      setRowCount(m_list.m_items.size());
    }

    ~ObjectListTableModel() override
    {
      auto it = std::find(m_list.m_models.begin(), m_list.m_models.end(), this);
      assert(it != m_list.m_models.end());
      m_list.m_models.erase(it);
    }
};

#endif
