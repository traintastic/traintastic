/**
 * client/src/widget/object/objecteditwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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
//#include <QTabWidget>
//#include <QtWaitingSpinner/waitingspinnerwidget.h>
//#include "../../network/connection.hpp"
#include "../../network/object.hpp"
#include "../../network/property.hpp"
#include "../../network/objectproperty.hpp"
#include "../../network/unitproperty.hpp"
#include "../../network/method.hpp"
//#include "../../network/utils.hpp"
//#include "../alertwidget.hpp"
#include "../propertycheckbox.hpp"
#include "../propertycombobox.hpp"
#include "../propertyspinbox.hpp"
#include "../propertylineedit.hpp"
#include "../propertytextedit.hpp"
#include "../propertyobjectedit.hpp"
#include "../propertydirectioncontrol.hpp"
#include "../propertyvaluelabel.hpp"
#include "../methodpushbutton.hpp"
#include "../unitpropertyedit.hpp"
#include "../createwidget.hpp"
#include "../../utils/geticonforclassid.hpp"
//#include "../../mainwindow.hpp"
#include <traintastic/enum/category.hpp>
#include <traintastic/enum/direction.hpp>

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
  AbstractEditWidget(object, parent)
{
  buildForm();
}

ObjectEditWidget::ObjectEditWidget(const QString& id, QWidget* parent) :
  AbstractEditWidget(id, parent)
{
}

void ObjectEditWidget::buildForm()
{
  setIdAsWindowTitle();
  setWindowIcon(getIconForClassId(m_object->classId()));

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
    {
      if(InterfaceItem* item = m_object->getInterfaceItem(name))
      {
        if(!item->getAttributeBool(AttributeName::ObjectEditor, true))
          continue;

        QWidget* w = nullptr;

        if(AbstractProperty* baseProperty = dynamic_cast<AbstractProperty*>(item))
        {
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
            else
            {
              w = new PropertyObjectEdit(*property);
            }
          }
          else
          {
            Property* property = static_cast<Property*>(baseProperty);
            if(UnitProperty* unitProperty = dynamic_cast<UnitProperty*>(property))
              w = new UnitPropertyEdit(*unitProperty);
            else if(!property->isWritable())
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
          }
        }
        else if(Method* method = dynamic_cast<Method*>(item))
        {
          w = new MethodPushButton(*method, this);
        }

        Category category = item->getAttributeEnum<Category>(AttributeName::Category, Category::General);
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

        static_cast<QFormLayout*>(tabWidget->layout())->addRow(item->displayName(), w);
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
