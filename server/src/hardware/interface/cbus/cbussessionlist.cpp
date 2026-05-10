/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "cbussessionlist.hpp"
#include "cbussessionlisttablemodel.hpp"

CBUSSessionList::CBUSSessionList(Object& parent_, std::string_view parentPropertyName)
  : SubObject(parent_, parentPropertyName)
{
}

TableModelPtr CBUSSessionList::getModel()
{
  return std::make_shared<CBUSSessionListTableModel>(*this);
}

CBUSSessionList::iterator CBUSSessionList::findEngine(uint16_t address, bool isLongAddress)
{
  return std::find_if(m_sessions.begin(), m_sessions.end(),
    [address, isLongAddress](const auto& item)
    {
      return item.address == address && item.isLongAddress == isLongAddress;
    });
}

CBUSSessionList::iterator CBUSSessionList::findSession(uint8_t session)
{
  return std::find_if(m_sessions.begin(), m_sessions.end(),
    [session](const auto& item)
    {
      return item.session && *item.session == session;
    });
}

void CBUSSessionList::add(Session&& session)
{
  m_sessions.emplace_back(std::move(session));

  const auto rowCount = static_cast<uint32_t>(m_sessions.size());
  for(auto& model : m_models)
  {
    model->setRowCount(rowCount);
  }
}

void CBUSSessionList::clear()
{
  m_sessions.clear();
  for(auto& model : m_models)
  {
    model->setRowCount(0);
  }
}

void CBUSSessionList::rowChanged(uint32_t row)
{
  for(auto& model : m_models)
  {
    model->rowsChanged(row, row);
  }
}
