/**
 * client/src/widget/inputmonitorwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#include "inputmonitorwidget.hpp"
#include <QGridLayout>
#include "ledwidget.hpp"
#include "../network/inputmonitor.hpp"

constexpr LEDWidget::State toState(TriState value)
{
  switch(value)
  {
    case TriState::False:
      return LEDWidget::State::Off;
    case TriState::True:
      return LEDWidget::State::On;
    case TriState::Undefined:
      break;
  }
  return LEDWidget::State::Undefined;
}


InputMonitorWidget::InputMonitorWidget(std::shared_ptr<InputMonitor> object, QWidget* parent) :
  QWidget(parent),
  m_object{std::move(object)}
{
  QGridLayout* grid = new QGridLayout();

  for(int i = 1; i <= 128; i++)
  {
    auto* led = new LEDWidget(this);
    led->setEnabled(false);
    led->setText(QString::number(i));
    grid->addWidget(led, (i - 1) / 16, (i - 1) % 16);
    m_leds.emplace(i, led);
  }

  setLayout(grid);

  connect(m_object.get(), &InputMonitor::inputIdChanged, this,
    [this](uint32_t address, QString id)
    {
      if(auto* led = getLED(address))
        led->setEnabled(!id.isEmpty());
    });
  connect(m_object.get(), &InputMonitor::inputValueChanged, this,
    [this](uint32_t address, TriState value)
    {
      if(auto* led = getLED(address))
        led->setState(toState(value));
    });

  m_object->refresh();
}

LEDWidget* InputMonitorWidget::getLED(uint32_t address)
{
  auto it = m_leds.find(address);
  return it != m_leds.end() ? it->second : nullptr;
}
