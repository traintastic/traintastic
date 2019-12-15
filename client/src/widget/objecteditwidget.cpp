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
#include <QVBoxLayout>
#include <QTabWidget>
#include <QtWaitingSpinner/waitingspinnerwidget.h>
#include "../network/client.hpp"
#include "../network/object.hpp"
#include "../network/property.hpp"
#include "../network/utils.hpp"
#include "../widget/alertwidget.hpp"
#include "../widget/propertycheckbox.hpp"
#include "../widget/propertyspinbox.hpp"
#include "../widget/propertylineedit.hpp"
#include "../widget/propertytextedit.hpp"
#include "../widget/propertydirectioncontrol.hpp"

#include <enum/direction.hpp>

ObjectEditWidget::ObjectEditWidget(const QString& id, QWidget* parent) :
  QWidget(parent),
  m_id{id}
{
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
  QMap<QString, QWidget*> tabs;

  for(const QString& name : m_object->interfaceItems().names())
    if(Property* property = m_object->getProperty(name))
    {
      QWidget* w = nullptr;

      if(property->type() == PropertyType::Boolean)
        w = new PropertyCheckBox(*property);
      else if(property->type() == PropertyType::Integer)
        w = new PropertySpinBox(*property);
      else if(property->type() == PropertyType::String)
      {
        if(property->name() == "notes")
        {
          PropertyTextEdit* edit = new PropertyTextEdit(*property);
          edit->setPlaceholderText(property->displayName());
          Q_ASSERT(!tabs.contains("notes"));
          tabs.insert("notes", edit);
          continue;
        }
        else
          w = new PropertyLineEdit(*property);
      }
      else if(property->type() == PropertyType::Enum)
      {
        if(property->enumName() == EnumName<Direction>::value)
        {
          w = new PropertyDirectionControl(*property);
        }

        //else
        //  w = dropdown
      }

      QWidget* tabWidget;
      QString tab = "general"; // TODO: get sttribute
      if(!tabs.contains(tab))
      {
        tabWidget = new QWidget();
        tabWidget->setLayout(new QFormLayout());
        tabs.insert(tab, tabWidget);
      }
      else
        tabWidget = tabs[tab];

      static_cast<QFormLayout*>(tabWidget->layout())->addRow(property->displayName(), w);
    }

  if(tabs.count() > 1)
  {
    QTabWidget* tabWidget = new QTabWidget();
    for(auto it = tabs.constBegin(); it != tabs.constEnd(); it++)
      tabWidget->addTab(it.value(), it.key());
    QVBoxLayout* l = new QVBoxLayout();
    l->setMargin(0);
    l->addWidget(tabWidget);
    setLayout(l);
  }
  else if(tabs.count() == 1)
  {
    QWidget* w = tabs.first();
    setLayout(w->layout());
    delete w;
  }
}
