/**
 * server/src/core/tablemodel.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_TABLEMODEL_HPP
#define TRAINTASTIC_SERVER_CORE_TABLEMODEL_HPP

#include "object.hpp"
#include <functional>
#include "tablemodelptr.hpp"

class TableModel : public Object
{
  public:
    struct Region
    {
      uint32_t columnMin;
      uint32_t columnMax;
      uint32_t rowMin;
      uint32_t rowMax;


      Region() :
        columnMin{1},
        columnMax{0},
        rowMin{1},
        rowMax{0}
      {
      }

      Region(uint32_t _columnMin, uint32_t _columnMax, uint32_t _rowMin, uint32_t _rowMax) :
        columnMin{_columnMin},
        columnMax{_columnMax},
        rowMin{_rowMin},
        rowMax{_rowMax}
      {
      }

      bool operator==(const Region& rhs) const
      {
        return
          this->columnMin == rhs.columnMin &&
          this->columnMax == rhs.columnMax &&
          this->rowMin == rhs.rowMin &&
          this->rowMax == rhs.rowMax;
      }

      bool operator!=(const Region& rhs) const
      {
        return !(*this == rhs);
      }

      bool operator<=(const Region& rhs) const
      {
        return
          this->columnMin >= rhs.columnMin &&
          this->columnMax <= rhs.columnMax &&
          this->rowMin >= rhs.rowMin &&
          this->rowMax <= rhs.rowMax;
      }

      bool isValid() const
      {
        return columnMax >= columnMin && rowMax >= rowMin;
      }
    };

  private:
    std::vector<std::string_view> m_columnHeaders;
    uint32_t m_rowCount;
    Region m_region;

  protected:
    void setColumnHeaders(std::vector<std::string_view> values);
    void setRowCount(uint32_t value);

    void changed(uint32_t row, uint32_t column);

  public:
    std::function<void(const TableModelPtr&)> columnHeadersChanged;
    std::function<void(const TableModelPtr&)> rowCountChanged;
    std::function<void(const TableModelPtr&, const Region& region)> updateRegion;

    TableModel();

    std::string getObjectId() const final { assert(false); return {}; }

    const std::vector<std::string_view>& columnHeaders() const { return m_columnHeaders; }
    uint32_t columnCount() const { return static_cast<uint32_t>(m_columnHeaders.size()); }
    uint32_t rowCount() const { return m_rowCount; }

    virtual std::string getText(uint32_t column, uint32_t row) const = 0;

    void setRegion(const Region& value);

    void rowsChanged(uint32_t first, uint32_t last);
    void rowRemovedHack(uint32_t row);
};

#endif
