/**
 * client/src/widget/propertylineedit.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2023 Reinder Feenstra
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

#include "propertylineedit.hpp"
#include <QToolTip>
#include "../network/property.hpp"
#include "../network/object.hpp"
#include "../network/connection.hpp"
#include "../network/error.hpp"
#include <traintastic/locale/locale.hpp>

PropertyLineEdit::PropertyLineEdit(Property& property, QWidget* parent) :
  QLineEdit(parent),
  m_property{property},
  m_requestId{Connection::invalidRequestId}
{
  Q_ASSERT(m_property.type() == ValueType::String);
  setEnabled(m_property.getAttributeBool(AttributeName::Enabled, true));
  setVisible(m_property.getAttributeBool(AttributeName::Visible, true));
  setText(m_property.toString());
  connect(&m_property, &Property::valueChangedString, this,
    [this](const QString& value)
    {
      if(!hasFocus())
        setText(value);
    });
  connect(&m_property, &Property::attributeChanged, this,
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
  connect(this, &PropertyLineEdit::textEdited, this,
    [this](const QString& value)
    {
      cancelRequest();
      m_requestId = m_property.setValueString(value,
        [this](std::optional<const Error> error)
        {
          if(error)
            showError(error->toString());
        });
    });
}

PropertyLineEdit::~PropertyLineEdit()
{
  cancelRequest();
}

void PropertyLineEdit::cancelRequest()
{
  if(m_requestId == Connection::invalidRequestId)
    return;
  m_property.object().connection()->cancelRequest(m_requestId);
  m_requestId = Connection::invalidRequestId;
}

void PropertyLineEdit::showError(const QString& error)
{
  // TODO: replace by a proper error tip
  if(!error.isEmpty())
    QToolTip::showText(mapToGlobal(rect().bottomLeft()), Locale::tr(error));
  else
    QToolTip::hideText();
}

void PropertyLineEdit::focusOutEvent(QFocusEvent* event)
{
  QLineEdit::focusOutEvent(event);
  setText(m_property.toString());
}
