/**
 * client/src/widget/objecteditwidget.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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

#ifndef CLIENT_WIDGET_OBJECTEDITWIDGET_HPP
#define CLIENT_WIDGET_OBJECTEDITWIDGET_HPP

#include <QWidget>
#include "../network/objectptr.hpp"

class ObjectEditWidget : public QWidget
{
  Q_OBJECT

  protected:
    const QString m_id;
    int m_requestId;
    ObjectPtr m_object;

    virtual void buildForm();

  public:
    explicit ObjectEditWidget(const QString& id, QWidget* parent = nullptr);
    ~ObjectEditWidget() override;
};

#endif
