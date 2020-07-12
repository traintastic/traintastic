/**
 * server/src/core/subobject.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CORE_SUBOBJECT_HPP
#define TRAINTASTIC_SERVER_CORE_SUBOBJECT_HPP

#include "object.hpp"

class SubObject : public Object
{
  private:
    Object& m_parent;
    const std::string& m_parentPropertyName;

  protected:
    void logDebug(const std::string& message);
    void logInfo(const std::string& message);
    void logNotice(const std::string& message);
    void logWarning(const std::string& message);
    void logError(const std::string& message);
    void logCritical(const std::string& message);
    void logFatal(const std::string& message);

  public:
    SubObject(Object& _parent, const std::string& parentPropertyName);

    Object& parent() const { return m_parent; }
    std::string id() const;
};

#endif

