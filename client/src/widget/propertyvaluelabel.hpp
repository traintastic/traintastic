/**
 * client/src/widget/propertyvaluelabel.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020 Reinder Feenstra
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

#ifndef CLIENT_WIDGET_PROPERTYVALUELABEL_HPP
#define CLIENT_WIDGET_PROPERTYVALUELABEL_HPP

#include <QLabel>

class Property;

class PropertyValueLabel : public QLabel
{
  protected:
    Property& m_property;

  public:
    PropertyValueLabel(Property& property, QWidget* parent = nullptr);
};

#endif
