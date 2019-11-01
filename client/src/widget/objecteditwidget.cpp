/**
 * client/src/widget/objecteditwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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

#include "objecteditwidget.hpp"
#include <QFormLayout>
#include <QtWaitingSpinner/waitingspinnerwidget.h>
#include "../network/client.hpp"
#include "../network/object.hpp"
#include "../network/property.hpp"
#include "../network/utils.hpp"
#include "../widget/alertwidget.hpp"
#include "../widget/propertycheckbox.hpp"

ObjectEditWidget::ObjectEditWidget(const QString& id, QWidget* parent) :
  QWidget(parent),
  m_id{id}
{
  setLayout(new QFormLayout());

  auto* spinner = new WaitingSpinnerWidget(this, true, false);
  spinner->start();

  m_requestId = Client::instance->getObject(m_id,
    [this, spinner](const ObjectPtr& object, Message::ErrorCode ec)
    {
      m_requestId = Client::invalidRequestId;
      if(object)
      {
        m_object = object;
        buildForm();
      }
      else
        static_cast<QFormLayout*>(this->layout())->addRow(AlertWidget::error(errorCodeToText(ec)));
      delete spinner;
    });
}

ObjectEditWidget::~ObjectEditWidget()
{
  Client::instance->cancelRequest(m_requestId);
}

void ObjectEditWidget::buildForm()
{
  QFormLayout* l = static_cast<QFormLayout*>(this->layout());

  for(auto item : m_object->interfaceItems())
    if(Property* property = dynamic_cast<Property*>(item))
    {
      QWidget* w = nullptr;

      if(property->type() == PropertyType::Boolean)
        w = new PropertyCheckBox(*property);

      l->addRow(property->displayName(), w);
    }
}
