/**
 * server/src/hardware/decoder/decoderlisttablemodel.cpp
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

#include "decoderlisttablemodel.hpp"
#include "decoderlist.hpp"

using Hardware::Decoder;

constexpr uint32_t columnId = 0;
constexpr uint32_t columnName = 1;
constexpr uint32_t columnAddress = 2;

DecoderListTableModel::DecoderListTableModel(DecoderList& list) :
  ObjectListTableModel<Decoder>(list)
{
  setColumnHeaders({"Id", "Name", "Address"});
}

std::string DecoderListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < rowCount())
  {
    const Decoder& decoder = getItem(row);

    switch(column)
    {
      case columnId:
        return decoder.id;

      case columnName:
        return decoder.name;

      case columnAddress:
        return decoder.address.toString();

      default:
        assert(false);
        break;
    }
  }

  return "";
}
