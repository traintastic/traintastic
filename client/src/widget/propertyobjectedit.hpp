/**
 * client/src/widget/propertyobjectedit.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_WIDGET_PROPERTYOBJECTEDIT_HPP
#define TRAINTASTIC_CLIENT_WIDGET_PROPERTYOBJECTEDIT_HPP

#include <QWidget>

class ObjectProperty;
class QLineEdit;
class QToolButton;

class PropertyObjectEdit : public QWidget
{
  Q_OBJECT

  protected:
    ObjectProperty& m_property;
    QLineEdit* m_lineEdit;
    QToolButton* m_changeButton;
    QToolButton* m_editButton;
    int m_editObjectRequestId;
    int m_getObjectRequestId;
    QMetaObject::Connection m_idChangedConnection;

  protected:
    void connectToIdChanged();

  public:
    explicit PropertyObjectEdit(ObjectProperty& property, QWidget* parent = nullptr);
    ~PropertyObjectEdit() override;
};

#endif
