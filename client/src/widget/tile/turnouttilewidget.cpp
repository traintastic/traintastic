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

#include "turnouttilewidget.hpp"
#include <QTabWidget>
#include <traintastic/locale/locale.hpp>
#include "../../network/error.hpp"
#include "../../network/object.hpp"
#include "../../network/objectproperty.hpp"
#include "../../network/property.hpp"
#include "../../utils/settabwidget.hpp"
#include "../outputmapwidget.hpp"
#include "../interfaceitemnamelabel.hpp"
#include "../createform.hpp"
#include "tileimagewidget.hpp"

TurnoutTileWidget::TurnoutTileWidget(ObjectPtr object, QWidget* parent)
  : TileWidget(std::move(object), parent)
{
  if(auto* position = m_object->getProperty("position")) [[likely]]
  {
    m_image->setTurnoutPosition(position->toEnum<TurnoutPosition>());
    connect(position, &Property::valueChanged, this,
      [this, position]()
      {
        m_image->setTurnoutPosition(position->toEnum<TurnoutPosition>());
      });
  }

  if(auto* outputMap = m_object->getObjectProperty(QStringLiteral("output_map"))) [[likely]]
  {
    const int tabIndex = m_tabs->addTab(new QWidget(this), outputMap->displayName());
    (void)outputMap->getObject(
      [this, tabIndex](const ObjectPtr& obj, std::optional<const Error> /*error*/)
      {
        if(obj) [[likely]]
        {
          setTabWidget(m_tabs, tabIndex, new OutputMapWidget(obj, this));
        }
      });
  }

  if(auto* w = createFormWidget(*m_object, {QStringLiteral("dual_motor")}, this))
  {
    m_tabs->addTab(w, Locale::tr("category:options"));
  }
}
