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
#include <QToolBar>
#include <QKeyEvent>
#include <QActionGroup>
#include <traintastic/locale/locale.hpp>
#include "ledwidget.hpp"
#include "../network/connection.hpp"
#include "../network/inputmonitor.hpp"
#include "../network/abstractproperty.hpp"
#include "../theme/theme.hpp"

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
  setWindowIcon(Theme::getIcon("input_monitor"));

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

  for(uint32_t i = 0; i < m_leds.size(); i++)
  {
    auto* led = new LEDWidget(colors, this);
    connect(led, &LEDWidget::clicked, this,
      [this, index=i]()
      {
        if(m_object->connection()->world()->getPropertyValueBool("simulation", false))
        {
          const uint32_t address = static_cast<uint32_t>(m_addressMin->toInt64() + m_page * m_leds.size() + index);
          m_object->simulateInputChange(address);
        }
      });
    grid->addWidget(led, i / columns, i % columns);
    m_leds[i] = led;
  }

  QVBoxLayout* l = new QVBoxLayout();
  l->setContentsMargins(0, 0, 0, 0);
  l->addWidget(toolbar);
  l->addLayout(grid);
  setLayout(l);

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

  updateLEDs();
}

void InputMonitorWidget::keyReleaseEvent(QKeyEvent* event)
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

uint32_t InputMonitorWidget::pageCount() const
{
  return static_cast<uint32_t>(m_addressMax->toInt64() - m_addressMin->toInt64() + m_leds.size()) / static_cast<uint32_t>(m_leds.size());
}

void InputMonitorWidget::setPage(uint32_t value)
{
  if(m_page != value && value < pageCount())
  {
    m_page = value;
    updateLEDs();
  }
}

void InputMonitorWidget::setGroupBy(uint32_t value)
{
  assert(value > 0);
  if(m_groupBy != value)
  {
    m_groupBy = value;
    updateLEDs();
  }
}

LEDWidget* InputMonitorWidget::getLED(uint32_t address)
{
  const uint32_t first = static_cast<uint32_t>(m_addressMin->toInt64()) + m_page * static_cast<uint32_t>(m_leds.size());

  if(address >= first && (address - first) < m_leds.size())
    return m_leds[address - first];

  return nullptr;
}

void InputMonitorWidget::updateLEDs()
{
  m_previousPage->setEnabled(m_page != 0);
  m_nextPage->setEnabled(m_page != pageCount() - 1);

  const uint32_t addressMin = static_cast<uint32_t>(m_addressMin->toInt64());
  const uint32_t addressMax = static_cast<uint32_t>(m_addressMax->toInt64());
  uint32_t address = addressMin + m_page * static_cast<uint32_t>(m_leds.size());

  for(auto* led : m_leds)
  {
    led->setEnabled(!m_object->getInputId(address).isEmpty());
    led->setState(toState(m_object->getInputState(address)));
    if(m_groupBy > 1)
      led->setText(QString("%1.%2").arg(1 + (address - addressMin) / m_groupBy).arg(addressMin + (address - addressMin) % m_groupBy));
    else
      led->setText(QString::number(address));
    led->setVisible(address <= addressMax);
    address++;
  }
}
