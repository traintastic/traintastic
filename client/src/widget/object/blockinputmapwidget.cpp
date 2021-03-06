/**
 * client/src/widget/object/blockinputmapwidget.cpp
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

#include "blockinputmapwidget.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolBar>
#include <QListWidget>
#include <QStackedWidget>
#include "objecteditwidget.hpp"
#include "../../network/object.hpp"
#include "../../network/objectvectorproperty.hpp"
#include "../../network/method.hpp"
#include "../../network/callmethod.hpp"
#include "../../misc/methodaction.hpp"
#include "../../theme/theme.hpp"

constexpr int objectIdRole = Qt::UserRole;

BlockInputMapWidget::BlockInputMapWidget(const ObjectPtr& object, QWidget* parent) :
  AbstractEditWidget(object, parent),
  m_methodAdd{nullptr},
  m_stack{nullptr},
  m_list{nullptr}
{
  buildForm();
}

BlockInputMapWidget::BlockInputMapWidget(const QString& id, QWidget* parent) :
  AbstractEditWidget(id, parent),
  m_methodAdd{nullptr},
  m_methodRemove{nullptr},
  m_stack{nullptr},
  m_list{nullptr}
{
}

void BlockInputMapWidget::buildForm()
{
  setIdAsWindowTitle();
  setWindowIcon(Theme::getIconForClassId(m_object->classId()));

  m_propertyItems = dynamic_cast<ObjectVectorProperty*>(m_object->getVectorProperty("items"));
  m_methodAdd = m_object->getMethod("add");
  m_methodRemove = m_object->getMethod("remove");
  m_methodMoveUp = m_object->getMethod("move_up");
  m_methodMoveDown = m_object->getMethod("move_down");

  if(Q_UNLIKELY(!(m_propertyItems && m_methodAdd && m_methodRemove && m_methodMoveUp && m_methodMoveDown)))
    return;

  QVBoxLayout* left = new QVBoxLayout();

  QToolBar* toolbar = new QToolBar(this);
  QAction* act;
  toolbar->addAction(new MethodAction(Theme::getIcon("add"), *m_methodAdd, toolbar));
  toolbar->addAction(new MethodAction(Theme::getIcon("remove"), *m_methodRemove,
    [this]()
    {
      if(auto* item = m_list->currentItem())
        callMethod(*m_methodRemove, nullptr, item->data(objectIdRole).toString());
    }, toolbar));
  toolbar->addAction(new MethodAction(Theme::getIcon("up"), *m_methodMoveUp,
    [this]()
    {
      if(auto* item = m_list->currentItem())
        callMethod(*m_methodMoveUp, nullptr, item->data(objectIdRole).toString());
    }, toolbar));
  toolbar->addAction(new MethodAction(Theme::getIcon("down"), *m_methodMoveDown,
    [this]()
    {
      if(auto* item = m_list->currentItem())
        callMethod(*m_methodMoveDown, nullptr, item->data(objectIdRole).toString());
    }, toolbar));
  left->addWidget(toolbar);

  m_list = new QListWidget(this);
  left->addWidget(m_list);

  QHBoxLayout* l = new QHBoxLayout();
  l->addLayout(left);
  m_stack = new QStackedWidget(this);
  l->addWidget(m_stack);
  setLayout(l);

  connect(m_propertyItems, &ObjectVectorProperty::valueChanged, this, &BlockInputMapWidget::itemsChanged);
  itemsChanged();
  connect(m_list, &QListWidget::currentItemChanged, this,
    [this](QListWidgetItem* current, QListWidgetItem* previous)
    {
      if(current)
        if(auto* w = m_items.value(current->data(objectIdRole).toString()))
          m_stack->setCurrentWidget(w);
    });
}

void BlockInputMapWidget::updateListItems()
{
  m_list->clear();
  for(const QString& objectId : *m_propertyItems)
  {
    QListWidgetItem* item = new QListWidgetItem(objectId);
    item->setData(objectIdRole, objectId);
    m_list->addItem(item);
  }

  // update selected row:
  if(auto* w = dynamic_cast<ObjectEditWidget*>(m_stack->currentWidget()))
  {
    const QString id = m_items.key(w);
    if(!id.isEmpty())
    {
      const int count = m_list->count();
      for(int i = 0; i < count; i++)
        if(id == m_list->item(i)->data(objectIdRole).toString())
        {
          m_list->setCurrentRow(i);
          break;
        }
    }
  }
}

void BlockInputMapWidget::itemsChanged()
{
  QList<QString> remove = m_items.keys();

  // create widget for new item(s):
  for(const QString& objectId : *m_propertyItems)
  {
    if(!m_items.contains(objectId))
    {
      ObjectEditWidget* w = new ObjectEditWidget(objectId, m_stack);
      m_stack->addWidget(w);
      m_items.insert(objectId, w);
    }
    else
      remove.removeOne(objectId);
  }

  // remove widgets:
  for(const QString& objectId : remove)
    delete m_items.take(objectId);

  updateListItems();
}
