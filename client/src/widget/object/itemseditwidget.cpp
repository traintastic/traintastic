/**
 * client/src/widget/object/itemseditwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2025 Reinder Feenstra
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

#include "itemseditwidget.hpp"
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

ItemsEditWidget::ItemsEditWidget(const ObjectPtr& object, QWidget* parent) :
  AbstractEditWidget(object, parent),
  m_methodCreate{nullptr},
  m_methodDelete{nullptr},
  m_methodMoveUp{nullptr},
  m_methodMoveDown{nullptr},
  m_actionDelete{nullptr},
  m_actionMoveUp{nullptr},
  m_actionMoveDown{nullptr},
  m_stack{nullptr},
  m_list{nullptr}
{
  buildForm();
}

ItemsEditWidget::ItemsEditWidget(const QString& id, QWidget* parent) :
  AbstractEditWidget(id, parent),
  m_methodCreate{nullptr},
  m_methodDelete{nullptr},
  m_methodMoveUp{nullptr},
  m_methodMoveDown{nullptr},
  m_actionDelete{nullptr},
  m_actionMoveUp{nullptr},
  m_actionMoveDown{nullptr},
  m_stack{nullptr},
  m_list{nullptr}
{
}

void ItemsEditWidget::buildForm()
{
  setObjectWindowTitle();
  Theme::setWindowIcon(*this, m_object->classId());

  m_propertyItems = dynamic_cast<ObjectVectorProperty*>(m_object->getVectorProperty("items"));
  m_methodCreate = m_object->getMethod("create");
  m_methodDelete = m_object->getMethod("delete");
  m_methodMoveUp = m_object->getMethod("move_up");
  m_methodMoveDown = m_object->getMethod("move_down");

  if(Q_UNLIKELY(!(m_propertyItems)))
    return;

  QVBoxLayout* left = new QVBoxLayout();

  QToolBar* toolbar = new QToolBar(this);

  if(m_methodCreate)
    toolbar->addAction(new MethodAction(Theme::getIcon("add"), *m_methodCreate, toolbar));

  if(m_methodDelete)
  {
    m_actionDelete = new MethodAction(Theme::getIcon("delete"), *m_methodDelete,
      [this]()
      {
        if(auto* item = m_list->currentItem())
          callMethod(*m_methodDelete, nullptr, item->data(objectIdRole).toString());
      }, toolbar);
    toolbar->addAction(m_actionDelete);
  }

  if(m_methodMoveUp)
  {
    m_actionMoveUp = new MethodAction(Theme::getIcon("up"), *m_methodMoveUp,
      [this]()
      {
        if(auto* item = m_list->currentItem())
          callMethod(*m_methodMoveUp, nullptr, item->data(objectIdRole).toString());
      }, toolbar);
    toolbar->addAction(m_actionMoveUp);
  }

  if(m_methodMoveDown)
  {
    m_actionMoveDown = new MethodAction(Theme::getIcon("down"), *m_methodMoveDown,
      [this]()
      {
        if(auto* item = m_list->currentItem())
          callMethod(*m_methodMoveDown, nullptr, item->data(objectIdRole).toString());
      }, toolbar);
    toolbar->addAction(m_actionMoveDown);
  }
  left->addWidget(toolbar);

  m_list = new QListWidget(this);
  left->addWidget(m_list);

  QHBoxLayout* l = new QHBoxLayout();
  l->addLayout(left);
  m_stack = new QStackedWidget(this);
  l->addWidget(m_stack);
  setLayout(l);

  connect(m_propertyItems, &ObjectVectorProperty::valueChanged, this, &ItemsEditWidget::itemsChanged);
  itemsChanged();
  connect(m_list, &QListWidget::currentItemChanged, this,
    [this](QListWidgetItem* current, QListWidgetItem* /*previous*/)
    {
      if(current)
        if(auto* w = m_items.value(current->data(objectIdRole).toString()))
          m_stack->setCurrentWidget(w);

      if(m_actionMoveUp)
        m_actionMoveUp->setForceDisabled(!current || current == m_list->item(0));

      if(m_actionMoveDown)
        m_actionMoveDown->setForceDisabled(!current || current == m_list->item(m_list->count() - 1));
    });
}

void ItemsEditWidget::updateListItems()
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

  if(m_actionDelete)
    m_actionDelete->setForceDisabled(m_list->count() == 0);
}

void ItemsEditWidget::itemsChanged()
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
