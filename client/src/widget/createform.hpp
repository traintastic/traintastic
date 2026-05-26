/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_WIDGET_CREATEFORM_HPP
#define TRAINTASTIC_CLIENT_WIDGET_CREATEFORM_HPP

#include <initializer_list>
#include <span>
#include <QString>

class QLayout;
class QWidget;
class Object;
class InterfaceItem;

QLayout* createFormLayout(Object& object, std::initializer_list<QString> items, QWidget* parent = nullptr);
QWidget* createFormWidget(Object& object, std::initializer_list<QString> items, QWidget* parent = nullptr);

QLayout* createFormLayout(std::span<InterfaceItem*> items, QWidget* parent = nullptr);
QWidget* createFormWidget(std::span<InterfaceItem*> items, QWidget* parent = nullptr);

#endif
