/**
 * client/src/widget/throttlewidget.hpp
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

#ifndef TRAINTASTIC_CLIENT_WIDGET_THROTTLE_THROTTLEWIDGET_HPP
#define TRAINTASTIC_CLIENT_WIDGET_THROTTLE_THROTTLEWIDGET_HPP

#include <QWidget>
#include <list>
#include <map>
#include <traintastic/enum/decoderfunctionfunction.hpp>
#include "../../network/objectptr.hpp"

class QLabel;
class QPushButton;
class QVBoxLayout;
class SpeedoMeterWidget;
class SliderWidget;
class ThrottleFunctionButton;
class ThrottleButton;
class AbstractProperty;
class ObjectProperty;
class UnitProperty;
class Method;

enum class Direction : uint8_t;

class ThrottleWidget final : public QWidget
{
  friend class ScreenShotDialog;

  private:
    struct VehicleDecoder
    {
      ObjectPtr decoder;
      ObjectPtr decoderFunctions;
      std::vector<ObjectPtr> functions;
    };

    ObjectPtr m_train;
    AbstractProperty* m_trainHasThrottle;
    AbstractProperty* m_trainThrottleName;
    AbstractProperty* m_trainDirection;
    UnitProperty* m_trainSpeed;
    UnitProperty* m_trainTargetSpeed;
    AbstractProperty* m_trainIsStopped;
    AbstractProperty* m_trainEmergencyStop;
    ObjectPtr m_trainVehiclesList;
    std::vector<ObjectPtr> m_trainVehicles;
    std::vector<VehicleDecoder> m_trainVehicleDecoders;

    ObjectPtr m_throttle;
    ObjectProperty* m_throttleTrain = nullptr;
    Method* m_throttleAcquire = nullptr;
    Method* m_throttleRelease = nullptr;
    Method* m_throttleSetSpeed = nullptr;
    Method* m_throttleFaster = nullptr;
    Method* m_throttleSlower = nullptr;
    Method* m_throttleEmergencyStop = nullptr;
    Method* m_throttleSetDirection = nullptr;

    int m_createThrottleRequestId;
    int m_vehiclesRequestId;
    std::unordered_map<size_t, int> m_vehicleDecoderRequestIds;

    QLabel* m_nameLabel;
    QVBoxLayout* m_functions;
    SpeedoMeterWidget* m_speedoMeter;
    SliderWidget* m_speedSlider;
    ThrottleButton* m_stopButton;
    ThrottleButton* m_reverseButton;
    ThrottleButton* m_forwardButton;
    ThrottleButton* m_eStopButton;
    QLabel* m_throttleStatus;
    ThrottleButton* m_throttleAction;
    bool m_speedSliderUpdateFromNetwork = false;

    bool throttleAcquired() const;
    void setDirection(Direction value);
    void setSpeed(double value, bool immediate);
    void faster(bool immediate);
    void slower(bool immediate);
    void emergencyStop();

    const ObjectPtr& getFunction(int number);
    const ObjectPtr& getFunction(DecoderFunctionFunction function);
    const ObjectPtr& getFunction(const QKeyEvent& event);

    void updateSpeedMax();
    void updateSpeedUnit();
    void updateThrottleControls();

    void fetchTrainVehicles();
    void fetchTrainVehicleDecoders();
    void fetchTrainVehicleDecoderFunctions(size_t vehicleIndex);

    void createDecoderFunctionWidgets(size_t vehicleIndex);

  protected:
    void keyPressEvent(QKeyEvent* event) final;
    void keyReleaseEvent(QKeyEvent* event) final;
    void wheelEvent(QWheelEvent* event) final;
    void paintEvent(QPaintEvent*) final;

  public:
    explicit ThrottleWidget(ObjectPtr train, QWidget* parent = nullptr);
    ~ThrottleWidget() final;
};

#endif
