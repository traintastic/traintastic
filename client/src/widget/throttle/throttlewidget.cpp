/**
 * client/src/widget/throttlewidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2025 Reinder Feenstra
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

#include "throttlewidget.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QPainter>
#include <traintastic/enum/decoderfunctionfunction.hpp>
#include <traintastic/enum/speedunit.hpp>
#include "../../network/connection.hpp"
#include "../../network/object.hpp"
#include "../../network/objectproperty.hpp"
#include "../../network/abstractproperty.hpp"
#include "../../network/objectvectorproperty.hpp"
#include "../../network/method.hpp"
#include "../../network/unitproperty.hpp"
#include "../../network/error.hpp"
#include "throttledirectionbutton.hpp"
#include "throttlefunctionbutton.hpp"
#include "throttlestopbutton.hpp"
#include "speedometerwidget.hpp"
#include "sliderwidget.hpp"

ThrottleWidget::ThrottleWidget(ObjectPtr object, QWidget* parent)
  : QWidget(parent)
  , m_object{std::move(object)}
  , m_functionsRequestId{Connection::invalidRequestId}
  , m_toggleDirection{m_object->getMethod("toggle_direction")}
  , m_nameLabel{new QLabel("", this)}
  , m_functionGrid{new QGridLayout()}
  , m_speedoMeter{new SpeedoMeterWidget(this)}
  , m_speedSlider{new SliderWidget(Qt::Vertical, this)}
  , m_stopButton{new ThrottleStopButton(m_object, this)}
  , m_reverseButton{new ThrottleDirectionButton(m_object, Direction::Reverse, this)}
  , m_forwardButton{new ThrottleDirectionButton(m_object, Direction::Forward, this)}
{
  setFocusPolicy(Qt::StrongFocus);

  if(auto* name = m_object->getProperty("name"))
  {
    setWindowTitle(name->toString());
    connect(name, &AbstractProperty::valueChangedString, this, &ThrottleWidget::setWindowTitle);
    m_nameLabel->setText(name->toString());
    connect(name, &AbstractProperty::valueChangedString, m_nameLabel, &QLabel::setText);
  }

  if((m_speed = m_object->getUnitProperty("speed")) &&
    (m_throttleSpeed = m_object->getUnitProperty("throttle_speed"))) // train
  {
    m_speedoMeter->setUnit("km/h");
    m_speedoMeter->setSpeedMax(m_speed->getAttributeDouble(AttributeName::Max, 0));
    m_speedoMeter->setSpeed(m_speed->toDouble());
    m_speedoMeter->setSpeedTarget(m_throttleSpeed->toDouble());

    m_speedSlider->setMaximum(m_speed->getAttributeDouble(AttributeName::Max, 0));
    m_speedSlider->setValue(m_throttleSpeed->toDouble());

    connect(m_speed, &AbstractProperty::valueChangedDouble, this,
      [this](double value)
      {
        m_speedoMeter->setSpeed(value);
      });
    connect(m_speed, &InterfaceItem::attributeChanged, this,
      [this](AttributeName name, const QVariant& value)
      {
        switch(name)
        {
          case AttributeName::Max:
            m_speedoMeter->setSpeedMax(value.toDouble());
            m_speedSlider->setMaximum(value.toDouble());
            break;

          default:
            break;
        }
      });
    connect(m_throttleSpeed, &AbstractProperty::valueChangedDouble, this,
      [this](double value)
      {
        m_speedoMeter->setSpeedTarget(value);
        m_speedSlider->setValue(value);
      });
    connect(m_speedSlider, &SliderWidget::valueChanged, this,
      [this](float value)
      {
        m_throttleSpeed->setValueDouble(value);
      });
  }
  else if((m_throttle = m_object->getProperty("throttle"))) // decoder
  {
    m_speedoMeter->setUnit("%");
    m_speedoMeter->setSpeedMax(m_throttle->getAttributeDouble(AttributeName::Max, 0) * 100);
    m_speedoMeter->setSpeed(m_throttle->toDouble() * 100);

    m_speedSlider->setMaximum(m_throttle->getAttributeDouble(AttributeName::Max, 0) * 100);
    m_speedSlider->setValue(m_throttle->toDouble() * 100);

    connect(m_throttle, &AbstractProperty::valueChangedDouble, this,
      [this](double value)
      {
        m_speedoMeter->setSpeed(value * 100);
        m_speedSlider->setValue(value * 100);
      });
    connect(m_throttle, &InterfaceItem::attributeChanged, this,
      [this](AttributeName name, const QVariant& value)
      {
        switch(name)
        {
          case AttributeName::Max:
            m_speedoMeter->setSpeedMax(value.toDouble() * 100);
            m_speedSlider->setMaximum(value.toDouble() * 100);
            break;

          default:
            break;
        }
      });
    connect(m_speedSlider, &SliderWidget::valueChanged, this,
      [this](float value)
      {
        m_throttle->setValueDouble(value);
      });
  }

  if((m_emergencyStop = m_object->getProperty("emergency_stop")))
  {
    m_speedoMeter->setEStop(m_emergencyStop->toBool());
    connect(m_emergencyStop, &AbstractProperty::valueChangedBool, m_speedoMeter, &SpeedoMeterWidget::setEStop);
  }

  if(auto* p = dynamic_cast<ObjectProperty*>(m_object->getProperty("functions")))
  {
    m_functionsRequestId = p->getObject(
      [this](const ObjectPtr& functions, std::optional<const Error> /*error*/)
      {
        m_functionsRequestId = Connection::invalidRequestId;

        if(!functions)
          return;

        if(auto* items = dynamic_cast<ObjectVectorProperty*>(functions->getVectorProperty("items")))
        {
          int i = 0;
          for(const QString& id : *items)
          {
            const int functionRequestId = functions->connection()->getObject(id,
              [this, i](const ObjectPtr& function, std::optional<const Error> /*error*/)
              {
                m_functionRequestIds.erase(i);

                if(function)
                {
                  static constexpr int buttonsPerRow = 6;
                  auto* btn = new ThrottleFunctionButton(function, this);
                  m_functionGrid->addWidget(btn, i / buttonsPerRow, i % buttonsPerRow);
                  m_functionButtons.push_back(btn);
                }
              });
            m_functionRequestIds.emplace(i, functionRequestId);
            i++;
          }
        }
      });
  }

  auto* left = new QVBoxLayout();
  QFont font = m_nameLabel->font();
  font.setBold(true);
  m_nameLabel->setFont(font);
  left->addWidget(m_nameLabel);
  left->addStretch();
  left->addLayout(m_functionGrid);

  auto* right = new QVBoxLayout();
  right->addWidget(m_speedoMeter);
  auto* h = new QHBoxLayout();
  h->addStretch();
  h->addWidget(m_reverseButton);
  h->addWidget(m_stopButton);
  h->addWidget(m_forwardButton);
  h->addStretch();
  right->addLayout(h);
  auto* l = new QHBoxLayout();
  l->addLayout(left);
  l->addLayout(right);
  l->addWidget(m_speedSlider);
  setLayout(l);
}

ThrottleWidget::~ThrottleWidget()
{
  const auto& c = m_object->connection();
  if(m_functionsRequestId != Connection::invalidRequestId)
    c->cancelRequest(m_functionsRequestId);
  for(const auto& it : m_functionRequestIds)
    c->cancelRequest(it.second);
}

void ThrottleWidget::keyPressEvent(QKeyEvent* event)
{
  if(ThrottleFunctionButton* btn = getFunctionButton(*event))
  {
    btn->press();
  }
}

void ThrottleWidget::keyReleaseEvent(QKeyEvent* event)
{
  if(ThrottleFunctionButton* btn = getFunctionButton(*event))
  {
    btn->release();
  }
  else
  {
    switch(event->key())
    {
      case Qt::Key_Space:
        m_stopButton->click();
        return;

      case Qt::Key_Return:
      case Qt::Key_Enter:
        if(Q_LIKELY(m_toggleDirection))
          m_toggleDirection->call();
        return;

      case Qt::Key_Left:
        m_reverseButton->click();
        return;

      case Qt::Key_Right:
        m_forwardButton->click();
        return;

      case Qt::Key_Up:
        changeSpeed(true);
        return;

      case Qt::Key_Down:
        changeSpeed(false);
        return;
    }
  }
  QWidget::keyReleaseEvent(event);
}

void ThrottleWidget::wheelEvent(QWheelEvent* event)
{
  if(event->angleDelta().y() > 0)
  {
    changeSpeed(true);
  }
  else if(event->angleDelta().y() < 0)
  {
    changeSpeed(false);
  }
}

void ThrottleWidget::paintEvent(QPaintEvent*)
{
  const QColor backgroundColor{0x10, 0x10, 0x10};
  QPainter painter(this);
  painter.fillRect(rect(), backgroundColor);
}

void ThrottleWidget::changeSpeed(bool up)
{
  if(up && m_emergencyStop && m_emergencyStop->toBool())
    return;

  if(m_throttleSpeed)
  {
    double throttle = m_throttleSpeed->toDouble();
    double step = 0;
    switch(m_throttleSpeed->unit<SpeedUnit>())
    {
      case SpeedUnit::KiloMeterPerHour:
        step = ((throttle < 40) || (!up && qFuzzyCompare(throttle, 40))) ? 2 : 5;
        break;

      case SpeedUnit::MilePerHour:
        step = ((throttle < 30) || (!up && qFuzzyCompare(throttle, 30))) ? 2 : 5;
        break;

      case SpeedUnit::MeterPerSecond:
        step = ((throttle < 10) || (!up && qFuzzyCompare(throttle, 10))) ? 0.5 : 1;
        break;
    }
    throttle += up ? step : -step;
    m_throttleSpeed->setValueDouble(std::clamp(throttle, m_throttleSpeed->getAttributeDouble(AttributeName::Min, 0), m_throttleSpeed->getAttributeDouble(AttributeName::Max, 1)));
  }
  else if(m_throttle)
  {
    static constexpr double throttleStep = 0.02; // 2%
    static constexpr double throttleStepHalf = throttleStep / 2;

    double throttle = m_throttle->toDouble();
    if(throttle < 0.2)
    {
      throttle += up ? throttleStepHalf : -throttleStepHalf;
    }
    else
    {
      throttle += up ? throttleStep : -throttleStep;
    }
    m_throttle->setValueDouble(std::clamp(throttle, m_throttle->getAttributeDouble(AttributeName::Min, 0), m_throttle->getAttributeDouble(AttributeName::Max, 1)));
  }
}

ThrottleFunctionButton* ThrottleWidget::getFunctionButton(int number)
{
  auto it = std::find_if(m_functionButtons.begin(), m_functionButtons.end(),
    [number](const auto* btn)
    {
      return btn->number() == number;
    });
  return (it != m_functionButtons.end()) ? *it : nullptr;
}

ThrottleFunctionButton* ThrottleWidget::getFunctionButton(DecoderFunctionFunction function)
{
  auto it = std::find_if(m_functionButtons.begin(), m_functionButtons.end(),
    [function](const auto* btn)
    {
      return btn->function() == function;
    });
  return (it != m_functionButtons.end()) ? *it : nullptr;
}

ThrottleFunctionButton* ThrottleWidget::getFunctionButton(const QKeyEvent& event)
{
  if(event.key() >= Qt::Key_0 && event.key() <= Qt::Key_9)
  {
    int n = event.key() - Qt::Key_0;

    switch(event.modifiers() & Qt::ControlModifier)
    {
      case Qt::NoModifier:
        break;

      case Qt::ControlModifier:
        n += 10;
        break;

      case Qt::AltModifier:
        n += 20;
        break;

      default:
        return nullptr;
    }

    return getFunctionButton(n);
  }

  switch(event.key())
  {
    case Qt::Key_L:
      return getFunctionButton(DecoderFunctionFunction::Light);
  }

  return nullptr;
}
