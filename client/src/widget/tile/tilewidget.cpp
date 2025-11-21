/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#include "tilewidget.hpp"
#include <QGridLayout>
#include <QTabWidget>
#include "../../network/object.hpp"
#include "../../network/property.hpp"
#include "../../theme/theme.hpp"
#include "../interfaceitemnamelabel.hpp"
#include "../createwidget.hpp"
#include "tileimagewidget.hpp"

TileWidget::TileWidget(ObjectPtr object, QWidget* parent)
  : QWidget(parent)
  , m_object{std::move(object)}
  , m_tabs{new QTabWidget(this)}
  , m_image{new TileImageWidget(this)}
{
  Theme::setWindowIcon(*this, m_object->classId());

  auto* grid = new QGridLayout();
  grid->setContentsMargins(2, 2, 2, 2);

  if(auto* tileId = m_object->getProperty(QStringLiteral("tile_id"))) [[likely]]
  {
    m_image->setTileId(tileId->toEnum<TileId>());
  }
  grid->addWidget(m_image, 0, 0, 2, 1);

  if(auto* name = m_object->getProperty(QStringLiteral("name"))) [[likely]]
  {
    grid->addWidget(new InterfaceItemNameLabel(*name, this), 0, 1);
    grid->addWidget(createWidget(*name, this), 0, 2);
    connect(name, &AbstractProperty::valueChangedString, this, &TileWidget::setWindowTitle);
    setWindowTitle(name->toString());
  }
  if(auto* id = m_object->getProperty(QStringLiteral("id"))) [[likely]]
  {
    grid->addWidget(new InterfaceItemNameLabel(*id, this), 1, 1);
    grid->addWidget(createWidget(*id, this), 1, 2);
  }

  grid->addWidget(m_tabs, 2, 0, 1, 3);

  setLayout(grid);
}
