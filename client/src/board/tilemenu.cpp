/**
 * client/src/board/tilemenu.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023-2024 Reinder Feenstra
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
#include "../network/callmethod.hpp"
#include "../network/object.hpp"
#include "../network/object/blockrailtile.hpp"
#include "../network/object/trainblockstatus.hpp"
#include "../dialog/objectselectlistdialog.hpp"
#include "../misc/methodaction.hpp"

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
