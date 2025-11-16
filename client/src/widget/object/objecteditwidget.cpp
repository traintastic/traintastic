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

    if(QWidget* customWidget = createWidgetIfCustom(m_object))
    {
        QVBoxLayout* l = new QVBoxLayout();
        l->setContentsMargins(0, 0, 0, 0);
        l->addWidget(customWidget);
        setLayout(l);
        return;
    }

    QList<QWidget*> tabs;
    QMap<QString, QWidget*> categoryTabs;

    for(const QString& name : m_object->interfaceItems().names())
    {
        if(InterfaceItem* item = m_object->getInterfaceItem(name))
        {
            if(!item->getAttributeBool(AttributeName::ObjectEditor, true))
                continue;

            QWidget* editorWidget = nullptr;

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
                        editorWidget = new ObjectPropertyComboBox(*property, this);
                    }
                    else
                    {
                        editorWidget = new PropertyObjectEdit(*property, this);
                    }
                }
                else
                {
                    Property* property = static_cast<Property*>(baseProperty);
                    if(UnitProperty* unitProperty = dynamic_cast<UnitProperty*>(property))
                    {
                        if(unitProperty->hasAttribute(AttributeName::Values))
                            editorWidget = new UnitPropertyComboBox(*unitProperty, this);
                        else
                            editorWidget = new UnitPropertyEdit(*unitProperty, this);
                    }
                    else if(!property->isWritable())
                        editorWidget = new PropertyValueLabel(*property, this);
                    else if(property->type() == ValueType::Boolean)
                        editorWidget = new PropertyCheckBox(*property, this);
                    else
                        editorWidget = createWidget(*property, this);
                }
            }
            else if(Method* method = dynamic_cast<Method*>(item))
            {
                editorWidget = new MethodPushButton(*method, this);
            }

            const QString category = item->getAttributeString(AttributeName::Category, "category:general");
            QWidget* tabPage = nullptr;

            if(!categoryTabs.contains(category))
            {
                tabPage = new QWidget(this);
                tabPage->setWindowTitle(Locale::tr(category));
                tabPage->setLayout(new QFormLayout());
                tabs.append(tabPage);
                categoryTabs.insert(category, tabPage);
            }
            else
                tabPage = categoryTabs[category];

            // Create label + optional info button
            InterfaceItemNameLabel* label = new InterfaceItemNameLabel(*item, this);
            const QString helpText = item->getAttributeString(AttributeName::Help, QString());

            if(!helpText.isEmpty())
            {
                label->setToolTip(helpText);

                if(editorWidget)
                    editorWidget->setToolTip(helpText);

                QToolButton* infoButton = new QToolButton(this);
                infoButton->setText(QString::fromUtf8("ⓘ"));
                infoButton->setToolTip(helpText);
                infoButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
                infoButton->setCursor(Qt::WhatsThisCursor);

                QHBoxLayout* labelLayout = new QHBoxLayout();
                labelLayout->setContentsMargins(0, 0, 0, 0);
                labelLayout->addWidget(label);
                labelLayout->addWidget(infoButton, 0, Qt::AlignRight);

                QWidget* labelContainer = new QWidget(this);
                labelContainer->setLayout(labelLayout);

                static_cast<QFormLayout*>(tabPage->layout())->addRow(labelContainer, editorWidget);
            }
            else
            {
                static_cast<QFormLayout*>(tabPage->layout())->addRow(label, editorWidget);
            }
        }
    }

    // Final layout
    if(tabs.count() > 1)
    {
        QTabWidget* mainTabWidget = new QTabWidget(this);
        for(auto* t : tabs)
            mainTabWidget->addTab(t, t->windowTitle());

        QVBoxLayout* l = new QVBoxLayout();
        l->setContentsMargins(0, 0, 0, 0);
        l->addWidget(mainTabWidget);
        setLayout(l);
    }
    else if(tabs.count() == 1)
    {
        QWidget* onlyTab = tabs.first();
        setLayout(onlyTab->layout());
        delete onlyTab;
    }
}
