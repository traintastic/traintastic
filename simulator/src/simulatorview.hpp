/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SIMULATOR_SIMULATORVIEW_HPP
#define TRAINTASTIC_SIMULATOR_SIMULATORVIEW_HPP

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "simulator.hpp"

class SimulatorView : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT

public:
  explicit SimulatorView(QWidget *parent = nullptr);

  Simulator *simulator() const;
  void setSimulator(Simulator *value);

  bool showTrackOccupancy() const
  {
    return m_showTrackOccupancy;
  }

  void setShowTrackOccupancy(bool value)
  {
    m_showTrackOccupancy = value;
    update();
  }

protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

  void keyPressEvent(QKeyEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent *event) override;

private:
  struct Turnout
  {
    size_t segmentIndex;
    Simulator::Point points[3];
  };
  using Turnouts = std::vector<Turnout>;

  Simulator* m_simulator = nullptr;
  Turnouts m_turnouts;
  float m_cameraX = 0.0f;
  float m_cameraY = 0.0f;
  float m_zoomLevel = 1.0f;
  QPoint m_leftClickMousePos;
  QPoint m_rightMousePos;
  bool m_showTrackOccupancy = true;
  size_t m_trainIndex = 0;

  void mouseLeftClick(QPointF pos);

  void drawTracks();
  void drawTrains();

  void updateProjection();
};

#endif
