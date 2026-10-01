/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2023-2026 Reinder Feenstra
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

#include "tilemenu.hpp"
#include <QMenu>
#include <traintastic/board/tileid.hpp>
#include <traintastic/locale/locale.hpp>
#include "../mainwindow.hpp"
#include "../network/callmethod.hpp"
#include "../network/object.tpp"
#include "../network/object/blockrailtile.hpp"
#include "../network/object/trainblockstatus.hpp"
#include "../dialog/objectselectlistdialog.hpp"
#include "../misc/methodaction.hpp"

namespace {

void addHeader(QMenu& menu, const QString& title)
{
  auto* action = menu.addAction(title);
  action->setEnabled(false);
  auto font = action->font();
  font.setBold(true);
  action->setFont(font);
  menu.addSeparator();
}

void addTrainActions(const ObjectPtr& train, QMenu& menu, QWidget* parent)
{
  (void)train;
  (void)menu;
  (void)parent;
}

void addBlockActions(const std::shared_ptr<BlockRailTile>& block, QMenu& menu, QWidget* parent)
{
  // Train section:
  for(const auto& item : block->trains())
  {
    if(auto* trainBlockStatus = dynamic_cast<TrainBlockStatus*>(item.get())) [[likely]]
    {
      if(const auto& train = trainBlockStatus->train()) [[likely]]
      {
        auto* trainMenu = menu.addMenu(Locale::tr(QString("class_id:").append(train->classId())));
        if(auto name = train->getPropertyValueString("name"); !name.isEmpty())
        {
          addHeader(*trainMenu, name);
        }

        addTrainActions(train, *trainMenu, parent);

        trainMenu->addSeparator();
        if(auto* flipTrain = block->getMethod("flip_train"); flipTrain && flipTrain->getAttributeBool(AttributeName::Enabled, true))
        {
          trainMenu->addAction(new MethodAction(*flipTrain));
        }
        if(auto* removeTrain = block->getMethod("remove_train"))
        {
          trainMenu->addAction(new MethodAction(*removeTrain,
            [removeTrain, train]()
            {
              callMethod(*removeTrain,
                [](std::optional<const Error> error)
                {
                  if(error)
                  {
                    error->show();
                  }
                }, train);
            }));
        }

        trainMenu->addSeparator();
        trainMenu->addAction(Locale::tr("tile_menu:properties"),
          [train]()
          {
            MainWindow::instance->showObject(train, SubWindowType::Object);
          });
      }
    }
  }
  if(auto* assignTrain = block->getMethod("assign_train"); assignTrain && assignTrain->getAttributeBool(AttributeName::Enabled, true))
  {
    menu.addAction(new MethodAction(*assignTrain,
      [parent, assignTrain]()
      {
        std::make_unique<ObjectSelectListDialog>(*assignTrain, false, parent)->exec();
      }));
  }
  menu.addSeparator();
}

void addTurnoutActions(const ObjectPtr& turnout, QMenu& menu, QWidget* parent)
{
  (void)turnout;
  (void)menu;
  (void)parent;
}

}

std::unique_ptr<QMenu> getTileMenu(const ObjectPtr& tile, QWidget* parent)
{
  if(!tile)
  {
    return {};
  }

  const auto tileId = tile->getPropertyValueEnum<TileId>("tile_id", TileId::None);

  if(isActive(tileId))
  {
    auto menu = std::make_unique<QMenu>(parent);

    if(auto name = tile->getPropertyValueString("name"); !name.isEmpty())
    {
      addHeader(*menu, name);
    }
    else if(auto text = tile->getPropertyValueString("text"); !text.isEmpty())
    {
      addHeader(*menu, text);
    }
    else if(auto id = tile->getPropertyValueString("id"); !id.isEmpty())
    {
      addHeader(*menu, id);
    }

    if(tileId == TileId::RailBlock)
    {
      if(auto block = std::dynamic_pointer_cast<BlockRailTile>(tile)) [[likely]]
      {
        addBlockActions(block, *menu, parent);
      }
    }
    else if(isRailTurnout(tileId))
    {
      addTurnoutActions(tile, *menu, parent);
    }

    menu->addSeparator();
    menu->addAction(Locale::tr("tile_menu:properties"),
      [tile]()
      {
        MainWindow::instance->showObject(tile, SubWindowType::Object);
      });

    return menu;
  }
  return {};
}

std::unique_ptr<QMenu> TileMenu::getBlockRailTileMenu(const ObjectPtr& tile, QWidget* parent)
{
  assert(tile->classId() == "board_tile.rail.block");

  auto menu = std::make_unique<QMenu>(parent);

  if(auto* assignTrain = tile->getMethod("assign_train"))
    menu->addAction(new MethodAction(*assignTrain,
      [parent, assignTrain]()
      {
        std::make_unique<ObjectSelectListDialog>(*assignTrain, false, parent)->exec();
      }));
  if(auto* removeTrain = tile->getMethod("remove_train"))
  {
    const auto& block = dynamic_cast<BlockRailTile&>(*tile);
    const auto& trains = block.trains();

    if(trains.size() == 1)
    {
      if(auto* trainBlockStatus = dynamic_cast<TrainBlockStatus*>(trains.front().get())) /*[[likely]]*/
      {
        menu->addAction(new MethodAction(*removeTrain,
          [removeTrain, train=trainBlockStatus->train()]()
          {
            callMethod(*removeTrain,
              [](std::optional<const Error> error)
              {
                if(error)
                {
                  error->show();
                }
              }, train);
          }));
      }
    }
    else if(trains.size() > 1)
    {
      auto* subMenu = menu->addMenu(removeTrain->displayName());

      for(const auto& item : trains)
      {
        if(auto* trainBlockStatus = dynamic_cast<TrainBlockStatus*>(item.get())) /*[[likely]]*/
        {
          subMenu->addAction(trainBlockStatus->train()->getPropertyValueString("name"),
            [removeTrain, train=trainBlockStatus->train()]()
            {
              callMethod(*removeTrain,
                [](std::optional<const Error> error)
                {
                  if(error)
                  {
                    error->show();
                  }
                }, train);
            });
        }
      }
    }
    else
    {
      auto* act = new MethodAction(*removeTrain);
      act->setForceDisabled(true);
      menu->addAction(act);
    }
  }

  if(auto* flipTrain = tile->getMethod("flip_train"))
    menu->addAction(new MethodAction(*flipTrain));

  return menu;
}
