/**
 * client/src/widget/outputkeyboardwidget.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2022,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_WIDGET_OUTPUTKEYBOARDWIDGET_HPP
#define TRAINTASTIC_CLIENT_WIDGET_OUTPUTKEYBOARDWIDGET_HPP

#include <QWidget>
#include <memory>
#include <array>
#include <traintastic/enum/outputchannel.hpp>

class OutputKeyboard;
class LEDWidget;
class AbstractProperty;
class Method;
class Event;

class OutputKeyboardWidget : public QWidget
{
  private:
    static constexpr uint32_t columns = 16;
    static constexpr uint32_t rows = 8;

    std::shared_ptr<OutputKeyboard> m_object;
    const OutputChannel m_channel;
    AbstractProperty* m_addressMin;
    AbstractProperty* m_addressMax;
    Method* m_setOutputValue;
    std::array<LEDWidget*, columns * rows> m_leds;
    QAction* m_previousPage;
    QAction* m_nextPage;
    uint32_t m_page = 0;
    uint32_t m_groupBy = 1;

    uint32_t pageCount() const;
    void setPage(uint32_t value);

    void setGroupBy(uint32_t value);

    LEDWidget* getLED(uint32_t address);
    std::pair<LEDWidget*, LEDWidget*> getLEDs(uint32_t address);
    void updateLEDs();

  protected:
    void keyReleaseEvent(QKeyEvent* event) final;

  public:
    OutputKeyboardWidget(std::shared_ptr<OutputKeyboard> object, QWidget* parent = nullptr);
};

#endif
