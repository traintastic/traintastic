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

#include <span>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <traintastic/simulator/simulator.hpp>

#include <QImage>

class SimulatorView
  : public QOpenGLWidget
  , protected QOpenGLFunctions
{
  Q_OBJECT

public:
  explicit SimulatorView(QWidget* parent = nullptr);
  ~SimulatorView() override;

  Simulator* simulator() const;
  void setSimulator(std::shared_ptr<Simulator> value);

  bool showTrackOccupancy() const
  {
    return m_showTrackOccupancy;
  }

  void setShowTrackOccupancy(bool value)
  {
    m_showTrackOccupancy = value;
    update();
  }

  void zoomIn();
  void zoomOut();
  void zoomToFit();

signals:
  void powerOnChanged(bool value);

protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

  void keyPressEvent(QKeyEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

private:
  struct Turnout
  {
    size_t segmentIndex;
    std::span<const Simulator::Point, 3> points;
  };
  using Turnouts = std::vector<Turnout>;

  static constexpr float zoomLevelMin =  0.1f;
  static constexpr float zoomLevelMax = 15.0f;
  static constexpr float zoomFactorIn = 1.1f;
  static constexpr float zoomFactorOut = 0.9f;

  std::shared_ptr<Simulator> m_simulator;
  Simulator::StateData m_stateData;

  struct Image
  {
    QImage img;
    Simulator::ImageRef ref;
  };

  std::vector<Image> m_images;

  struct
  {
    bool powerOn = false;
  } m_stateDataPrevious;
  std::vector<boost::signals2::connection> m_simulatorConnections;
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
  void drawMisc();

  void setZoomLevel(float zoomLevel);

  void updateProjection();

private slots:
  void tick();
};

#endif
