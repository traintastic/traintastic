/**
 * client/src/widget/throttlewidget.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2023 Reinder Feenstra
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
class QGridLayout;
class SpeedoMeterWidget;
class ThrottleStopButton;
class ThrottleDirectionButton;
class ThrottleFunctionButton;
class AbstractProperty;
class UnitProperty;
class Method;

class ThrottleWidget final : public QWidget
{
  private:
    ObjectPtr m_object;
    int m_functionsRequestId;
    std::map<int, int> m_functionRequestIds;
    AbstractProperty* m_emergencyStop;
    UnitProperty* m_speed = nullptr;
    UnitProperty* m_throttleSpeed = nullptr;
    AbstractProperty* m_throttle = nullptr;
    Method* m_toggleDirection;
    QLabel* m_nameLabel;
    QGridLayout* m_functionGrid;
    std::list<ThrottleFunctionButton*> m_functionButtons;
    SpeedoMeterWidget* m_speedoMeter;
    ThrottleStopButton* m_stopButton;
    ThrottleDirectionButton* m_reverseButton;
    ThrottleDirectionButton* m_forwardButton;

    void changeSpeed(bool up);

    ThrottleFunctionButton* getFunctionButton(int number);
    ThrottleFunctionButton* getFunctionButton(DecoderFunctionFunction function);
    ThrottleFunctionButton* getFunctionButton(const QKeyEvent& event);

  protected:
    void keyPressEvent(QKeyEvent* event) final;
    void keyReleaseEvent(QKeyEvent* event) final;
    void wheelEvent(QWheelEvent* event) final;
    void paintEvent(QPaintEvent*) final;

  public:
    explicit ThrottleWidget(ObjectPtr object, QWidget* parent = nullptr);
    ~ThrottleWidget() final;
};

#endif
