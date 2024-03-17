/**
 * client/src/wizard/wizard.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "wizard.hpp"
#include "page/textpage.hpp"
#include "page/propertypage.hpp"

Wizard::Wizard(QWidget* parent)
  : QWizard(parent)
{
}

TextPage* Wizard::addTextPage(const QString& title, const QString& text)
{
  auto* page = new TextPage(this);
  page->setTitle(title);
  page->setText(text);
  addPage(page);
  return page;
}

PropertyPage* Wizard::addPropertyPage(const QString& title, const QString& text, std::initializer_list<AbstractProperty*> properties)
{
  auto* page = new PropertyPage(this);
  page->setTitle(title);
  page->setText(text);
  for(auto* property : properties)
  {
    if(property) /*[[likely]]*/
    {
      page->addProperty(*property);
    }
  }
  addPage(page);
  return page;
}
