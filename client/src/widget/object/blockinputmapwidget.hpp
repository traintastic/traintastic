/**
 * client/src/widget/object/blockinputmapwidget.hpp
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


#ifndef TRAINTASTIC_CLIENT_WIDGET_OBJECT_BLOCKINPUTMAPWIDGET_HPP
#define TRAINTASTIC_CLIENT_WIDGET_OBJECT_BLOCKINPUTMAPWIDGET_HPP

#include "abstracteditwidget.hpp"
#include <QMap>

class QStackedWidget;
class QListWidget;
class ObjectEditWidget;
class ObjectVectorProperty;
class Method;

class BlockInputMapWidget final : public AbstractEditWidget
{
  Q_OBJECT

  private:
    ObjectVectorProperty* m_propertyItems;
    Method* m_methodAdd;
    Method* m_methodRemove;
    Method* m_methodMoveUp;
    Method* m_methodMoveDown;
    QStackedWidget* m_stack;
    QListWidget* m_list;
    QMap<QString, ObjectEditWidget*> m_items;

    void updateListItems();

  private slots:
    void itemsChanged();

  protected:
    void buildForm() final;

  public:
    explicit BlockInputMapWidget(const ObjectPtr& object, QWidget* parent = nullptr);
    explicit BlockInputMapWidget(const QString& id, QWidget* parent = nullptr);
};

#endif
