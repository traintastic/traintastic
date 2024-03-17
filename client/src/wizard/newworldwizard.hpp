/**
 * client/src/wizard/newworldwizard.hpp
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

#ifndef TRAINTASTIC_CLIENT_WIZARD_NEWWORLDWIZARD_HPP
#define TRAINTASTIC_CLIENT_WIZARD_NEWWORLDWIZARD_HPP

#include "wizard.hpp"
#include <traintastic/enum/worldscale.hpp>
#include "../network/objectptr.hpp"

class NewWorldWizard : public Wizard
{
  public:
    NewWorldWizard(ObjectPtr world, QWidget* parent = nullptr);

  private:
    AbstractProperty* m_name;
    AbstractProperty* m_scale;
    QString m_originalName;
    WorldScale m_originalScale;
};

#endif
