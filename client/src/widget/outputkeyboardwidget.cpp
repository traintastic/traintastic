/**
 * client/src/widget/outputkeyboardwidget.cpp
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

#include "outputkeyboardwidget.hpp"
#include <QGridLayout>
#include <QToolBar>
#include <QKeyEvent>
#include <QActionGroup>
#include <traintastic/enum/outputpairvalue.hpp>
#include <traintastic/enum/tristate.hpp>
#include <traintastic/locale/locale.hpp>
#include "ledwidget.hpp"
#include "../network/outputkeyboard.hpp"
#include "../network/abstractproperty.hpp"
#include "../network/method.hpp"
#include "../network/event.hpp"
#include "../network/callmethod.hpp"
#include "../theme/theme.hpp"

static const LEDWidget::Colors colorsPurple = {
  QColor(),
  QColor(0x20, 0x20, 0x20),
  QColor(0xAE, 0x37, 0xFF),
  QColor(0x3C, 0x00, 0x64)
};

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


constexpr LEDWidget::State triStateToState(TriState value)
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

constexpr LEDWidget::State outputPairValueToState(OutputPairValue value, OutputPairValue onValue)
{
  switch(value)
  {
    case OutputPairValue::First:
    case OutputPairValue::Second:
      return value == onValue ? LEDWidget::State::On : LEDWidget::State::Off;

    case OutputPairValue::Undefined:
      break;
  }
  return LEDWidget::State::Undefined;
}


OutputKeyboardWidget::OutputKeyboardWidget(std::shared_ptr<OutputKeyboard> object, QWidget* parent)
  : QWidget(parent)
  , m_object{std::move(object)}
  , m_channel{m_object->getProperty("channel")->toEnum<OutputChannel>()}
  , m_addressMin{m_object->getProperty("address_min")}
  , m_addressMax{m_object->getProperty("address_max")}
  , m_setOutputValue{m_object->getMethod("set_output_value")}
{
  setWindowTitle(Locale::tr("hardware:output_keyboard"));
  setWindowIcon(Theme::getIcon("output_keyboard"));

  setFocusPolicy(Qt::StrongFocus);

  auto* toolbar = new QToolBar(this);
  m_previousPage = toolbar->addAction(Theme::getIcon("previous_page"), Locale::tr("qtapp:previous"),
    [this]()
    {
      if(m_page != 0)
        setPage(m_page - 1);
    });
  m_nextPage = toolbar->addAction(Theme::getIcon("next_page"), Locale::tr("qtapp:next"),
    [this]()
    {
      setPage(m_page + 1);
    });
  toolbar->addSeparator();
  {
    auto* group = new QActionGroup(this);
    auto* action = group->addAction("1");
    action->setCheckable(true);
    action->setChecked(true);
    connect(action, &QAction::triggered, [this]() { setGroupBy(1); });
    toolbar->addAction(action);
    action = group->addAction("2");
    action->setCheckable(true);
    connect(action, &QAction::triggered, [this]() { setGroupBy(2); });
    toolbar->addAction(action);
    action = group->addAction("8");
    action->setCheckable(true);
    connect(action, &QAction::triggered, [this]() { setGroupBy(8); });
    toolbar->addAction(action);
    action = group->addAction("16");
    action->setCheckable(true);
    connect(action, &QAction::triggered, [this]() { setGroupBy(16); });
    toolbar->addAction(action);
  }

  QGridLayout* grid = new QGridLayout();

  switch(m_object->outputType())
  {
    case OutputType::Single:
      for(uint32_t i = 0; i < m_leds.size(); i++)
      {
        auto* led = new LEDWidget(colorsPurple, this);
        connect(led, &LEDWidget::clicked, this,
          [this, index=i]()
          {
            const uint32_t address = static_cast<uint32_t>(m_addressMin->toInt64() + m_page * m_leds.size() + index);
            const auto value = (qobject_cast<LEDWidget*>(sender())->state() == LEDWidget::State::On) ? TriState::False : TriState::True;
            callMethod(*m_setOutputValue, nullptr, address, value);
          });
        grid->addWidget(led, i / columns, i % columns);
        m_leds[i] = led;
      }
      break;

    case OutputType::Pair:
      for(uint32_t i = 0; i < m_leds.size(); i++)
      {
        auto* led = new LEDWidget((i & 1) ? colorsGreen : colorsRed, this);
        connect(led, &LEDWidget::clicked, this,
          [this, index=i]()
          {
            const uint32_t address = static_cast<uint32_t>(m_addressMin->toInt64()) + m_page * m_leds.size() / 2 + index / 2;
            const auto value = (index & 0x1) ? OutputPairValue::Second : OutputPairValue::First;
            callMethod(*m_setOutputValue, nullptr, address, value);
          });
        grid->addWidget(led, ((i / (2 * columns)) * 2) + (i % 2), (i / 2) % columns);
        m_leds[i] = led;
      }
      break;

    case OutputType::Aspect:
    case OutputType::ECoSState:
      assert(false); // not (yet) supported
      break;
  }

  QVBoxLayout* l = new QVBoxLayout();
  l->setContentsMargins(0, 0, 0, 0);
  l->addWidget(toolbar);
  l->addLayout(grid);
  setLayout(l);

  connect(m_object.get(), &OutputKeyboard::outputStateChanged, this,
    [this](uint32_t address)
    {
      switch(m_object->outputType())
      {
        case OutputType::Single:
          if(auto* led = getLED(address))
          {
            const auto& outputState = m_object->getOutputState(address);
            led->setEnabled(outputState.used);
            const auto value = std::holds_alternative<TriState>(outputState.value) ? std::get<TriState>(outputState.value) : TriState::Undefined;
            led->setState(triStateToState(value));
          }
          break;

        case OutputType::Pair:
        {
          auto [ledR, ledG] = getLEDs(address);
          if(ledR && ledG)
          {
            const auto& outputState = m_object->getOutputState(address);
            ledR->setEnabled(outputState.used);
            ledG->setEnabled(outputState.used);
            const auto value = std::holds_alternative<OutputPairValue>(outputState.value) ? std::get<OutputPairValue>(outputState.value) : OutputPairValue::Undefined;
            ledR->setState(outputPairValueToState(value, OutputPairValue::First));
            ledG->setState(outputPairValueToState(value, OutputPairValue::Second));
          }
          break;
        }
        case OutputType::Aspect: /*[[unlikely]]*/
        case OutputType::ECoSState: /*[[unlikely]]*/
          assert(false);
          break;
      }
    });

  updateLEDs();
}

void OutputKeyboardWidget::keyReleaseEvent(QKeyEvent* event)
{
  switch(event->key())
  {
    case Qt::Key_Left:
      m_previousPage->trigger();
      return;

    case Qt::Key_Right:
      m_nextPage->trigger();
      return;

    case Qt::Key_Home:
      setPage(0);
      return;

    case Qt::Key_End:
      setPage(pageCount() - 1);
      return;
  }
  QWidget::keyReleaseEvent(event);
}

uint32_t OutputKeyboardWidget::pageCount() const
{
  auto leds = m_addressMax->toInt64() - m_addressMin->toInt64() + 1;
  if(m_object->outputType() == OutputType::Pair)
  {
    leds *= 2;
  }
  return static_cast<uint32_t>(leds + m_leds.size() - 1) / m_leds.size();
}

void OutputKeyboardWidget::setPage(uint32_t value)
{
  if(m_page != value && value < pageCount())
  {
    m_page = value;
    updateLEDs();
  }
}

void OutputKeyboardWidget::setGroupBy(uint32_t value)
{
  assert(value > 0);
  if(m_groupBy != value)
  {
    m_groupBy = value;
    updateLEDs();
  }
}

LEDWidget* OutputKeyboardWidget::getLED(uint32_t address)
{
  assert(m_object->outputType() == OutputType::Single);

  const uint32_t first = static_cast<uint32_t>(m_addressMin->toInt64()) + m_page * m_leds.size();

  if(address >= first && (address - first) < m_leds.size())
    return m_leds[address - first];

  return nullptr;
}

std::pair<LEDWidget*, LEDWidget*> OutputKeyboardWidget::getLEDs(uint32_t address)
{
  assert(m_object->outputType() == OutputType::Pair);

  const uint32_t first = static_cast<uint32_t>(m_addressMin->toInt64()) + m_page * m_leds.size() / 2;

  if(address >= first && (address - first) < m_leds.size())
    return {m_leds[(address - first) * 2], m_leds[(address - first) * 2 + 1]};

  return {nullptr, nullptr};
}

void OutputKeyboardWidget::updateLEDs()
{
  m_previousPage->setEnabled(m_page != 0);
  m_nextPage->setEnabled(m_page != pageCount() - 1);

  const uint32_t addressMin = static_cast<uint32_t>(m_addressMin->toInt64());
  const uint32_t addressMax = static_cast<uint32_t>(m_addressMax->toInt64());

  switch(m_object->outputType())
  {
    case OutputType::Single:
    {
      uint32_t address = addressMin + m_page * m_leds.size();
      for(auto* led : m_leds)
      {
        const auto& outputState = m_object->getOutputState(address);
        led->setEnabled(outputState.used);
        led->setState(
          std::holds_alternative<TriState>(outputState.value)
            ? triStateToState(std::get<TriState>(outputState.value))
            : LEDWidget::State::Undefined);
        if(m_groupBy > 1)
          led->setText(QString("%1.%2").arg(1 + (address - addressMin) / m_groupBy).arg(addressMin + (address - addressMin) % m_groupBy));
        else
          led->setText(QString::number(address));
        led->setVisible(address <= addressMax);
        address++;
      }
      break;
    }
    case OutputType::Pair:
    {
      uint32_t address = addressMin + m_page * m_leds.size() / 2;
      bool second = false;
      for(auto* led : m_leds)
      {
        const auto& outputState = m_object->getOutputState(address);
        led->setEnabled(outputState.used);
        led->setState(
          std::holds_alternative<OutputPairValue>(outputState.value)
            ? outputPairValueToState(std::get<OutputPairValue>(outputState.value), second ? OutputPairValue::Second : OutputPairValue::First)
            : LEDWidget::State::Undefined);
        if(m_groupBy > 1)
          led->setText(QString("%1.%2").arg(1 + (address - addressMin) / m_groupBy).arg(addressMin + (address - addressMin) % m_groupBy));
        else
          led->setText(QString::number(address));
        led->setVisible(address <= addressMax);
        if(second)
        {
          address++;
        }
        second = !second;
      }
      break;
    }
    case OutputType::Aspect: /*[[unlikely]]*/
    case OutputType::ECoSState: /*[[unlikely]]*/
      assert(false);
      break;
  }
}
