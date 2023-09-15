/**
 * server/src/hardware/interface/marklincan/marklincanlocomotivelist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#include "marklincanlocomotivelist.hpp"
#include "marklincanlocomotivelisttablemodel.hpp"
#include "../marklincaninterface.hpp"
#include "../../decoder/list/decoderlist.hpp"
#include "../../input/list/inputlist.hpp"
#include "../../output/list/outputlist.hpp"
#include "../../../core/attributes.hpp"
#include "../../../core/method.tpp"
#include "../../../core/objectproperty.tpp"
#include "../../../world/getworld.hpp"
#include "../../../world/world.hpp"

MarklinCANLocomotiveList::MarklinCANLocomotiveList(Object& parent_, std::string_view parentPropertyName)
  : SubObject(parent_, parentPropertyName)
  , importOrSync{*this, "import_or_sync",
      [this](const std::string& name)
      {
        if(!m_data)
          return;

        auto it = std::find_if(m_data->begin(), m_data->end(),
          [&name](auto item)
          {
            return item.name == name;
          });

        if(it != m_data->end())
        {
          import(*it);
        }
      }}
  , importOrSyncAll{*this, "import_or_sync_all",
      [this]()
      {
        if(!m_data)
          return;

        for(const auto& locomotive : *m_data)
        {
          import(locomotive);
        }
      }}
  , reload{*this, "reload",
      [this]()
      {
        if(const auto& kernel = interface().m_kernel)
        {
          kernel->getLocomotiveList();
        }
      }}
{
  Attributes::addEnabled(importOrSync, false);
  m_interfaceItems.add(importOrSync);

  Attributes::addEnabled(importOrSyncAll, false);
  m_interfaceItems.add(importOrSyncAll);

  Attributes::addEnabled(reload, false);
  m_interfaceItems.add(reload);
}

void MarklinCANLocomotiveList::setData(std::shared_ptr<MarklinCAN::LocomotiveList> value)
{
  m_data = std::move(value);

  const uint32_t rowCount = m_data ? static_cast<uint32_t>(m_data->size()) : 0;

  for(auto& model : m_models)
  {
    model->setRowCount(rowCount);
  }
}

TableModelPtr MarklinCANLocomotiveList::getModel()
{
  return std::make_shared<MarklinCANLocomotiveListTableModel>(*this);
}

void MarklinCANLocomotiveList::worldEvent(WorldState state, WorldEvent event)
{
  SubObject::worldEvent(state, event);

  updateEnabled();
}

void MarklinCANLocomotiveList::clear()
{
  m_data.reset();
  for(auto& model : m_models)
  {
    model->setRowCount(0);
  }
  updateEnabled();
}

MarklinCANInterface& MarklinCANLocomotiveList::interface()
{
  assert(dynamic_cast<MarklinCANInterface*>(&parent()));
  return dynamic_cast<MarklinCANInterface&>(parent());
}

void MarklinCANLocomotiveList::import(const MarklinCAN::LocomotiveList::Locomotive& locomotive)
{
  auto& decoders = *interface().decoders;
  std::shared_ptr<Decoder> decoder;

  // 1. try to find by MFX UID:
  if(locomotive.protocol == DecoderProtocol::MFX && locomotive.mfxUID != 0)
  {
    auto it = std::find_if(decoders.begin(), decoders.end(),
      [&locomotive](const auto& item)
      {
        return item->protocol == DecoderProtocol::MFX && item->mfxUID == locomotive.mfxUID;
      });

    if(it != decoders.end())
      decoder = *it;
  }

  // 2. try to find by name:
  if(!decoder)
  {
    auto it = std::find_if(decoders.begin(), decoders.end(),
      [&locomotive](const auto& item)
      {
        return item->name.value() == locomotive.name;
      });

    if(it != decoders.end())
      decoder = *it;
  }

  // 3. try to find by protocol / address (only: motorola or DCC)
  if(!decoder && locomotive.protocol != DecoderProtocol::MFX)
  {
    auto it = std::find_if(decoders.begin(), decoders.end(),
      [&locomotive](const auto& item)
      {
        return item->protocol == locomotive.protocol && item->address == locomotive.address;
      });

    if(it != decoders.end())
      decoder = *it;
  }

  if(!decoder) // not found, create a new one
  {
    decoder = decoders.create();
  }

  // update it:
  decoder->name = locomotive.name;
  decoder->protocol = locomotive.protocol;
  if(decoder->protocol == DecoderProtocol::MFX)
  {
    decoder->address = 0;
    decoder->mfxUID = locomotive.mfxUID;
  }
  else // motorola or DCC
  {
    decoder->address = locomotive.address;
  }

  //! \todo create/update locomotive
}

void MarklinCANLocomotiveList::updateEnabled()
{
  const auto worldState = getWorld(parent()).state.value();
  const bool enabled = m_data && !m_data->empty() && contains(worldState, WorldState::Edit) && !contains(worldState, WorldState::Run);
  Attributes::setEnabled({importOrSync, importOrSyncAll}, enabled);
}
