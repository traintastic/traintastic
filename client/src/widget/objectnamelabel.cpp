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

#include "objectnamelabel.hpp"
#include "../network/connection.hpp"
#include "../network/error.hpp"
#include "../network/object.hpp"
#include "../network/objectproperty.hpp"

ObjectNameLabel::ObjectNameLabel(ObjectProperty& property, QWidget* parent)
  : QLabel(parent)
  , m_property{property}
  , m_requestId{Connection::invalidRequestId}
{
  setVisible(m_property.getAttributeBool(AttributeName::Visible, true));
  connect(&m_property, &ObjectProperty::attributeChanged, this,
    [this](AttributeName name, const QVariant& value)
    {
      switch(name)
      {
        case AttributeName::Visible:
          setVisible(value.toBool());
          break;

        default:
          break;
      }
    });
  connect(&m_property, &ObjectProperty::valueChanged, this, &ObjectNameLabel::objectChanged);
  objectChanged();
}

void ObjectNameLabel::objectChanged()
{
  const auto& connection = m_property.object().connection();

  if(m_requestId != Connection::invalidRequestId)
  {
    connection->cancelRequest(m_requestId);
    m_requestId = Connection::invalidRequestId;
  }

  disconnect(m_valueChanged);
  setText({});
  m_object.reset();

  if(m_property.hasObject())
  {
    m_requestId = m_property.getObject(
      [this](const ObjectPtr& object, std::optional<const Error>)
      {
        if(object)
        {
          m_object = object;
          if(auto* name = m_object->getProperty("name"))
          {
            setText(name->toString());
            m_valueChanged = connect(name, &AbstractProperty::valueChangedString, this, &ObjectNameLabel::setText);
          }
          else if(auto* id = m_object->getProperty("id"))
          {
            setText(id->toString());
            m_valueChanged = connect(id, &AbstractProperty::valueChangedString, this, &ObjectNameLabel::setText);
          }
        }
      });
  }
}
