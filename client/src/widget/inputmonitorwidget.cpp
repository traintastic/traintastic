/**
 * client/src/widget/inputmonitorwidget.cpp
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

#include "inputmonitorwidget.hpp"
#include <QGridLayout>
#include <traintastic/locale/locale.hpp>
#include "ledwidget.hpp"
#include "../network/inputmonitor.hpp"
#include "../network/abstractproperty.hpp"

static const LEDWidget::Colors colors = {
  QColor(),
  QColor(0x20, 0x20, 0x20),
  QColor(0x00, 0xBF, 0xFF),
  Qt::black
};

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


InputMonitorWidget::InputMonitorWidget(std::shared_ptr<InputMonitor> object, QWidget* parent)
  : QWidget(parent)
  , m_object{std::move(object)}
  , m_addressMin{m_object->getProperty("address_min")}
  , m_addressMax{m_object->getProperty("address_max")}
{
  setWindowTitle(Locale::tr("hardware:input_monitor"));

  QGridLayout* grid = new QGridLayout();

  const uint32_t first = static_cast<uint32_t>(m_addressMin->toInt64());
  for(uint32_t i = first; i < first + 128; i++)
  {
    auto* led = new LEDWidget(colors, this);
    led->setEnabled(false);
    led->setText(QString::number(i));
    grid->addWidget(led, (i - first) / 16, (i - first) % 16);
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
