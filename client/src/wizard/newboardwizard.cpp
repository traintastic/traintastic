/**
 * client/src/wizard/newboardwizard.cpp
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

#include "newboardwizard.hpp"
#include <traintastic/locale/locale.hpp>
#include "../network/object.hpp"
#include "../network/abstractproperty.hpp"
#include "../settings/boardsettings.hpp"

NewBoardWizard::NewBoardWizard(ObjectPtr world, QWidget* parent)
  : Wizard(parent)
  , m_name{world->getProperty("name")}
{
  setWindowTitle(Locale::tr("wizard.new_board:title"));

  if(BoardSettings::instance().showIntroductionWizard)
  {
    addTextPage(
      Locale::tr("wizard.new_board.introduction:title"),
      Locale::tr("wizard.new_board.introduction.long:text"));

    addTextPage(
      Locale::tr("wizard.new_board.editing:title"),
      Locale::tr("wizard.new_board.editing:text"));

    addTextPage(
      Locale::tr("wizard.new_board.adding:title"),
      Locale::tr("wizard.new_board.adding:text"));
  }
  else
  {
    addTextPage(
      Locale::tr("wizard.new_board.introduction:title"),
      Locale::tr("wizard.new_board.introduction.short:text"));
  }

  if(m_name) /*[[likely]]*/
  {
    m_originalName = m_name->toString();

    addPropertyPage(
      Locale::tr("wizard.new_board.name:title"),
      Locale::tr("wizard.new_board.name:text"),
      m_name);
  }

  addTextPage(
    Locale::tr("wizard.new_board.finalization:title"),
    Locale::tr("wizard.new_board.finalization:text"));

  connect(this, &NewBoardWizard::accepted,
    []()
    {
      BoardSettings::instance().showIntroductionWizard = false;
    });

  connect(this, &NewBoardWizard::rejected,
    [this]()
    {
      if(m_name) /*[[likely]]*/
      {
        m_name->setValueString(m_originalName);
      }
    });
}
