/**
 * client/src/misc/methodaction.cpp
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

#include "methodaction.hpp"
#include "../network/method.hpp"

MethodAction::MethodAction(Method& method, QObject* parent) :
  QAction(parent),
  m_method{method},
  m_forceDisabled{false}
{
  init();
}

MethodAction::MethodAction(Method& method, std::function<void()> triggered, QObject* parent)
  : QAction(parent)
  , m_method{method}
  , m_forceDisabled{false}
{
  init(false);
  connect(this, &QAction::triggered, std::move(triggered));
}

MethodAction::MethodAction(const QIcon &icon, Method& method, QObject* parent) :
  QAction(icon, QString(), parent),
  m_method{method},
  m_forceDisabled{false}
{
  init();
}

MethodAction::MethodAction(const QIcon &icon, Method& method, std::function<void()> triggered, QObject* parent) :
  QAction(icon, QString(), parent),
  m_method{method},
  m_forceDisabled{false}
{
  init(false);
  connect(this, &QAction::triggered, std::move(triggered));
}

bool MethodAction::forceDisabled() const
{
  return m_forceDisabled;
}

void MethodAction::setForceDisabled(bool value)
{
  m_forceDisabled = value;
  setEnabled(!m_forceDisabled && m_method.getAttributeBool(AttributeName::Enabled, true));
}

void MethodAction::init(bool connectTriggeredSignalToMethodIfCompatible)
{
  setText(m_method.displayName());
  setEnabled(m_method.getAttributeBool(AttributeName::Enabled, true));
  setVisible(m_method.getAttributeBool(AttributeName::Visible, true));

  connect(&m_method, &Method::attributeChanged, this,
    [this](AttributeName name, const QVariant& value)
    {
      if(name == AttributeName::Enabled)
        setEnabled(!m_forceDisabled && value.toBool());
      else if(name == AttributeName::Visible)
        setVisible(value.toBool());
      else if(name == AttributeName::DisplayName)
        setText(m_method.displayName());
    });

  // if no return value and no arguments -> setup execute
  if(connectTriggeredSignalToMethodIfCompatible && m_method.resultType() == ValueType::Invalid && m_method.argumentTypes().isEmpty())
    connect(this, &QAction::triggered, this,
      [this]()
      {
        m_method.call();
      });
}
