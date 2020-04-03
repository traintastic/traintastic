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
#include "../network/connection.hpp"
#include "../network/object.hpp"
#include "../network/property.hpp"
#include "../network/objectproperty.hpp"
#include "../network/utils.hpp"
#include "alertwidget.hpp"
#include "propertycheckbox.hpp"
#include "propertycombobox.hpp"
#include "propertyspinbox.hpp"
#include "propertylineedit.hpp"
#include "propertytextedit.hpp"
#include "propertydirectioncontrol.hpp"
#include "propertyvaluelabel.hpp"
#include "createwidget.hpp"
#include "../mainwindow.hpp"
#include <enum/category.hpp>

#include <enum/direction.hpp>


QString toString(Category value)
{
  switch(value)
  {
    case Category::General: return "General";
    case Category::Info: return "Info";
    case Category::Status: return "Status";
    case Category::XpressNet: return "XpressNet";
  }
  return "?";
}


ObjectEditWidget::ObjectEditWidget(const ObjectPtr& object, QWidget* parent) :
  QWidget(parent),
  m_requestId{Connection::invalidRequestId},
  m_object{object}
{
  buildForm();
}

ObjectEditWidget::ObjectEditWidget(const QString& id, QWidget* parent) :
  QWidget(parent)
{
  setWindowTitle(id);

  auto* spinner = new WaitingSpinnerWidget(this, true, false);
  spinner->start();

  m_requestId = MainWindow::instance->connection()->getObject(id,
    [this, spinner](const ObjectPtr& object, Message::ErrorCode ec)
    {
      m_requestId = Connection::invalidRequestId;
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
  m_object->connection()->cancelRequest(m_requestId);
}

void ObjectEditWidget::buildForm()
{
  if(AbstractProperty* id = m_object->getProperty("id"))
  {
    connect(id, &AbstractProperty::valueChangedString, this, &ObjectEditWidget::setWindowTitle);
    setWindowTitle(id->toString());
  }

  if(QWidget* widget = createWidgetIfCustom(m_object))
  {
    QVBoxLayout* l = new QVBoxLayout();
    l->setMargin(0);
    l->addWidget(widget);
    setLayout(l);
  }
  else
  {
    QList<QWidget*> tabs;
    QMap<Category, QWidget*> categoryTabs;

    for(const QString& name : m_object->interfaceItems().names())
      if(AbstractProperty* baseProperty = m_object->getProperty(name))
      {
        if(!baseProperty->getAttributeBool(AttributeName::ObjectEditor, true))
          continue;

        if(baseProperty->type() == ValueType::Object)
        {
          ObjectProperty* property = static_cast<ObjectProperty*>(baseProperty);
          if(contains(baseProperty->flags(), PropertyFlags::SubObject))
          {
            QWidget* w = new ObjectEditWidget(property->objectId());
            w->setWindowTitle(property->displayName());
            tabs.append(w);
            continue;
          }
        }
        else
        {
          Property* property = static_cast<Property*>(baseProperty);
          Category category = property->getAttributeEnum<Category>(AttributeName::Category, Category::General);
          QWidget* w = nullptr;

          if(!property->isWritable())
            w = new PropertyValueLabel(*property);
          else if(property->type() == ValueType::Boolean)
            w = new PropertyCheckBox(*property);
          else if(property->type() == ValueType::Integer)
            w = new PropertySpinBox(*property);
          else if(property->type() == ValueType::String)
          {
            if(property->name() == "notes")
            {
              PropertyTextEdit* edit = new PropertyTextEdit(*property);
              edit->setWindowTitle(property->displayName());
              edit->setPlaceholderText(property->displayName());
              tabs.append(edit);
              continue;
            }
            else if(property->name() == "code")
            {
              PropertyTextEdit* edit = new PropertyTextEdit(*property);
              edit->setWindowTitle(property->displayName());
              edit->setPlaceholderText(property->displayName());
              tabs.append(edit);
              continue;
            }
            else
              w = new PropertyLineEdit(*property);
          }
          else if(property->type() == ValueType::Enum)
          {
            if(property->enumName() == EnumName<Direction>::value)
              w = new PropertyDirectionControl(*property);
            else
              w = new PropertyComboBox(*property);
          }

          QWidget* tabWidget;
          if(!categoryTabs.contains(category))
          {
            tabWidget = new QWidget();
            tabWidget->setWindowTitle(toString(category));
            tabWidget->setLayout(new QFormLayout());
            tabs.append(tabWidget);
            categoryTabs.insert(category, tabWidget);
          }
          else
            tabWidget = categoryTabs[category];

          static_cast<QFormLayout*>(tabWidget->layout())->addRow(property->displayName(), w);
        }
      }

    if(tabs.count() > 1)
    {
      QTabWidget* tabWidget = new QTabWidget();
      for(auto* tab : tabs)
        tabWidget->addTab(tab, tab->windowTitle());
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
}
