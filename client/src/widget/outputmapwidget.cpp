/**
 * client/src/widget/outputmapwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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
#include <QToolBar>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <traintastic/locale/locale.hpp>
#include "propertycheckbox.hpp"
#include "outputmapoutputactionwidget.hpp"
#include "../dialog/objectselectlistdialog.hpp"
#include "../network/callmethod.hpp"
#include "../network/method.hpp"
#include "../network/property.hpp"
#include "../utils/enum.hpp"
#include "../theme/theme.hpp"
#include "../misc/methodaction.hpp"

constexpr int columnCountNonOutput = 2;
constexpr int columnUse = 0;
constexpr int columnKey = 1;
constexpr int columnOutputFirst = 2;

OutputMapWidget::OutputMapWidget(std::shared_ptr<OutputMap> object, QWidget* parent) :
  QWidget(parent),
  m_object{std::move(object)},
  m_table{new QTableWidget(this)}
{
  QVBoxLayout* l = new QVBoxLayout();

  QToolBar* toolbar = new QToolBar(this);

  if((m_methodAdd = m_object->getMethod("add_output")))
    toolbar->addAction(new MethodAction(Theme::getIcon("add"), *m_methodAdd,
      [this]()
      {
        std::make_unique<ObjectSelectListDialog>(*m_methodAdd, this)->exec();
      }, toolbar));

  if((m_methodRemove = m_object->getMethod("remove_output")))
  {
    m_actionRemove = new MethodAction(Theme::getIcon("remove"), *m_methodRemove,
      [this]()
      {
        const int index = m_table->currentColumn() - columnOutputFirst;
        if(index >= 0)
          callMethod(*m_methodRemove, nullptr, m_object->outputs()[index]);
      }, toolbar);
    m_actionRemove->setForceDisabled(true);
    toolbar->addAction(m_actionRemove);
  }

  l->addWidget(toolbar);

  m_table->setColumnCount(columnCountNonOutput);
  m_table->setRowCount(0);
  m_table->setHorizontalHeaderLabels({Locale::tr("output_map:use"), Locale::tr(m_object->classId() + ":key")});
  m_table->verticalHeader()->setVisible(false);
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->setSelectionMode(QAbstractItemView::SingleSelection);
  m_table->setSelectionBehavior(QAbstractItemView::SelectColumns);
  connect(m_table, &QTableWidget::itemSelectionChanged, this,
    [this]()
    {
      m_actionRemove->setForceDisabled(m_table->currentColumn() < columnOutputFirst);
    });

  l->addWidget(m_table);

  setLayout(l);

  connect(m_object.get(), &OutputMap::itemsChanged, this, &OutputMapWidget::updateItems);
  if(m_object->items().empty())
    m_object->getItems();
  else
    updateItems();

  connect(m_object.get(), &OutputMap::outputsChanged, this, &OutputMapWidget::updateOutputs);
  if(m_object->outputs().empty())
    m_object->getOutputs();
  else
    updateOutputs();
}

void OutputMapWidget::updateItems()
{
  const auto& items = m_object->items();
  m_table->setRowCount(items.size());
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
      m_table->setItem(i, columnKey, new QTableWidgetItem(translateEnum(*p)));
  }
}

void OutputMapWidget::updateOutputs()
{
  const auto& items = m_object->items();
  const auto& outputs = m_object->outputs();
  m_table->setColumnCount(columnCountNonOutput + outputs.size());

  int column = columnOutputFirst;
  for(auto& output : outputs)
  {
    if(auto* p = output->getProperty("name"))
      m_table->setHorizontalHeaderItem(column, new QTableWidgetItem(p->toString()));

    for(int row = 0; row < m_table->rowCount(); row++)
    {
      m_table->setCellWidget(row, column, new OutputMapOutputActionWidget(items[row], output, this));
    }

    column++;
  }
}
