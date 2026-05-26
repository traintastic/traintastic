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

#include "decoderwidget.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include "../../network/connection.hpp"
#include "../../network/error.hpp"
#include "../../network/method.hpp"
#include "../../network/object.hpp"
#include "../../network/objectproperty.hpp"
#include "../../network/property.hpp"
#include "../object/objecteditwidget.hpp"
#include "../interfaceitemnamelabel.hpp"
#include "../objectpropertycombobox.hpp"
#include "../createwidget.hpp"

DecoderWidget::DecoderWidget(ObjectProperty& decoderProperty, QWidget* parent)
  : QWidget(parent)
  , m_decoderProperty{decoderProperty}
  , m_createDecoder{m_decoderProperty.object().getMethod("create_decoder")}
  , m_requestId{Connection::invalidRequestId}
{
  setWindowTitle(m_decoderProperty.displayName());

  connect(&m_decoderProperty, &ObjectProperty::valueChanged, this, &DecoderWidget::decoderChanged);

  decoderChanged();
}

DecoderWidget::~DecoderWidget()
{
  cancelRequest();
}

void DecoderWidget::cancelRequest()
{
  if(m_requestId != Connection::invalidRequestId)
  {
    m_decoderProperty.object().connection()->cancelRequest(m_requestId);
    m_requestId = Connection::invalidRequestId;
  }
}

void DecoderWidget::decoderChanged()
{
  delete layout();
  delete m_createDecoderButton;

  m_createDecoderButton = nullptr;
  m_object.reset();

  cancelRequest();

  if(m_decoderProperty.hasObject())
  {
    m_requestId = m_decoderProperty.getObject(
      [this](const ObjectPtr& object, std::optional<const Error> /*error*/)
      {
        m_requestId = Connection::invalidRequestId;
        if(object)
        {
          m_object = object;

          auto* form = new QFormLayout();
          if(auto* interface = dynamic_cast<ObjectProperty*>(m_object->getProperty("interface")))
          {
            form->addRow(new InterfaceItemNameLabel(*interface, this), new ObjectPropertyComboBox(*interface, this));
          }
          if(auto* protocol = dynamic_cast<Property*>(m_object->getProperty("protocol")))
          {
            form->addRow(new InterfaceItemNameLabel(*protocol, this), createWidget(*protocol, this));
          }
          if(auto* address = dynamic_cast<Property*>(m_object->getProperty("address")))
          {
            form->addRow(new InterfaceItemNameLabel(*address, this), createWidget(*address, this));
          }
          if(auto* speedSteps = dynamic_cast<Property*>(m_object->getProperty("speed_steps")))
          {
            form->addRow(new InterfaceItemNameLabel(*speedSteps, this), createWidget(*speedSteps, this));
          }
          setLayout(form);
        }
      });
  }
  else
  {
    m_createDecoderButton = new QPushButton("Add decoder", this);
    m_createDecoderButton->setEnabled(m_createDecoder);
    connect(m_createDecoderButton, &QPushButton::clicked,
      [this]()
      {
        if(m_createDecoder) [[likely]]
        {
          m_createDecoder->call();
        }
      });

    auto* h = new QHBoxLayout();
    h->addStretch();
    h->addWidget(m_createDecoderButton);
    h->addStretch();

    auto* v = new QVBoxLayout();
    v->addStretch();
    v->addLayout(h);
    v->addStretch();
    setLayout(v);
  }
}
