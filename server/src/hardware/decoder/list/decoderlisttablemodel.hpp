/**
 * server/src/hardware/decoder/list/decoderlisttablemodel.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_DECODER_LIST_DECODERLISTTABLEMODEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_DECODER_LIST_DECODERLISTTABLEMODEL_HPP

#include "../../../core/objectlisttablemodel.hpp"
#include "decoderlistcolumn.hpp"
#include "../decoder.hpp"

class DecoderList;

class DecoderListTableModel : public ObjectListTableModel<Decoder>
{
  friend class DecoderList;

  private:
    std::vector<DecoderListColumn> m_columns;

    void changed(uint32_t row, DecoderListColumn column);

  protected:
    void propertyChanged(BaseProperty& property, uint32_t row) final;

  public:
    CLASS_ID("decoder_list_table_model")

    static bool isListedProperty(std::string_view name);

    DecoderListTableModel(DecoderList& commandStationList);

    std::string getText(uint32_t column, uint32_t row) const final;
};

#endif
