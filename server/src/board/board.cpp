/**
 * server/src/board/board.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2022 Reinder Feenstra
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
#include "boardlist.hpp"
#include "boardlisttablemodel.hpp"
#include "map/link.hpp"
#include "tile/tiles.hpp"
#include "../core/objectproperty.tpp"
#include "../world/world.hpp"
#include "../world/worldloader.hpp"
#include "../core/attributes.hpp"
#include "../utils/displayname.hpp"
#include <cassert>

Board::Board(World& world, std::string_view _id) :
  IdObject(world, _id),
  name{this, "name", id, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly},
  left{this, "left", 0, PropertyFlags::ReadOnly | PropertyFlags::Store},
  top{this, "top", 0, PropertyFlags::ReadOnly | PropertyFlags::Store},
  right{this, "right", 0, PropertyFlags::ReadOnly | PropertyFlags::Store},
  bottom{this, "bottom", 0, PropertyFlags::ReadOnly | PropertyFlags::Store},
  addTile{*this, "add_tile",
    [this](int16_t x, int16_t y, TileRotate rotate, std::string_view tileClassId, bool replace)
    {
      const TileLocation l{x, y};

      if(auto it = m_tiles.find(l); it != m_tiles.end())
      {
        if(!replace)
        {
          const TileRotate tileRotate = it->second->rotate;

          if(it->second->tileId() == TileId::RailStraight && tileClassId == StraightRailTile::classId) // merge to bridge
          {
            if((tileRotate == rotate + TileRotate::Deg90 || tileRotate == rotate - TileRotate::Deg90) && deleteTile(x, y))
            {
              tileClassId = Bridge90RailTile::classId;
              rotate = tileRotate;
            }
            else if((tileRotate == rotate + TileRotate::Deg45 || tileRotate == rotate + TileRotate::Deg225) && deleteTile(x, y))
            {
              tileClassId = Bridge45LeftRailTile::classId;
              rotate = tileRotate;
            }
            else if((tileRotate == rotate - TileRotate::Deg45 || tileRotate == rotate - TileRotate::Deg225) && deleteTile(x, y))
            {
              tileClassId = Bridge45RightRailTile::classId;
              rotate = tileRotate;
            }
            else
              return false;
          }
          else if(it->second->tileId() == TileId::RailStraight && // replace straight by a straight with something extra
                  Tiles::canUpgradeStraightRail(tileClassId) &&
                  (tileRotate == rotate || (tileRotate + TileRotate::Deg180) == rotate) &&
                  deleteTile(x, y))
          {
            // tileClassId and rotate are ok :)
          }
          else
            return false;
        }
        else if(!replace || !deleteTile(x, y))
          return false;
      }

      auto tile = Tiles::create(m_world, tileClassId);
      if(!tile)
        return false;

      tile->x.setValueInternal(l.x);
      tile->y.setValueInternal(l.y);
      tile->setRotate(rotate);

      const int16_t x2 = tile->location().x + tile->width;
      const int16_t y2 = tile->location().y + tile->height;
      if(tile->location().x < sizeMin || x2 >= sizeMax ||
          tile->location().y < sizeMin || y2 >= sizeMax)
      {
        tile->destroy();
        return false;
      }
      for(int16_t xx = tile->location().x; xx < x2; xx++)
        for(int16_t yy = tile->location().y; yy < y2; yy++)
          m_tiles[TileLocation{xx, yy}] = tile;

      tileDataChanged(*this, tile->location(), tile->data());
      updateSize();
      m_modified = true;
      return true;
    }},
  moveTile{*this, "move_tile",
    [this](int16_t xFrom, int16_t yFrom, int16_t xTo, int16_t yTo, TileRotate rotate, const bool replace)
    {
      // check if there is a tile at <From>
      auto tile = getTile({xFrom, yFrom});
      if(!tile)
        return false;

      // correct coordinates, so <to> is tile origin
      {
        const int16_t xDiff = xFrom - tile->location().x;
        xFrom -= xDiff;
        xTo -= xDiff;
        const int16_t yDiff = yFrom - tile->location().y;
        yFrom -= yDiff;
        yTo -= yDiff;
      }

      // determine new width and height
      uint8_t width = tile->width;
      uint8_t height = tile->height;
      if(tile->rotate != rotate && width != height)
      {
        if(isDiagonal(rotate))
          return false;

        if(diff(tile->rotate, rotate) == TileRotate::Deg90)
          std::swap(width, height);
      }

      const int16_t xTo2 = xTo + width;
      const int16_t yTo2 = yTo + height;

      // check if <To> is within board limits
      if(xTo < sizeMin || xTo2 >= sizeMax || yTo < sizeMin || yTo2 >= sizeMax)
        return false;

      // check if <To> is occupied, delete tile(s) if remove is allowed
      for(int16_t x = xTo; x < xTo2; x++)
        for(int16_t y = yTo; y < yTo2; y++)
          if(auto t = getTile({x, y}); t && t != tile)
          {
            if(replace)
              deleteTile(x, y);
            else
              return false;
          }

      // remove tile at tile origin
      removeTile(tile->location().x, tile->location().y);

      // set new params
      tile->x.setValueInternal(xTo);
      tile->y.setValueInternal(yTo);
      tile->height.setValueInternal(height);
      tile->width.setValueInternal(width);
      tile->rotate.setValueInternal(rotate);

      // place tile at <To>
      for(int16_t x = xTo; x < xTo2; x++)
        for(int16_t y = yTo; y < yTo2; y++)
        {
          const TileLocation l{x, y};
          assert(m_tiles.find(l) == m_tiles.end());
          m_tiles[l] = tile;
        }
      tileDataChanged(*this, tile->location(), tile->data());

      updateSize();
      m_modified = true;
      return true;
    }},
  resizeTile{*this, "resize_tile",
    [this](const int16_t x, const int16_t y, const uint8_t width, const uint8_t height)
    {
      // check for illigal size
      if(width == 0 || height == 0)
        return false;

      // check if there is a tile at <x, y> and it is it's origin
      auto tile = getTile({x, y});
      if(!tile || tile->location().x != x || tile->location().y != y)
        return false;

      const uint8_t oldWidth = tile->width;
      const uint8_t oldHeight = tile->height;

      // check if space is available (if growing)
      if(width > oldWidth || height > oldHeight)
      {
        const int16_t x2 = x + width;
        const int16_t y2 = y + height;
        for(int16_t xx = x; xx < x2; xx++)
          for(int16_t yy = y; yy < y2; yy++)
            if(auto t = getTile({xx, yy}); t && t != tile)
              return false;
      }

      // check if new tile size is valid
      if(!tile->resize(width, height))
        return false;

      // update m_tiles
      {
        const int16_t x2 = x + std::max(width, oldWidth);
        const int16_t y2 = y + std::max(height, oldHeight);
        const int16_t xNew = x + width;
        const int16_t yNew = y + height;

        for(int16_t xx = x; xx < x2; xx++)
          for(int16_t yy = y; yy < y2; yy++)
            if(xx < xNew && yy < yNew)
              m_tiles[{xx, yy}] = tile;
            else
              m_tiles.erase({xx, yy});
      }

      tileDataChanged(*this, tile->location(), tile->data());
      m_modified = true;
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
        m_modified = true;
      }
      return true;
    }},
  resizeToContents{*this, "resize_to_contents",
    [this]()
    {
      updateSize(true);
    }}
{
  const bool editable = contains(world.state.value(), WorldState::Edit);
  const bool stopped = !contains(world.state.value(), WorldState::Run);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);
  Attributes::addObjectEditor(left, false);
  m_interfaceItems.add(left);
  Attributes::addObjectEditor(top, false);
  m_interfaceItems.add(top);
  Attributes::addObjectEditor(right, false);
  m_interfaceItems.add(right);
  Attributes::addObjectEditor(bottom, false);
  m_interfaceItems.add(bottom);
  Attributes::addEnabled(addTile, editable && stopped);
  Attributes::addObjectEditor(addTile, false);
  m_interfaceItems.add(addTile);
  Attributes::addEnabled(moveTile, editable && stopped);
  Attributes::addObjectEditor(moveTile, false);
  m_interfaceItems.add(moveTile);
  Attributes::addEnabled(resizeTile, editable && stopped);
  Attributes::addObjectEditor(resizeTile, false);
  m_interfaceItems.add(resizeTile);
  Attributes::addEnabled(deleteTile, editable && stopped);
  Attributes::addObjectEditor(deleteTile, false);
  m_interfaceItems.add(deleteTile);
  Attributes::addEnabled(resizeToContents, editable);
  Attributes::addObjectEditor(resizeToContents, false);
  m_interfaceItems.add(resizeToContents);
}

void Board::addToWorld()
{
  IdObject::addToWorld();

  m_world.boards->addObject(shared_ptr<Board>());
}

void Board::destroying()
{
  for(auto& it : m_tiles)
    if(it.first == it.second->location()) // only on origin
      it.second->destroy();
  m_tiles.clear();
  m_world.boards->removeObject(shared_ptr<Board>());
  IdObject::destroying();
}

void Board::load(WorldLoader& loader, const nlohmann::json& data)
{
  IdObject::load(loader, data);

  nlohmann::json objects = data.value("tiles", nlohmann::json::array());
  std::vector<ObjectPtr> items;
  m_tiles.reserve(objects.size());
  for(auto& [_, tileId] : objects.items())
  {
    static_cast<void>(_); // silence unused warning
    if(auto tile = std::dynamic_pointer_cast<Tile>(loader.getObject(tileId.get<std::string_view>())))
    {
      if(tile->width > 1 || tile->height > 1)
      {
        const int16_t x2 = tile->location().x + tile->width;
        const int16_t y2 = tile->location().y + tile->height;
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
}

void Board::save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const
{
  IdObject::save(saver, data, state);

  nlohmann::json tiles = nlohmann::json::array();
  for(const auto& it : m_tiles)
    if(it.first == it.second->location())
      tiles.push_back(it.second->id);
  std::sort(tiles.begin(), tiles.end(),
    [](const nlohmann::json& a, const nlohmann::json& b)
    {
      return (a < b);
    });
  data["tiles"] = tiles;
}

void Board::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);
  const bool stopped = !contains(state, WorldState::Run);

  Attributes::setEnabled(name, editable);
  Attributes::setEnabled(addTile, editable && stopped);
  Attributes::setEnabled(moveTile, editable && stopped);
  Attributes::setEnabled(resizeTile, editable && stopped);
  Attributes::setEnabled(deleteTile, editable && stopped);
  Attributes::setEnabled(resizeToContents, editable);

  if(event == WorldEvent::EditDisabled || event == WorldEvent::Run)
  {
    modified();
  }
}

void Board::loaded()
{
  IdObject::loaded();

  m_modified = true;
  modified();
}

void Board::modified()
{
  if(!m_modified)
    return;

  auto updateLink =
    [this](const std::shared_ptr<Tile>& startTile, const Connector& startConnector)
    {
      assert(startTile->node());

      std::vector<std::shared_ptr<Tile>> tiles;
      std::vector<Connector> connectors;
      connectors.reserve(2);

      Connector connector{startConnector.opposite()};
      while(auto nextTile = getTile(connector.location))
      {
        if(nextTile->node())
        {
          auto link = std::make_shared<Link>(std::move(tiles));
          link->connect(*startTile->node(), startConnector, *nextTile->node(), connector);
          return;
        }
        tiles.emplace_back(nextTile);
        connectors.clear();
        nextTile->getConnectors(connectors);
        if(connectors.size() == 2)
        {
          assert(connectors[0] == connector || connectors[1] == connector);
          connector = connectors[connectors[0] == connector ? 1 : 0].opposite();
        }
        else
        {
          assert(connectors.size() == 1);
          assert(connectors[0] == connector);
          break;
        }
      }

      startTile->node()->get().disconnect(startConnector);
    };

  // check/rebuild links:
  {
    std::vector<Connector> connectors;

    for(auto& [l, tile] : m_tiles)
    {
      if(auto node = tile->node(); node && l == tile->location())
      {
        connectors.clear();

        tile->getConnectors(connectors);

        assert(!connectors.empty());
        for(const auto connector : connectors)
        {
          updateLink(tile, connector);
        }
      }
    }
  }

  // notify board changed:
  for(auto& [l, tile] : m_tiles)
    if(l == tile->location()) // check origin to notify each tile once
      tile->boardModified();

  m_modified = false;
}

void Board::removeTile(const int16_t x, const int16_t y)
{
  auto tile = getTile({x, y});
  if(!tile)
    return;
  const auto l = tile->location();
  const int16_t x2 = l.x + tile->width;
  const int16_t y2 = l.y + tile->height;
  for(int16_t xx = l.x; xx < x2; xx++)
    for(int16_t yy = l.y; yy < y2; yy++)
      m_tiles.erase(TileLocation{xx, yy});
  tileDataChanged(*this, l, TileData());
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
