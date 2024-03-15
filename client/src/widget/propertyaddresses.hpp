/**
 * client/src/widget/propertyaddresses.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_WIDGET_PROPERTYADDRESSES_HPP
#define TRAINTASTIC_CLIENT_WIDGET_PROPERTYADDRESSES_HPP

#include <QWidget>

class AbstractVectorProperty;
class Method;
class QSpinBox;
class QToolButton;

class PropertyAddresses : public QWidget
{
  Q_OBJECT

  private:
    AbstractVectorProperty& m_property;
    std::vector<QSpinBox*> m_addresses;
    int m_size = 0;

    void updateSize();

  public:
    PropertyAddresses(AbstractVectorProperty& property, Method* addMethod, Method* removeMethod, QWidget* parent = nullptr);
};

#endif
