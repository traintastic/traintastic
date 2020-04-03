/**
 * server/src/hardware/decoder/decoderlisttablemodel.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_DECODER_DECODERFUNCTIONLISTTABLEMODEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_DECODER_DECODERFUNCTIONLISTTABLEMODEL_HPP

#include "../../core/objectlisttablemodel.hpp"
#include "decoderfunction.hpp"

class DecoderFunctionList;

class DecoderFunctionListTableModel : public ObjectListTableModel<Hardware::DecoderFunction>
{
  friend class DecoderList;

  protected:
    void propertyChanged(AbstractProperty& property, uint32_t row) final;

  public:
    CLASS_ID("decoder_function_list_table_model")

    static bool isListedProperty(const std::string& name);

    DecoderFunctionListTableModel(DecoderFunctionList& list);

    std::string getText(uint32_t column, uint32_t row) const final;
};

#endif
