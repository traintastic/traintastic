/**
 * client/src/widget/methodpushbutton.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2023-2024 Reinder Feenstra
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

#include "methodpushbutton.hpp"
#include "../network/connection.hpp"
#include "../network/method.hpp"
#include "../network/error.hpp"
#include "../mainwindow.hpp"

MethodPushButton::MethodPushButton(Method& method, QWidget* parent) :
  QPushButton(parent),
  m_method{method},
  m_requestId{Connection::invalidRequestId}
{
  setEnabled(m_method.getAttributeBool(AttributeName::Enabled, true));
  setVisible(m_method.getAttributeBool(AttributeName::Visible, true));
  connect(&m_method, &Method::attributeChanged, this,
    [this](AttributeName name, const QVariant& value)
    {
      switch(name)
      {
        case AttributeName::Enabled:
          setEnabled(value.toBool());
          break;

        case AttributeName::Visible:
          setVisible(value.toBool());
          break;

        default:
          break;
      }
    });
  connect(this, &MethodPushButton::clicked,
    [this]()
    {
      if(m_method.argumentTypes().count() == 0)
        switch(m_method.resultType())
        {
          case ValueType::Invalid:
            m_method.call();
            break;

          case ValueType::Object:
            m_requestId = m_method.call(
              [](const ObjectPtr& object, std::optional<const Error> /*error*/)
              {
                if(object)
                  MainWindow::instance->showObject(object);
              });
            break;

           default:
             Q_ASSERT(false);
            break;
        }
    });
}
