/**
 * server/src/board/board.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2021 Reinder Feenstra
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

#include "board.hpp"
#include "boardlisttablemodel.hpp"
#include "tile/tiles.hpp"
#include "../world/world.hpp"
#include "../world/worldloader.hpp"
#include "../core/attributes.hpp"
#include "../utils/displayname.hpp"
#include <cassert>

Board::Board(const std::weak_ptr<World>& world, std::string_view _id) :
  IdObject(world, _id),
  name{this, "name", id, PropertyFlags::ReadWrite | PropertyFlags::Store},
  left{this, "left", 0, PropertyFlags::ReadOnly | PropertyFlags::Store},
  top{this, "top", 0, PropertyFlags::ReadOnly | PropertyFlags::Store},
  right{this, "right", 0, PropertyFlags::ReadOnly | PropertyFlags::Store},
  bottom{this, "bottom", 0, PropertyFlags::ReadOnly | PropertyFlags::Store},
  addTile{*this, "add_tile",
    [this](int16_t x, int16_t y, TileRotate rotate, std::string_view classId, bool replace)
    {
      const TileLocation l{x, y};
      auto w = m_world.lock();
      if(!w)
        return false;

      if(auto it = m_tiles.find(l); it != m_tiles.end())
        if(!replace || !deleteTile(x, y))
          return false;

      auto tile = Tiles::create(w, classId);
      if(!tile)
        return false;

      tile->m_location = l;
      tile->setRotate(rotate);

      const int16_t x2 = tile->location().x + tile->data().width();
      const int16_t y2 = tile->location().y + tile->data().height();
      if(tile->location().x < sizeMin || x2 >= sizeMax ||
          tile->location().y < sizeMin || y2 >= sizeMax)
      {
        tile->destroy();
        return false;
      }
      for(int16_t x = tile->location().x; x < x2; x++)
        for(int16_t y = tile->location().y; y < y2; y++)
          m_tiles[TileLocation{x, y}] = tile;

      tileDataChanged(*this, tile->location(), tile->data());
      updateSize();
      return true;
    }},
  moveTile{*this, "moveTile",
    [this](const int16_t xFrom, const int16_t yFrom, const int16_t xTo, const int16_t yTo, const bool replace)
    {
      // check if there is a tile at <From> and it is it's origin
      auto tile = getTile({xFrom, yFrom});
      if(!tile || tile->location().x != xFrom || tile->location().y != yFrom)
        return false;

      const int16_t xFrom2 = xFrom + tile->data().width();
      const int16_t yFrom2 = yFrom + tile->data().height();
      const int16_t xTo2 = xTo + tile->data().width();
      const int16_t yTo2 = yTo + tile->data().height();

      // check if <To> is within board limits
      if(xTo < sizeMin || xTo2 >= sizeMax || yTo < sizeMin || yTo2 >= sizeMax)
        return false;

      // check if <To> is occupied, delete tile(s) if remove is allowed
      for(int16_t x = xTo; x < xTo2; x++)
        for(int16_t y = yTo; y < yTo2; y++)
          if(auto t = getTile({x, y}); t && t != tile)
            if(replace)
              deleteTile(x, y);
            else
              return false;

      // remove tile at tile origin
      removeTile(tile->location().x, tile->location().y);

      // set new origin
      tile->m_location = {xTo, yTo};

      // place tile at <To>
      for(int16_t x = xTo; x < xTo2; x++)
        for(int16_t y = yTo; y < yTo2; y++)
        {
          const TileLocation l{x, y};
          assert(m_tiles.find(l) == m_tiles.end());
          m_tiles[l] = tile;
        }
      tileDataChanged(*this, tile->location(), TileData());

      updateSize();

      return true;
    }},
  deleteTile{*this, "delete_tile",
    [this](int16_t x, int16_t y)
    {
      auto tile = getTile({x, y});
      if(tile)
      {
        removeTile(x, y);
        tile->destroy();
        updateSize();
      }
      return true;
    }},
  resizeToContents{*this, "resize_to_contents",
    [this]()
    {
      updateSize(true);
    }}
{
  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);
  const bool stopped = w && !contains(w->state.value(), WorldState::Run);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);
  m_interfaceItems.add(left);
  m_interfaceItems.add(top);
  m_interfaceItems.add(right);
  m_interfaceItems.add(bottom);
  Attributes::addEnabled(addTile, editable && stopped);
  m_interfaceItems.add(addTile);
  Attributes::addEnabled(moveTile, editable && stopped);
  m_interfaceItems.add(moveTile);
  Attributes::addEnabled(deleteTile, editable && stopped);
  m_interfaceItems.add(deleteTile);
  Attributes::addEnabled(resizeToContents, editable);
  m_interfaceItems.add(resizeToContents);
}

void Board::addToWorld()
{
  IdObject::addToWorld();

  if(auto world = m_world.lock())
    world->boards->addObject(shared_ptr<Board>());
}

void Board::destroying()
{
  for(auto& it : m_tiles)
    it.second->destroy();
  m_tiles.clear();
  if(auto world = m_world.lock())
    world->boards->removeObject(shared_ptr<Board>());
  IdObject::destroying();
}

void Board::load(WorldLoader& loader, const nlohmann::json& data)
{
  IdObject::load(loader, data);

  nlohmann::json objects = data.value("tiles", nlohmann::json::array());
  std::vector<ObjectPtr> items;
  m_tiles.reserve(objects.size());
  for(auto& [_, id] : objects.items())
    if(auto tile = std::dynamic_pointer_cast<Tile>(loader.getObject(id)))
    {
      if(tile->data().width() > 1 || tile->data().height() > 1)
      {
        const int16_t x2 = tile->location().x + tile->data().width();
        const int16_t y2 = tile->location().y + tile->data().height();
        for(int16_t x = tile->location().x; x < x2; x++)
          for(int16_t y = tile->location().y; y < y2; y++)
            m_tiles.emplace(TileLocation{x, y}, tile);
      }
      else
      {
        const TileLocation l = tile->location();
        m_tiles.emplace(l, std::move(tile));
      }
    }
}

void Board::save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const
{
  IdObject::save(saver, data, state);

  nlohmann::json tiles = nlohmann::json::array();
  for(const auto& it : m_tiles)
    if(it.first == it.second->location())
      tiles.push_back(it.second->id);
  data["tiles"] = tiles;
}

void Board::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);
  const bool stopped = !contains(state, WorldState::Run);

  name.setAttributeEnabled(editable);
  addTile.setAttributeEnabled(editable && stopped);
  Attributes::setEnabled(moveTile, editable && stopped);
  deleteTile.setAttributeEnabled(editable && stopped);
  resizeToContents.setAttributeEnabled(editable);
}

void Board::removeTile(int16_t x, int16_t y)
{
  auto tile = getTile({x, y});
  if(!tile)
    return;
  x = tile->location().x;
  y = tile->location().y;
  const int16_t x2 = x + tile->data().width();
  const int16_t y2 = y + tile->data().height();
  for(; x < x2; x++)
    for(; y < y2; y++)
      m_tiles.erase(TileLocation{x, y});
  tileDataChanged(*this, tile->location(), TileData());
}

void Board::updateSize(bool allowShrink)
{
  if(!m_tiles.empty())
  {
    auto it = m_tiles.cbegin();
    int16_t xMin = it->first.x;
    int16_t xMax = it->first.x;
    int16_t yMin = it->first.y;
    int16_t yMax = it->first.y;

    while(++it != m_tiles.cend())
    {
      if(it->first.x < xMin)
        xMin = it->first.x;
      else if(it->first.x > xMax)
        xMax = it->first.x;

      if(it->first.y < yMin)
        yMin = it->first.y;
      else if(it->first.y > yMax)
        yMax = it->first.y;
    }

    xMin = std::clamp(xMin, sizeMin, sizeMax);
    yMin = std::clamp(yMin, sizeMin, sizeMax);
    xMax = std::clamp(xMax, sizeMin, sizeMax);
    yMax = std::clamp(yMax, sizeMin, sizeMax);

    if(!allowShrink)
    {
      xMin = std::min(xMin, left.value());
      yMin = std::min(yMin, top.value());
      xMax = std::max(xMax, right.value());
      yMax = std::max(yMax, bottom.value());
    }

    left.setValueInternal(xMin);
    top.setValueInternal(yMin);
    right.setValueInternal(xMax);
    bottom.setValueInternal(yMax);
  }
  else
  {
    left.setValueInternal(0);
    top.setValueInternal(0);
    right.setValueInternal(0);
    bottom.setValueInternal(0);
  }
}
