/**
 * client/src/widget/inputmonitorwidget.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_WIDGET_INPUTMONITORWIDGET_HPP
#define TRAINTASTIC_CLIENT_WIDGET_INPUTMONITORWIDGET_HPP

#include <QWidget>
#include <memory>
#include <unordered_map>

class InputMonitor;
class AbstractProperty;
class LEDWidget;

class InputMonitorWidget : public QWidget
{
  protected:
    std::shared_ptr<InputMonitor> m_object;
    AbstractProperty* m_addressMin;
    AbstractProperty* m_addressMax;
    std::unordered_map<uint32_t, LEDWidget*> m_leds;

    LEDWidget* getLED(uint32_t address);

  public:
    InputMonitorWidget(std::shared_ptr<InputMonitor> object, QWidget* parent = nullptr);
};

#endif
