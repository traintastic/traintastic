/**
 * client/src/misc/methodaction.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_MISC_METHODACTION_HPP
#define TRAINTASTIC_CLIENT_MISC_METHODACTION_HPP

#include <QAction>

class Method;

/**
 * @brief Action wrapper for method's
 *
 * The action's text is set to the method's display name.
 * The method's displayName, enabled and visible attributes are monitored for changes and applied to the action.
 *
 * If the method has no return value and no arguments it's will call the method when triggered.
 * For other method type this has to be done externally.
 */
class MethodAction : public QAction
{
  private:
    Method& m_method;
    bool m_forceDisabled;

    void init(bool connectTriggeredSignalToMethodIfCompatible = true);

  public:
    MethodAction(Method& method, QObject* parent = nullptr);
    MethodAction(Method& method, std::function<void()> triggered, QObject* parent = nullptr);
    MethodAction(const QIcon &icon, Method& method, QObject* parent = nullptr);
    MethodAction(const QIcon &icon, Method& method, std::function<void()> triggered, QObject* parent = nullptr);

    const Method& method() const { return m_method; }
    Method& method() { return m_method; }

    bool forceDisabled() const;
    void setForceDisabled(bool value);
};

#endif
