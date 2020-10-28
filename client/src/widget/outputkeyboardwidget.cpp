/**
 * client/src/widget/outputkeyboardwidget.cpp
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

#include "outputkeyboardwidget.hpp"
#include <QGridLayout>
#include "ledwidget.hpp"
#include "../network/outputkeyboard.hpp"

static const LEDWidget::Colors colorsGreen = {
  QColor(),
  QColor(0x20, 0x20, 0x20),
  QColor(0x20, 0xFF, 0x20),
  QColor(0x20, 0x80, 0x20)
};

static const LEDWidget::Colors colorsRed = {
  QColor(),
  QColor(0x20, 0x20, 0x20),
  QColor(0xFF, 0x20, 0x20),
  QColor(0x80, 0x20, 0x20)
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


OutputKeyboardWidget::OutputKeyboardWidget(std::shared_ptr<OutputKeyboard> object, QWidget* parent) :
  QWidget(parent),
  m_object{std::move(object)}
{
  QGridLayout* grid = new QGridLayout();

  for(int i = 0; i < 128; i++)
  {
    auto* led = new LEDWidget((i & 1) ? colorsGreen : colorsRed, this);
    led->setEnabled(false);
    led->setText(QString::number(1 + i));
    connect(led, &LEDWidget::clicked, this,
      [this, address=1 + i]()
      {
        m_object->outputSetValue(address, qobject_cast<LEDWidget*>(sender())->state() == LEDWidget::State::Off);
      });
    grid->addWidget(led, ((i / 32) * 2) + (i % 2), (i / 2) % 16);
    m_leds.emplace(1 + i, led);
  }

  setLayout(grid);

  connect(m_object.get(), &OutputKeyboard::outputIdChanged, this,
    [this](uint32_t address, QString id)
    {
      if(auto* led = getLED(address))
        led->setEnabled(!id.isEmpty());
    });
  connect(m_object.get(), &OutputKeyboard::outputValueChanged, this,
    [this](uint32_t address, TriState value)
    {
      if(auto* led = getLED(address))
        led->setState(toState(value));
    });

  m_object->refresh();
}

LEDWidget* OutputKeyboardWidget::getLED(uint32_t address)
{
  auto it = m_leds.find(address);
  return it != m_leds.end() ? it->second : nullptr;
}
