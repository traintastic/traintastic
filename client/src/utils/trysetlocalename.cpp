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

#include "trysetlocalename.hpp"
#include <traintastic/locale/locale.hpp>
#include "../network/abstractproperty.hpp"
#include "../network/object.hpp"

void trySetLocaleName(Object& object)
{
  if(auto* name = object.getProperty("name"))
  {
    if(auto* id = object.getProperty("id"); id && id->toString() == name->toString())
    {
      auto value = id->toString();
      if(int pos = value.lastIndexOf("_"); pos >= 0)
      {
        auto prefix = value.first(pos);
        auto number = value.last(value.size() - pos - 1);
        auto term = QString("default_name:").append(prefix);
        if(Locale::instance->exists(term))
        {
          name->setValueString(QString("%1 %2").arg(Locale::tr(term)).arg(number));
        }
      }
    }
  }
}
