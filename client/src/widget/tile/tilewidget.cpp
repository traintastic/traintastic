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
#include <traintastic/locale/locale.hpp>
#include "../../network/error.hpp"
#include "../../network/object.hpp"
#include "../../network/objectproperty.hpp"
#include "../../network/property.hpp"
#include "../../theme/theme.hpp"
#include "../../utils/settabwidget.hpp"
#include "../interfaceitemnamelabel.hpp"
#include "../createform.hpp"
#include "../createwidget.hpp"
#include "tileimagewidget.hpp"

TileWidget::TileWidget(ObjectPtr object, QWidget* parent)
  : QWidget(parent)
  , m_object{std::move(object)}
  , m_tabs{new QTabWidget(this)}
  , m_image{new TileImageWidget(m_object, this)}
{
  Theme::setWindowIcon(*this, m_object->classId());

  auto* grid = new QGridLayout();
  grid->setContentsMargins(2, 2, 2, 2);
  grid->addWidget(m_image, 0, 0);

  // Window title:
  if(auto* name = m_object->getProperty(QStringLiteral("name")))
  {
    connect(name, &AbstractProperty::valueChangedString, this, &TileWidget::setWindowTitle);
    setWindowTitle(name->toString());
  }
  else if(auto* id = m_object->getProperty(QStringLiteral("id")))
  {
    connect(id, &AbstractProperty::valueChangedString, this, &TileWidget::setWindowTitle);
    setWindowTitle(id->toString());
  }

  // Properties:
  for(const QString& category : m_object->interfaceItems().categories())
  {
    auto items = m_object->interfaceItems().items(category);
    items.erase(std::remove_if(items.begin(), items.end(),
      [](auto* item)
      {
        return !item->getAttributeBool(AttributeName::ObjectEditor, true) || !dynamic_cast<AbstractProperty*>(item);
      }), items.end());

    if(items.empty())
    {
      continue;
    }

    // sub object to own tab:
    for(auto it = items.begin(); it < items.end();)
    {
      if(auto* property = dynamic_cast<ObjectProperty*>(*it); property && contains(property->flags(), PropertyFlags::SubObject))
      {
        const auto tabIndex = m_tabs->addTab(new QWidget(this), property->displayName());
        (void)property->getObject(
          [this, tabIndex](const ObjectPtr& obj, std::optional<const Error> /*error*/)
          {
            if(obj) [[likely]]
            {
              setTabWidget(m_tabs, tabIndex, createWidget(obj, this));
            }
          });
        it = items.erase(it);
      }
      else
      {
        it++;
      }
    }

    if(QWidget* w = createFormWidget(items, this))
    {
      if(category.isEmpty())
      {
        w->layout()->setContentsMargins(2, 2, 2, 2);
        grid->addWidget(w, 0, 1);
      }
      else
      {
        m_tabs->addTab(w, Locale::tr(category));
      }
    }
  }

  if(m_tabs->count() > 0)
  {
    grid->addWidget(m_tabs, 1, 0, 1, 2);
  }
  else
  {
    delete m_tabs;
    m_tabs = nullptr;
  }

  setLayout(grid);
}
