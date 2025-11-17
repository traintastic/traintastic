/**
 * client/src/widget/object/objecteditwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2025 Reinder Feenstra
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
#include <QToolButton>
#include <QHBoxLayout>
#include "../../network/object.hpp"
#include "../../network/property.hpp"
#include "../../network/objectproperty.hpp"
#include "../../network/unitproperty.hpp"
#include "../../network/method.hpp"
#include "../interfaceitemnamelabel.hpp"
#include "../propertycheckbox.hpp"
#include "../propertycombobox.hpp"
#include "../objectpropertycombobox.hpp"
#include "../propertyspinbox.hpp"
#include "../propertydoublespinbox.hpp"
#include "../propertylineedit.hpp"
#include "../propertytextedit.hpp"
#include "../propertyobjectedit.hpp"
#include "../propertydirectioncontrol.hpp"
#include "../propertyvaluelabel.hpp"
#include "../methodpushbutton.hpp"
#include "../unitpropertycombobox.hpp"
#include "../unitpropertyedit.hpp"
#include "../createwidget.hpp"
#include "../decoder/decoderwidget.hpp"
#include "../decoder/decoderfunctionswidget.hpp"
#include "../../theme/theme.hpp"
#include <traintastic/enum/direction.hpp>
#include <traintastic/locale/locale.hpp>

ObjectEditWidget::ObjectEditWidget(const ObjectPtr& object, QWidget* parent) :
  AbstractEditWidget(object, parent)
{
  buildForm();
}

ObjectEditWidget::ObjectEditWidget(const QString& id, QWidget* parent) :
  AbstractEditWidget(id, parent)
{
}

ObjectEditWidget::ObjectEditWidget(ObjectProperty& property, QWidget* parent)
  : AbstractEditWidget(property, parent)
{
}

void ObjectEditWidget::buildForm()
{
  setObjectWindowTitle();
  setWindowIcon(Theme::getIconForClassId(m_object->classId()));

  if(QWidget* widget = createWidgetIfCustom(m_object))
  {
    QVBoxLayout* l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(widget);
    setLayout(l);
  }
  else
  {
    QList<QWidget*> tabs;
    QMap<QString, QWidget*> categoryTabs;

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
            if(property->name() == "decoder")
            {
              tabs.append(new DecoderWidget(*property, this));
              tabs.append(new DecoderFunctionsWidget(*property, this));
              continue;
            }
            else if(contains(baseProperty->flags(), PropertyFlags::SubObject))
            {
              tabs.append(new ObjectEditWidget(*property, this));
              continue;
            }
            else if(property->name() == "interface")
            {
              w = new ObjectPropertyComboBox(*property, this);
            }
            else
            {
              w = new PropertyObjectEdit(*property, this);
            }
          }
          else
          {
            Property* property = static_cast<Property*>(baseProperty);
            if(UnitProperty* unitProperty = dynamic_cast<UnitProperty*>(property))
            {
              if(unitProperty->hasAttribute(AttributeName::Values))
              {
                w = new UnitPropertyComboBox(*unitProperty, this);
              }
              else
              {
                w = new UnitPropertyEdit(*unitProperty, this);
              }
            }
            else if(!property->isWritable())
              w = new PropertyValueLabel(*property, this);
            else if(property->type() == ValueType::Boolean)
              w = new PropertyCheckBox(*property, this);
            else if(property->type() == ValueType::Integer)
            {
              w = createWidget(*property, this);
            }
            else if(property->type() == ValueType::Float)
              w = createWidget(*property, this);
            else if(property->type() == ValueType::String)
            {
              if(property->name() == "notes")
              {
                PropertyTextEdit* edit = new PropertyTextEdit(*property, this);
                edit->setWindowTitle(property->displayName());
                edit->setPlaceholderText(property->displayName());
                tabs.append(edit);
                continue;
              }
              else if(property->name() == "code")
              {
                PropertyTextEdit* edit = new PropertyTextEdit(*property, this);
                edit->setWindowTitle(property->displayName());
                edit->setPlaceholderText(property->displayName());
                tabs.append(edit);
                continue;
              }
              else
                w = createWidget(*property, this);
            }
            else if(property->type() == ValueType::Enum)
            {
              if(property->enumName() == EnumName<Direction>::value)
                w = new PropertyDirectionControl(*property, this);
              else
                w = createWidget(*property, this);
            }
          }
        }
        else if(Method* method = dynamic_cast<Method*>(item))
        {
          w = new MethodPushButton(*method, this);
        }

        const QString category = item->getAttributeString(AttributeName::Category, "category:general");
        QWidget* tabWidget;
        if(!categoryTabs.contains(category))
        {
          tabWidget = new QWidget(this);
          tabWidget->setWindowTitle(Locale::tr(category));
          tabWidget->setLayout(new QFormLayout());
          tabs.append(tabWidget);
          categoryTabs.insert(category, tabWidget);
        }
        else
          tabWidget = categoryTabs[category];

        InterfaceItemNameLabel* label = new InterfaceItemNameLabel(*item, this);
        static_cast<QFormLayout*>(tabWidget->layout())->addRow(label, w);

      }
    }

    if(tabs.count() > 1)
    {
      QTabWidget* mainTabWidget = new QTabWidget(this);
      for(auto* tab : tabs)
        mainTabWidget->addTab(tab, tab->windowTitle());
      QVBoxLayout* l = new QVBoxLayout();
      l->setContentsMargins(0, 0, 0, 0);
      l->addWidget(mainTabWidget);
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
