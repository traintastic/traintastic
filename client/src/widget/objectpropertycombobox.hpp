/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2024-2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_WIDGET_OBJECTPROPERTYCOMBOBOX_HPP
#define TRAINTASTIC_CLIENT_WIDGET_OBJECTPROPERTYCOMBOBOX_HPP

#include <QComboBox>
#include "../network/objectptr.hpp"

class ObjectProperty;

class ObjectPropertyComboBox : public QComboBox
{
  Q_OBJECT

  protected:
    ObjectProperty& m_property;
    ObjectPtr m_objectList;
    int m_requestId;
    bool m_modelUpdating = false;

    void updateValue();

  public:
    ObjectPropertyComboBox(ObjectProperty& property, QWidget* parent = nullptr);
    ~ObjectPropertyComboBox() override;
};

#endif
