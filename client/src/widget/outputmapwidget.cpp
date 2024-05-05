/**
 * client/src/widget/outputmapwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2024 Reinder Feenstra
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

#include "outputmapwidget.hpp"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QToolBar>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <traintastic/locale/locale.hpp>
#include "interfaceitemnamelabel.hpp"
#include "propertycheckbox.hpp"
#include "propertycombobox.hpp"
#include "propertyspinbox.hpp"
#include "objectpropertycombobox.hpp"
#include "propertyaddresses.hpp"
#include "outputmapoutputactionwidget.hpp"
#include "../dialog/objectselectlistdialog.hpp"
#include "../network/callmethod.hpp"
#include "../network/method.hpp"
#include "../network/property.hpp"
#include "../network/objectproperty.hpp"
#include "../network/vectorproperty.hpp"
#include "../network/objectvectorproperty.hpp"
#include "../utils/enum.hpp"
#include "../theme/theme.hpp"
#include "../misc/methodaction.hpp"

constexpr int columnCountNonOutput = 2;
constexpr int columnUse = 0;
constexpr int columnKey = 1;

static void setComboBoxMinimumWidth(QComboBox* comboBox)
{
  comboBox->setMinimumWidth(25 * comboBox->fontMetrics().averageCharWidth());
}

OutputMapWidget::OutputMapWidget(ObjectPtr object, QWidget* parent)
  : QWidget(parent)
  , m_object{std::move(object)}
  , m_addresses{m_object->getVectorProperty("addresses")}
  , m_ecosObject{dynamic_cast<Property*>(m_object->getProperty("ecos_object"))}
  , m_items{m_object->getObjectVectorProperty("items")}
  , m_table{new QTableWidget(this)}
{
  QVBoxLayout* l = new QVBoxLayout();

  QFormLayout* form = new QFormLayout();
  if(auto* interface = dynamic_cast<ObjectProperty*>(m_object->getProperty("interface")))
  {
    auto* comboBox = new ObjectPropertyComboBox(*interface, this);
    setComboBoxMinimumWidth(comboBox);
    form->addRow(new InterfaceItemNameLabel(*interface, this), comboBox);
  }
  if(auto* channel = dynamic_cast<Property*>(m_object->getProperty("channel")))
  {
    auto* comboBox = new PropertyComboBox(*channel, this);
    setComboBoxMinimumWidth(comboBox);
    form->addRow(new InterfaceItemNameLabel(*channel, this), comboBox);
  }
  if(m_addresses)
  {
    form->addRow(new InterfaceItemNameLabel(*m_addresses, this), new PropertyAddresses(*m_addresses, m_object->getMethod("add_address"), m_object->getMethod("remove_address"), this));
    connect(m_addresses, &AbstractVectorProperty::valueChanged, this, &OutputMapWidget::updateTableOutputColumns);
  }
  if(m_ecosObject)
  {
    auto* comboBox = new PropertyComboBox(*m_ecosObject, this);
    setComboBoxMinimumWidth(comboBox);
    form->addRow(new InterfaceItemNameLabel(*m_ecosObject, this), comboBox);
    connect(m_ecosObject, &AbstractVectorProperty::valueChanged, this, &OutputMapWidget::updateTableOutputColumns);
  }
  l->addLayout(form);

  m_table->setColumnCount(columnCountNonOutput);
  m_table->setRowCount(0);
  m_table->setHorizontalHeaderLabels({Locale::tr("output_map:use"), Locale::tr(m_object->classId() + ":key")});
  m_table->verticalHeader()->setVisible(false);
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

  l->addWidget(m_table);

  setLayout(l);

  if(m_items) /*[[likely]]*/
  {
    m_getItemsRequestId = m_items->getObjects(
      [this](const std::vector<ObjectPtr>& objects, std::optional<const Error> ec)
      {
        if(!ec)
        {
          updateItems(objects);
        }
      });
  }
}

void OutputMapWidget::updateItems(const std::vector<ObjectPtr>& items)
{
  m_table->setRowCount(items.size());
  m_itemObjects = items;
  m_actions.resize(items.size());
  for(size_t i = 0; i < items.size(); i++)
  {
    if(auto* p = dynamic_cast<Property*>(items[i]->getProperty("use")))
    {
      QWidget* w = new QWidget(m_table);
      QHBoxLayout* l = new QHBoxLayout();
      l->setAlignment(Qt::AlignCenter);
      l->addWidget(new PropertyCheckBox(*p, w));
      w->setLayout(l);
      m_table->setCellWidget(i, columnUse, w);
    }

    if(auto* p = items[i]->getProperty("key"))
    {
      QString text;
      if(p->type() ==  ValueType::Enum)
      {
        text = translateEnum(*p);
      }
      else if(p->type() == ValueType::Boolean)
      {
        text = p->toBool() ? "true" : "false";
      }
      else /*[[unlikely]]*/
      {
        assert(false);
        text = "?";
      }
      m_table->setItem(i, columnKey, new QTableWidgetItem(text));
    }

    if(auto* outputActions = dynamic_cast<ObjectVectorProperty*>(items[i]->getVectorProperty("output_actions")))
    {
      updateTableOutputActions(*outputActions, i);

      connect(outputActions, &ObjectVectorProperty::valueChanged, this,
        [this, row=i]()
        {
          updateTableOutputActions(*dynamic_cast<ObjectVectorProperty*>(sender()), row);
        });
    }
  }

  updateTableOutputColumns();
}

void OutputMapWidget::updateTableOutputColumns()
{
  if(m_addresses && m_addresses->getAttributeBool(AttributeName::Visible, true))
  {
    const auto size = m_addresses->size();

    m_table->setColumnCount(columnCountNonOutput + size);
    for(int i = 0; i < size; i++)
    {
      const int column = columnCountNonOutput + i;
      const int address = m_addresses->getInt(i);
      auto* item = new QTableWidgetItem(QString("#%1").arg(address));
      item->setToolTip(Locale::tr("output_map:address_x").arg(address));
      m_table->setHorizontalHeaderItem(column, item);
    }
  }
  else if(m_ecosObject && m_ecosObject->getAttributeBool(AttributeName::Visible, true))
  {
    m_table->setColumnCount(columnCountNonOutput + 1);
    m_table->setHorizontalHeaderItem(columnCountNonOutput, new QTableWidgetItem(Locale::tr("output.ecos_object:state")));
  }
  else
  {
    m_table->setColumnCount(columnCountNonOutput);
  }
}

void OutputMapWidget::updateTableOutputActions(ObjectVectorProperty& property, int row)
{
  if(!property.empty())
  {
    m_dummy = property.getObjects(
      [this, row](const std::vector<ObjectPtr>& objects, std::optional<const Error> /*ec*/)
      {
        const int columnCount = static_cast<int>(columnCountNonOutput + objects.size());
        if(columnCount > m_table->columnCount())
        {
          m_table->setColumnCount(columnCount);
        }

        auto& rowActions = m_actions[row];
        int column = columnCountNonOutput;
        for(auto& object : objects)
        {
          if(column >= static_cast<int>(rowActions.size()) || object.get() != rowActions[column].get())
          {
            if(auto* action = dynamic_cast<Property*>(object->getProperty("action")))
            {
              m_table->setCellWidget(row, column, new PropertyComboBox(*action, this));
            }
            else if(auto* aspect = dynamic_cast<Property*>(object->getProperty("aspect")))
            {
              m_table->setCellWidget(row, column, new PropertySpinBox(*aspect, this));
            }
            else if(auto* state = dynamic_cast<Property*>(object->getProperty("state")))
            {
              m_table->setCellWidget(row, column, new PropertySpinBox(*state, this));
            }
          }
          column++;
        }
        rowActions = objects;
      });
  }
}
