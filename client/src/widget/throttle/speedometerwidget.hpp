/**
 * client/src/widget/throttlespeedometerwidget.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_WIDGET_THROTTLE_SPEEDOMETERWIDGET_HPP
#define TRAINTASTIC_CLIENT_WIDGET_THROTTLE_SPEEDOMETERWIDGET_HPP

#include <QWidget>

class SpeedoMeterWidget : public QWidget
{
  Q_OBJECT

  private:
    bool m_eStop;
    double m_speed;
    double m_speedMax;
    double m_speedLimit;
    double m_speedTarget;
    QString m_unit;

  protected:
    void paintEvent(QPaintEvent*) final;

  public:
    SpeedoMeterWidget(QWidget* parent = nullptr);

    bool eStop() const { return m_eStop; }
    double speed() const { return m_speed; }
    double speedMax() const { return m_speedMax; }
    double speedLimit() const { return m_speedLimit; }
    const QString& unit() const { return m_unit; }

    QSize minimumSizeHint() const final { return QSize{150, 150}; }
    bool hasHeightForWidth() const final { return true; }
    int heightForWidth(int w) const final { return w; }

  public slots:
    void setEStop(bool value);
    void setSpeed(double value);
    void setSpeedMax(double value);
    void setSpeedLimit(double value);
    void setSpeedTarget(double value);
    void setUnit(QString value);
};

#endif
