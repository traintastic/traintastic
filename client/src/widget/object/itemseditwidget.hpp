/**
 * client/src/widget/object/itemseditwidget.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_WIDGET_OBJECT_ITEMSEDITWIDGET_HPP
#define TRAINTASTIC_CLIENT_WIDGET_OBJECT_ITEMSEDITWIDGET_HPP

#include "abstracteditwidget.hpp"
#include <QMap>

class QStackedWidget;
class QListWidget;
class ObjectEditWidget;
class ObjectVectorProperty;
class Method;
class MethodAction;

class ItemsEditWidget : public AbstractEditWidget
{
  Q_OBJECT

  private:
    ObjectVectorProperty* m_propertyItems;
    Method* m_methodCreate;
    Method* m_methodDelete;
    Method* m_methodMoveUp;
    Method* m_methodMoveDown;
    MethodAction* m_actionDelete;
    MethodAction* m_actionMoveUp;
    MethodAction* m_actionMoveDown;
    QStackedWidget* m_stack;
    QListWidget* m_list;
    QMap<QString, ObjectEditWidget*> m_items;

    void updateListItems();

  private slots:
    void itemsChanged();

  protected:
    void buildForm() final;

  public:
    explicit ItemsEditWidget(const ObjectPtr& object, QWidget* parent = nullptr);
    explicit ItemsEditWidget(const QString& id, QWidget* parent = nullptr);
};

#endif
