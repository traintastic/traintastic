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

#include "createform.hpp"
#include <QFormLayout>
#include "../network/object.hpp"
#include "createwidget.hpp"
#include "interfaceitemnamelabel.hpp"

QLayout* createFormLayout(Object& object, std::initializer_list<QString> items, QWidget* parent)
{
  auto* form = new QFormLayout();

  for(const auto& name : items)
  {
    if(auto* item = object.getInterfaceItem(name))
    {
      form->addRow(new InterfaceItemNameLabel(*item, parent), createWidget(*item, parent));
    }
  }

  if(!form->isEmpty())
  {
    return form;
  }

  delete form;
  return nullptr;
}

QWidget* createFormWidget(Object& object, std::initializer_list<QString> items, QWidget* parent)
{
  if(auto* form = createFormLayout(object, items, parent))
  {
    auto* w = new QWidget(parent);
    w->setLayout(form);
    return w;
  }
  return nullptr;
}

QLayout* createFormLayout(std::span<InterfaceItem*> items, QWidget* parent)
{
  auto* form = new QFormLayout();

  for(auto* item : items)
  {
    form->addRow(new InterfaceItemNameLabel(*item, parent), createWidget(*item, parent));
  }

  if(!form->isEmpty())
  {
    return form;
  }

  delete form;
  return nullptr;
}

QWidget* createFormWidget(std::span<InterfaceItem*> items, QWidget* parent)
{
  if(auto* form = createFormLayout(items, parent))
  {
    auto* w = new QWidget(parent);
    w->setLayout(form);
    return w;
  }
  return nullptr;
}
