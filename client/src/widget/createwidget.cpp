/**
 * client/src/widget/createwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2025 Reinder Feenstra
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

#include "createwidget.hpp"
#include "list/marklincanlocomotivelistwidget.hpp"
#include "objectlist/boardlistwidget.hpp"
#include "objectlist/interfacelistwidget.hpp"
#include "objectlist/throttleobjectlistwidget.hpp"
#include "objectlist/trainlistwidget.hpp"
#include "objectlist/zoneblocklistwidget.hpp"
#include "object/luascripteditwidget.hpp"
#include "object/objecteditwidget.hpp"
#include "object/itemseditwidget.hpp"
#include "tile/turnouttilewidget.hpp"
#include "inputmonitorwidget.hpp"
#include "outputkeyboardwidget.hpp"
#include "outputmapwidget.hpp"
#include "propertycheckbox.hpp"
#include "propertycombobox.hpp"
#include "propertydoublespinbox.hpp"
#include "propertyspinbox.hpp"
#include "propertylineedit.hpp"
#include "propertypairoutputaction.hpp"
#include "../board/boardwidget.hpp"
#include "../network/object.hpp"
#include "../network/inputmonitor.hpp"
#include "../network/outputkeyboard.hpp"
#include "../network/board.hpp"
#include "../network/property.hpp"
#include <QLabel>
#include <QFormLayout>




QWidget* createWidgetIfCustom(const ObjectPtr& object, QWidget* parent)
{
  const QString& classId = object->classId();

  if(classId == "list.interface")
  {
    return new InterfaceListWidget(object, parent);
  }
  else if(classId == "controller_list")
    return new ObjectListWidget(object, parent); // todo remove
  else if(classId == "rail_vehicle_list")
    return new ObjectListWidget(object, parent); // todo remove
  else if(classId == "lua.script_list")
    return new ObjectListWidget(object, parent); // todo remove
  else if(classId == "world_list")
    return new ObjectListWidget(object, parent);
  if(classId == "list.board")
  {
    return new BoardListWidget(object, parent);
  }
  if(classId == "list.train")
  {
    return new TrainListWidget(object, parent);
  }
  if(classId == "list.zone_block")
  {
    return new ZoneBlockListWidget(object, parent);
  }
  else if(object->classId().startsWith("list."))
    return new ObjectListWidget(object, parent);
  else if(classId == "lua.script")
    return new LuaScriptEditWidget(object, parent);
  else if(classId.startsWith("output_map."))
    return new OutputMapWidget(object, parent);
  else if(classId == "input_map.block" || classId == "decoder_functions")
    return new ItemsEditWidget(object, parent);
  else if(classId == "marklin_can_node_list")
    return new ListWidget(object, parent);
  else if(classId == "marklin_can_locomotive_list")
    return new MarklinCANLocomotiveListWidget(object, parent);
  else if(object->classId().startsWith("board_tile.rail.turnout"))
  {
    return new TurnoutTileWidget(object, parent);
  }
  else
    return nullptr;
}

QWidget* createWidget(const ObjectPtr& object, QWidget* parent)
{
  if(QWidget* widget = createWidgetIfCustom(object, parent))
    return widget;
  else if(auto inputMonitor = std::dynamic_pointer_cast<InputMonitor>(object))
    return new InputMonitorWidget(inputMonitor, parent);
  else if(auto outputKeyboard = std::dynamic_pointer_cast<OutputKeyboard>(object))
    return new OutputKeyboardWidget(outputKeyboard, parent);
  else
    return new ObjectEditWidget(object, parent);
}

QWidget* createWidget(InterfaceItem& item, QWidget* parent)
{
  if(auto* baseProperty = dynamic_cast<AbstractProperty*>(&item))
  {
    return createWidget(*baseProperty, parent);
  }
  assert(false);
  return nullptr;
}

QWidget* createWidget(AbstractProperty& baseProperty, QWidget* parent)
{
  if(auto* property = dynamic_cast<Property*>(&baseProperty))
  {
    return createWidget(*property, parent);
  }
  assert(false);
  return nullptr;
}

QWidget* createWidget(Property& property, QWidget* parent)
{
    QWidget* widget = nullptr;

    // 1. Create the appropriate editor widget
    switch(property.type())
    {
        case ValueType::Boolean:
            widget = new PropertyCheckBox(property, parent);
            break;

        case ValueType::Enum:
            if(property.enumName() == "pair_output_action")
                widget = new PropertyPairOutputAction(property, parent);
            else
                widget = new PropertyComboBox(property, parent);
            break;

        case ValueType::Integer:
            if(property.hasAttribute(AttributeName::Values) &&
               !property.hasAttribute(AttributeName::Min) &&
               !property.hasAttribute(AttributeName::Max))
            {
                widget = new PropertyComboBox(property, parent);
            }
            else
                widget = new PropertySpinBox(property, parent);
            break;

        case ValueType::Float:
            widget = new PropertyDoubleSpinBox(property, parent);
            break;

        case ValueType::String:
            if(property.hasAttribute(AttributeName::Values))
                widget = new PropertyComboBox(property, parent);
            else
                widget = new PropertyLineEdit(property, parent);
            break;

        case ValueType::Object:
        case ValueType::Set:
        case ValueType::Invalid:
            break;
    }

    if(widget && property.hasAttribute(AttributeName::Help))
    {
        QVariant helpVar = property.getAttribute(AttributeName::Help, QVariant());
        if(helpVar.isValid() && !helpVar.toString().isEmpty())
        {
            QString helpText = helpVar.toString();

            // Apply to the editor itself
            widget->setToolTip(helpText);

            // Try to apply tooltip to the label, if available via custom property widget
            if(auto* comboBox = dynamic_cast<PropertyComboBox*>(widget))
            {
                if(QLabel* label = comboBox->label())
                {
                    label->setToolTip(helpText);
                    return widget; // done
                }
            }
            else if(auto* checkBox = dynamic_cast<PropertyCheckBox*>(widget))
            {
                if(QLabel* label = checkBox->label())
                {
                    label->setToolTip(helpText);
                    return widget;
                }
            }
            else if(auto* spinBox = dynamic_cast<PropertySpinBox*>(widget))
            {
                if(QLabel* label = spinBox->label())
                {
                    label->setToolTip(helpText);
                    return widget;
                }
            }
            else if(auto* doubleSpin = dynamic_cast<PropertyDoubleSpinBox*>(widget))
            {
                if(QLabel* label = doubleSpin->label())
                {
                    label->setToolTip(helpText);
                    return widget;
                }
            }
            else if(auto* lineEdit = dynamic_cast<PropertyLineEdit*>(widget))
            {
                if(QLabel* label = lineEdit->label())
                {
                    label->setToolTip(helpText);
                    return widget;
                }
            }

            // Fallback: try to traverse QFormLayout if present
            QWidget* current = widget->parentWidget();
            while(current)
            {
                if(auto formLayout = qobject_cast<QFormLayout*>(current->layout()))
                {
                    for(int row = 0; row < formLayout->rowCount(); ++row)
                    {
                        QWidget* fieldWidget = formLayout->itemAt(row, QFormLayout::FieldRole)->widget();
                        if(fieldWidget == widget)
                        {
                            if(QLabel* label = qobject_cast<QLabel*>(formLayout->itemAt(row, QFormLayout::LabelRole)->widget()))
                            {
                                label->setToolTip(helpText);
                            }
                            break;
                        }
                    }
                    break; // found the form layout
                }
                current = current->parentWidget();
            }
        }
    }

    return widget;
}


