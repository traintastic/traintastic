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
#include <QGuiApplication>
#include <traintastic/enum/decoderfunctionfunction.hpp>
#include <traintastic/enum/decoderfunctiontype.hpp>
#include <traintastic/enum/direction.hpp>
#include <traintastic/enum/speedunit.hpp>
#include <traintastic/locale/locale.hpp>
#include "../../network/callmethod.hpp"
#include "../../network/connection.hpp"
#include "../../network/object.hpp"
#include "../../network/object.tpp"
#include "../../network/objectproperty.hpp"
#include "../../network/abstractproperty.hpp"
#include "../../network/objectvectorproperty.hpp"
#include "../../network/method.hpp"
#include "../../network/unitproperty.hpp"
#include "../../network/error.hpp"
#include "throttlefunctionbutton.hpp"
#include "throttlebutton.hpp"
#include "throttlestyle.hpp"
#include "speedometerwidget.hpp"
#include "sliderwidget.hpp"

namespace
{

constexpr auto immidiateSpeedModifier = Qt::ControlModifier;
constexpr auto releaseWithoutStopModifier = Qt::ShiftModifier;

static const ObjectPtr nullObject;

}

ThrottleWidget::ThrottleWidget(ObjectPtr train, QWidget* parent)
  : QWidget(parent)
  , m_train{std::move(train)}
  , m_trainHasThrottle{m_train->getProperty("has_throttle")}
  , m_trainThrottleName{m_train->getProperty("throttle_name")}
  , m_trainDirection{m_train->getProperty("direction")}
  , m_trainSpeed{m_train->getUnitProperty("speed")}
  , m_trainTargetSpeed{m_train->getUnitProperty("throttle_speed")}
  , m_trainIsStopped{m_train->getProperty("is_stopped")}
  , m_trainEmergencyStop{m_train->getProperty("emergency_stop")}
  , m_vehiclesRequestId{Connection::invalidRequestId}
  , m_nameLabel{new QLabel("", this)}
  , m_speedoMeter{new SpeedoMeterWidget(this)}
  , m_speedSlider{new SliderWidget(Qt::Vertical, this)}
  , m_stopButton{new ThrottleButton("0", this)}
  , m_reverseButton{new ThrottleButton("<", this)}
  , m_forwardButton{new ThrottleButton(">", this)}
  , m_eStopButton{new ThrottleButton(Locale::tr("throttle.estop"), this)}
  , m_throttleStatus{new QLabel("", this)}
  , m_throttleAction{new ThrottleButton("", this)}
{
  setFocusPolicy(Qt::StrongFocus);

  m_eStopButton->setColor(ThrottleStyle::buttonEStopColor);
  m_throttleAction->setEnabled(false);

  fetchTrainVehicles();

  m_createThrottleRequestId = m_train->connection()->createObject("throttle.client",
    [this](const ObjectPtr& obj, std::optional<const Error> /*error*/)
    {
      m_createThrottleRequestId = Connection::invalidRequestId;

      if(obj)
      {
        m_throttle = obj;
        m_throttleTrain = m_throttle->getObjectProperty("train");
        m_throttleRelease = m_throttle->getMethod("release");
        m_throttleAcquire = m_throttle->getMethod("acquire");
        m_throttleSetDirection = m_throttle->getMethod("set_direction");
        m_throttleSetSpeed = m_throttle->getMethod("set_speed");
        m_throttleFaster = m_throttle->getMethod("faster");
        m_throttleSlower = m_throttle->getMethod("slower");
        m_throttleEmergencyStop = m_throttle->getMethod("emergency_stop");
        connect(m_throttleTrain, &AbstractProperty::valueChanged, this, &ThrottleWidget::updateThrottleControls);
        m_throttleAction->setEnabled(true);
      }
    });

  if(auto* name = m_train->getProperty("name")) [[likely]]
  {
    setWindowTitle(name->toString());
    connect(name, &AbstractProperty::valueChangedString, this, &ThrottleWidget::setWindowTitle);
    m_nameLabel->setText(name->toString());
    connect(name, &AbstractProperty::valueChangedString, m_nameLabel, &QLabel::setText);
  }

  connect(m_trainHasThrottle, &AbstractProperty::valueChanged, this, &ThrottleWidget::updateThrottleControls);
  connect(m_trainThrottleName, &AbstractProperty::valueChanged, this, &ThrottleWidget::updateThrottleControls);
  connect(m_trainDirection, &AbstractProperty::valueChanged, this, &ThrottleWidget::updateThrottleControls);
  connect(m_trainSpeed, &AbstractProperty::valueChangedDouble, m_speedoMeter, &SpeedoMeterWidget::setSpeed);
  connect(m_trainSpeed, &AbstractProperty::attributeChanged, this,
    [this](AttributeName attribute)
    {
      if(attribute == AttributeName::Max)
      {
        updateSpeedMax();
      }
    });
  connect(m_trainTargetSpeed, &AbstractProperty::valueChangedDouble, m_speedoMeter, &SpeedoMeterWidget::setSpeedTarget);
  connect(m_trainTargetSpeed, &AbstractProperty::valueChangedDouble, this,
    [this](double value)
    {
      m_speedoMeter->setSpeedTarget(value);
      m_speedSliderUpdateFromNetwork = true;
      m_speedSlider->setValue(value);
      m_speedSliderUpdateFromNetwork = false;
    });

  connect(m_trainTargetSpeed, &UnitProperty::unitChanged, this, &ThrottleWidget::updateSpeedUnit);
  connect(m_trainIsStopped, &AbstractProperty::valueChanged, this, &ThrottleWidget::updateThrottleControls);
  connect(m_trainEmergencyStop, &AbstractProperty::valueChanged, this, &ThrottleWidget::updateThrottleControls);
  connect(m_trainEmergencyStop, &AbstractProperty::valueChangedBool, m_speedoMeter, &SpeedoMeterWidget::setEStop);
  m_speedoMeter->setEStop(m_trainEmergencyStop->toBool());

  connect(m_forwardButton, &ThrottleButton::clicked, std::bind_front(&ThrottleWidget::setDirection, this, Direction::Forward));
  connect(m_reverseButton, &ThrottleButton::clicked, std::bind_front(&ThrottleWidget::setDirection, this, Direction::Reverse));
  connect(m_stopButton, &ThrottleButton::clicked,
    [this]()
    {
      setSpeed(0.0, QGuiApplication::keyboardModifiers().testFlag(immidiateSpeedModifier));
    });
  connect(m_eStopButton, &ThrottleButton::clicked, this, &ThrottleWidget::emergencyStop);

  connect(m_speedSlider, &SliderWidget::valueChanged,
    [this](float value)
    {
      if(!m_speedSliderUpdateFromNetwork)
      {
        setSpeed(value, QGuiApplication::keyboardModifiers().testFlag(immidiateSpeedModifier));
      }
    });

  connect(m_throttleAction, &ThrottleButton::clicked,
    [this]()
    {
      if(!m_throttleTrain || !m_throttleAcquire || !m_throttleRelease) [[unlikely]]
      {
        return;
      }

      if(m_throttleTrain->hasObject()) // release
      {
        const bool stop = !QGuiApplication::keyboardModifiers().testFlag(releaseWithoutStopModifier);
        callMethod(*m_throttleRelease, nullptr, stop);
      }
      else // acquire / steal
      {
        callMethodR<bool>(*m_throttleAcquire,
          [](const bool& /*success*/, std::optional<const Error> /*error*/)
          {
          },
          m_train, // train
          m_trainHasThrottle->toBool()); // steal
      }
    });

  auto* main = new QVBoxLayout();
  QFont font = m_nameLabel->font();
  font.setBold(true);
  m_nameLabel->setFont(font);
  main->addWidget(m_nameLabel);

  auto* columns = new QHBoxLayout();
  m_functions = new QVBoxLayout();
  m_functions->addStretch();
  columns->addLayout(m_functions, 1);

  auto* right = new QGridLayout();
  right->addWidget(m_speedoMeter, 0, 0, 1, 3);
  right->addWidget(m_speedSlider, 0, 4, 3, 1);
  right->addWidget(m_reverseButton, 1, 0);
  right->addWidget(m_stopButton, 1, 1);
  right->addWidget(m_forwardButton, 1, 2);
  right->addWidget(m_eStopButton, 2, 0, 1, 3);
  right->setRowStretch(0, 1);
  columns->addLayout(right);
  main->addLayout(columns, 1);

  auto* bottom = new QHBoxLayout();
  bottom->addWidget(m_throttleStatus, 1);
  bottom->addWidget(m_throttleAction);
  main->addLayout(bottom);

  setLayout(main);

  updateSpeedMax();
  updateSpeedUnit();
  updateThrottleControls();
}

ThrottleWidget::~ThrottleWidget()
{
  auto& c = *m_train->connection();
  if(m_createThrottleRequestId != Connection::invalidRequestId)
  {
    c.cancelRequest(m_createThrottleRequestId);
  }
  if(m_vehiclesRequestId != Connection::invalidRequestId)
  {
    c.cancelRequest(m_vehiclesRequestId);
  }
  for(const auto& it : m_vehicleDecoderRequestIds)
  {
    c.cancelRequest(it.second);
  }
}

void ThrottleWidget::keyPressEvent(QKeyEvent* event)
{
  if(const auto& function = getFunction(*event))
  {
    if(function->getPropertyValueEnum<DecoderFunctionType>("type", DecoderFunctionType::OnOff) == DecoderFunctionType::Hold)
    {
      function->setPropertyValue("value", true);
    }
    return;
  }
  QWidget::keyPressEvent(event);
}

void ThrottleWidget::keyReleaseEvent(QKeyEvent* event)
{
  if(const auto& function = getFunction(*event))
  {
    if(function->getPropertyValueEnum<DecoderFunctionType>("type", DecoderFunctionType::OnOff) == DecoderFunctionType::Hold)
    {
      function->setPropertyValue("value", false);
    }
    else
    {
      function->setPropertyValue("value", !function->getPropertyValueBool("value", true)); // toggle
    }
    return;
  }
  else
  {
    switch(event->key())
    {
      case Qt::Key_Space:
        emergencyStop();
        return;

      case Qt::Key_Return:
      case Qt::Key_Enter:
        setDirection(~m_trainDirection->toEnum<Direction>()); // toggle
        return;

      case Qt::Key_Left:
        setDirection(Direction::Reverse);
        return;

      case Qt::Key_Right:
        setDirection(Direction::Forward);
        return;

      case Qt::Key_Up:
        faster(event->modifiers().testFlag(immidiateSpeedModifier));
        return;

      case Qt::Key_Down:
        slower(event->modifiers().testFlag(immidiateSpeedModifier));
        return;
    }
  }
  QWidget::keyReleaseEvent(event);
}

void ThrottleWidget::wheelEvent(QWheelEvent* event)
{
  const bool immediate = event->modifiers().testFlag(immidiateSpeedModifier);
  if(event->angleDelta().y() > 0)
  {
    faster(immediate);
  }
  else if(event->angleDelta().y() < 0)
  {
    slower(immediate);
  }
}

void ThrottleWidget::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  painter.fillRect(rect(), ThrottleStyle::backgroundColor);
}

bool ThrottleWidget::throttleAcquired() const
{
  return m_throttleTrain && m_throttleTrain->hasObject();
}

void ThrottleWidget::setDirection(Direction value)
{
  if(throttleAcquired() && m_throttleSetDirection)
  {
    callMethodR<bool>(*m_throttleSetDirection,
      [](bool /*success*/, std::optional<const Error> /*error*/)
      {
      },
      value);
  }
}

void ThrottleWidget::setSpeed(double value, bool immediate)
{
  if(throttleAcquired() && m_throttleSetSpeed)
  {
    callMethodR<bool>(*m_throttleSetSpeed,
      [](bool /*success*/, std::optional<const Error> /*error*/)
      {
      },
      value,
      m_trainSpeed->unit<SpeedUnit>(),
      immediate);
  }
}

void ThrottleWidget::faster(bool immediate)
{
  if(throttleAcquired() && m_throttleFaster)
  {
    callMethodR<bool>(*m_throttleFaster,
      [](bool /*success*/, std::optional<const Error> /*error*/)
      {
      },
      immediate);
  }
}

void ThrottleWidget::slower(bool immediate)
{
  if(throttleAcquired() && m_throttleSlower)
  {
    callMethodR<bool>(*m_throttleSlower,
      [](bool /*success*/, std::optional<const Error> /*error*/)
      {
      },
      immediate);
  }
}

void ThrottleWidget::emergencyStop()
{
  if(throttleAcquired() && m_throttleEmergencyStop)
  {
    callMethodR<bool>(*m_throttleEmergencyStop,
      [](bool /*success*/, std::optional<const Error> /*error*/)
      {
      });
  }
  else if(m_trainEmergencyStop) [[likely]]
  {
    m_trainEmergencyStop->setValueBool(true);
  }
}

const ObjectPtr& ThrottleWidget::getFunction(int number)
{
  for(const auto& vehicle : m_trainVehicleDecoders)
  {
    const auto it = std::find_if(vehicle.functions.begin(), vehicle.functions.end(),
      [number](const auto& f)
      {
        return f->getPropertyValueInt("number", -1) == number;
      });

    if(it != vehicle.functions.end())
    {
      return *it;
    }
  }
  return nullObject;
}

const ObjectPtr& ThrottleWidget::getFunction(DecoderFunctionFunction function)
{
  for(const auto& vehicle : m_trainVehicleDecoders)
  {
    const auto it = std::find_if(vehicle.functions.begin(), vehicle.functions.end(),
      [function](const auto& f)
      {
        return f->getPropertyValueEnum("function", DecoderFunctionFunction::Generic) == function;
      });

    if(it != vehicle.functions.end())
    {
      return *it;
    }
  }
  return nullObject;
}

const ObjectPtr& ThrottleWidget::getFunction(const QKeyEvent& event)
{
  if(event.key() >= Qt::Key_0 && event.key() <= Qt::Key_9)
  {
    int n = event.key() - Qt::Key_0;

    switch(event.modifiers() & (Qt::ControlModifier | Qt::AltModifier))
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
        return nullObject;
    }

    return getFunction(n);
  }

  switch(event.key())
  {
    case Qt::Key_L:
      return getFunction(DecoderFunctionFunction::Light);

    case Qt::Key_M:
      return getFunction(DecoderFunctionFunction::Mute);

    case Qt::Key_S:
      return getFunction(DecoderFunctionFunction::Sound);
  }

  return nullObject;
}

void ThrottleWidget::updateSpeedMax()
{
  const auto maxSpeed = m_trainSpeed->getAttributeDouble(AttributeName::Max, 1.0);
  m_speedoMeter->setSpeedMax(maxSpeed);
  m_speedSlider->setMaximum(maxSpeed);
}

void ThrottleWidget::updateSpeedUnit()
{
  auto unit = toDisplayUnit(m_trainTargetSpeed->unit<SpeedUnit>());
  m_speedoMeter->setUnit(QString::fromUtf8(unit.data(), unit.size()));
}

void ThrottleWidget::updateThrottleControls()
{
  if(!m_trainHasThrottle || !m_trainThrottleName) [[unlikely]]
  {
    return;
  }

  const bool acquired = throttleAcquired();

  m_forwardButton->setEnabled(acquired && (m_trainIsStopped->toBool() || m_trainDirection->toEnum<Direction>() == Direction::Forward));
  m_stopButton->setEnabled(acquired && (!m_trainIsStopped->toBool() || m_trainEmergencyStop->toBool()));
  m_reverseButton->setEnabled(acquired && (m_trainIsStopped->toBool() || m_trainDirection->toEnum<Direction>() == Direction::Reverse));
  m_speedSlider->setEnabled(acquired);
  m_eStopButton->setEnabled(!m_trainEmergencyStop->toBool());

  m_forwardButton->setColor(m_trainDirection->toEnum<Direction>() == Direction::Forward ? ThrottleStyle::buttonActiveColor : ThrottleStyle::buttonColor);
  m_reverseButton->setColor(m_trainDirection->toEnum<Direction>() == Direction::Reverse ? ThrottleStyle::buttonActiveColor : ThrottleStyle::buttonColor);

  if(acquired)
  {
    m_throttleStatus->setText(Locale::tr("throttle.in_control"));
    m_throttleAction->setColor(ThrottleStyle::buttonActiveColor);
    m_throttleAction->setText(Locale::tr("throttle.release"));
  }
  else if(m_trainHasThrottle->toBool())
  {
    m_throttleStatus->setText(Locale::tr("throttle.controlled_by_x").arg(m_trainThrottleName->toString()));
    m_throttleAction->setColor(ThrottleStyle::buttonColor);
    m_throttleAction->setText(Locale::tr("throttle.steal"));
  }
  else
  {
    m_throttleStatus->setText(Locale::tr("throttle.not_controlled"));
    m_throttleAction->setColor(ThrottleStyle::buttonColor);
    m_throttleAction->setText(Locale::tr("throttle.acquire"));
  }
}

void ThrottleWidget::fetchTrainVehicles()
{
  m_trainVehiclesList.reset();
  m_trainVehicles.clear();
  m_trainVehicleDecoders.clear();

  if(auto* vehicles = m_train->getObjectProperty("vehicles"))
  {
    m_vehiclesRequestId = vehicles->getObject(
      [this](const ObjectPtr& obj, std::optional<const Error> /*error*/)
      {
        m_vehiclesRequestId = Connection::invalidRequestId;
        if(obj)
        {
          m_trainVehiclesList = obj;
          if(int n = m_trainVehiclesList->getPropertyValueInt("length", 0) - 1; n >= 0)
          {
            m_vehiclesRequestId = m_trainVehiclesList->connection()->getObjects(*m_trainVehiclesList, 0, static_cast<uint32_t>(n),
              [this](const std::vector<ObjectPtr>& objects, std::optional<const Error> error)
              {
                m_vehiclesRequestId = Connection::invalidRequestId;
                if(!error)
                {
                  m_trainVehicles = objects;
                  fetchTrainVehicleDecoders();
                }
              });
          }
        }
      });
  }
}

void ThrottleWidget::fetchTrainVehicleDecoders()
{
  m_trainVehicleDecoders.clear();
  m_trainVehicleDecoders.resize(m_trainVehicles.size());

  for(size_t i = 0; i < m_trainVehicles.size(); ++i)
  {
    if(auto* decoder = m_trainVehicles[i]->getObjectProperty("decoder"); decoder && decoder->hasObject())
    {
      m_vehicleDecoderRequestIds[i] = decoder->getObject(
        [this, i](const ObjectPtr& obj, std::optional<const Error> /*error*/)
        {
          m_vehicleDecoderRequestIds.erase(i);

          if(obj)
          {
            m_trainVehicleDecoders[i].decoder = obj;
            fetchTrainVehicleDecoderFunctions(i);
          }
        });
    }
  }
}

void ThrottleWidget::fetchTrainVehicleDecoderFunctions(size_t vehicleIndex)
{
  if(vehicleIndex < m_trainVehicleDecoders.size() && m_trainVehicleDecoders[vehicleIndex].decoder) [[likely]]
  {
    assert(m_trainVehicleDecoders[vehicleIndex].functions.empty());

    if(auto* functions = m_trainVehicleDecoders[vehicleIndex].decoder->getObjectProperty("functions"))
    {
      m_vehicleDecoderRequestIds[vehicleIndex] = functions->getObject(
        [this, vehicleIndex](const ObjectPtr& obj, std::optional<const Error> /*error*/)
        {
          m_vehicleDecoderRequestIds[vehicleIndex] = Connection::invalidRequestId;
          if(obj)
          {
            if(auto* items = obj->getObjectVectorProperty("items")) [[likely]]
            {
              [[maybe_unused]] int y = items->getObjects(
                [this, vehicleIndex](const std::vector<ObjectPtr>& functionObjs, std::optional<const Error> error)
                {
                  if(!error)
                  {
                    m_trainVehicleDecoders[vehicleIndex].functions = functionObjs;
                    createDecoderFunctionWidgets(vehicleIndex);
                  }
                });
            }
          }
        });
    }
  }
}

void ThrottleWidget::createDecoderFunctionWidgets(size_t vehicleIndex)
{
  constexpr int functionButtonsPerRow = 5;

  auto* grid = new QGridLayout();

  if(auto* name = m_trainVehicles[vehicleIndex]->getProperty("name"))
  {
    auto* nameLabel = new QLabel(name->toString(), this);
    connect(name, &AbstractProperty::valueChangedString, nameLabel, &QLabel::setText);
    grid->addWidget(nameLabel, 0, 0, 1, functionButtonsPerRow + 1);
  }

  int index = 0;
  for(const auto& function : m_trainVehicleDecoders[vehicleIndex].functions)
  {
    grid->addWidget(new ThrottleFunctionButton(function, this), 1 + index / functionButtonsPerRow, index % functionButtonsPerRow);
    index++;
  }
  grid->addItem(new QSpacerItem(0, 0, QSizePolicy::Maximum), 1, functionButtonsPerRow);

  int layoutIndex = 0;
  for(size_t i = 0; i < vehicleIndex; ++i)
  {
    if(!m_trainVehicleDecoders[i].functions.empty())
    {
      layoutIndex++;
    }
  }
  m_functions->insertLayout(layoutIndex, grid);
}
