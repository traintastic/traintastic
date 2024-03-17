/**
 * client/src/wizard/newworldwizard.cpp
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

#include "newworldwizard.hpp"
#include <traintastic/locale/locale.hpp>
#include "../network/object.hpp"
#include "../network/abstractproperty.hpp"

NewWorldWizard::NewWorldWizard(ObjectPtr world, QWidget* parent)
  : Wizard(parent)
  , m_name{world->getProperty("name")}
  , m_scale{world->getProperty("scale")}
{
  setWindowTitle(Locale::tr("wizard.new_world:title"));

  addTextPage(
    Locale::tr("wizard.new_world.introduction:title"),
    Locale::tr("wizard.new_world.introduction:text"));

  if(m_name) /*[[likely]]*/
  {
    m_originalName = m_name->toString();

    addPropertyPage(
      Locale::tr("wizard.new_world.name:title"),
      Locale::tr("wizard.new_world.name:text"),
      m_name);
  }

  if(m_scale) /*[[likely]]*/
  {
    m_originalScale = m_scale->toEnum<WorldScale>();

    addPropertyPage(
      Locale::tr("wizard.new_world.scale:title"),
      Locale::tr("wizard.new_world.scale:text"),
      {m_scale, world->getProperty("scale_ratio")});
  }

  addTextPage(
    Locale::tr("wizard.new_world.finalization:title"),
    Locale::tr("wizard.new_world.finalization:text"));

  connect(this, &NewWorldWizard::rejected,
    [this]()
    {
      if(m_name) /*[[likely]]*/
      {
        m_name->setValueString(m_originalName);
      }
      if(m_scale) /*[[likely]]*/
      {
        m_scale->setValueEnum(m_originalScale);
      }
    });
}
