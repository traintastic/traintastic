/**
 * client/src/widget/propertyobjectedit.cpp
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

#include "propertyobjectedit.hpp"
#include <memory>
#include "../network/connection.hpp"
#include "../network/objectproperty.hpp"
#include "../network/object.hpp"
#include "../dialog/objectselectlistdialog.hpp"
#include "../mainwindow.hpp"
#include "../theme/theme.hpp"
#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>

PropertyObjectEdit::PropertyObjectEdit(ObjectProperty& property, QWidget *parent) :
  QWidget(parent),
  m_property{property},
  m_lineEdit{new QLineEdit(m_property.objectId(), this)},
  m_changeButton{m_property.isWritable() && m_property.hasAttribute(AttributeName::ObjectList) ? new QToolButton(this) : nullptr},
  m_editButton{new QToolButton(this)}
  , m_editObjectRequestId{Connection::invalidRequestId}
{
  bool enabled = m_property.getAttributeBool(AttributeName::Enabled, true);
  bool visible = m_property.getAttributeBool(AttributeName::Visible, true);
  connect(&m_property, &ObjectProperty::attributeChanged, this,
    [this](AttributeName name, const QVariant& value)
    {
      switch(name)
      {
        case AttributeName::Enabled:
          m_lineEdit->setEnabled(value.toBool());
          if(m_changeButton)
            m_changeButton->setEnabled(value.toBool());
          break;

        case AttributeName::Visible:
          m_lineEdit->setVisible(value.toBool());
          if(m_changeButton)
            m_changeButton->setVisible(value.toBool());
          break;

        default:
          break;
      }
    });
  connect(&m_property, &ObjectProperty::valueChanged, this,
    [this]()
    {
      m_editButton->setEnabled(m_property.hasObject());
      m_lineEdit->setText(m_property.objectId());
    });

  QHBoxLayout* l = new QHBoxLayout();
  l->setContentsMargins(0, 0, 0, 0);

  m_lineEdit->setEnabled(enabled);
  m_lineEdit->setVisible(visible);
  m_lineEdit->setReadOnly(true);
  l->addWidget(m_lineEdit, 1);

  if(m_changeButton)
  {
    m_changeButton->setEnabled(enabled);
    m_changeButton->setVisible(visible);
    m_changeButton->setText("...");
    connect(m_changeButton, &QToolButton::clicked, this,
      [this]()
      {
        std::make_unique<ObjectSelectListDialog>(m_property, this)->exec();
      });
    l->addWidget(m_changeButton);
  }

  m_editButton->setIcon(Theme::getIcon("edit"));
  m_editButton->setEnabled(m_property.hasObject());
  connect(m_editButton, &QToolButton::clicked, this,
    [this]()
    {
      if(!m_property.hasObject())
        return;

      if(m_editObjectRequestId != Connection::invalidRequestId)
        m_property.object().connection()->cancelRequest(m_editObjectRequestId);

      m_editObjectRequestId = m_property.getObject(
        [this](const ObjectPtr& object, Message::ErrorCode /*ec*/)
        {
          m_editObjectRequestId = Connection::invalidRequestId;

          if(object)
            MainWindow::instance->showObject(object);
        });
    });
  l->addWidget(m_editButton);

  setLayout(l);
}

PropertyObjectEdit::~PropertyObjectEdit()
{
  if(m_editObjectRequestId != Connection::invalidRequestId)
    m_property.object().connection()->cancelRequest(m_editObjectRequestId);
}
