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

#include <QBasicTimer>
#include <QImage>

class TrainsModel;

class QHelpEvent;

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

  void loadExtraImages(const nlohmann::json &world,
                       const QString &imagesFile,
                       QStringList &namesOut);

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

  inline Simulator::Point getCamera() const
  {
      return {m_cameraX, m_cameraY};
  }

  void setCamera(const Simulator::Point& cameraPt);

  inline Simulator::Point mapToSim(const QPointF& p)
  {
    return {m_cameraX + float(p.x()) / m_zoomLevel,
            m_cameraY + float(p.y()) / m_zoomLevel};
  }

  inline float getZoomLevel() const { return m_zoomLevel; }

  void setZoomLevel(float zoomLevel);

  void setImageVisible(int idx, bool val);

  nlohmann::json copySegmentData(size_t segmentIdx) const;

  bool thinTracks() const;
  void setThinTracks(bool newThinTracks);

signals:
  void tickActiveChanged(float value);
  void powerOnChanged(bool value);

protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;
  void resizeEvent(QResizeEvent* event) override;

  bool event(QEvent *e) override;
  void keyPressEvent(QKeyEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void timerEvent(QTimerEvent *e) override;
  void contextMenuEvent(QContextMenuEvent *e) override;

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
    bool visible = true;
  };

  std::vector<Image> m_images;
  std::vector<Image> m_extraImages;

  struct
  {
    bool powerOn = false;
  } m_stateDataPrevious;
  std::vector<boost::signals2::connection> m_simulatorConnections;
  Turnouts m_turnouts;

  // Trains
  TrainsModel *mTrainsModel = nullptr;

  // View
  float m_cameraX = 0.0f;
  float m_cameraY = 0.0f;
  float m_zoomLevel = 1.0f;

  bool m_showTrackOccupancy = true;
  bool m_thinTracks = false;
  bool m_zoomFit = false;

  QPoint m_leftClickMousePos;
  QPoint m_rightMousePos;

  size_t m_trainIndex = 0;

  // Turnout blink
  QBasicTimer turnoutBlinkTimer;
  bool turnoutBlinkState = false;

  // Mouse hover
  QBasicTimer segmentHoverTimer;
  Simulator::Point m_lastHoverPos;
  size_t m_hoverSegmentIdx = Simulator::invalidIndex;
  size_t m_hoverSensorIdx = Simulator::invalidIndex;

  void mouseLeftClick(const Simulator::Point &point, bool shiftPressed);
  void showItemTooltip(const Simulator::Point &point, QHelpEvent *ev);

  void drawTracks(QPainter *painter);
  void drawTrackObjects(QPainter *painter);
  void drawTrains(QPainter *painter);
  void drawMisc(QPainter *painter);

  void updateProjection();

  inline void resetSegmentHover()
  {
    m_hoverSegmentIdx = Simulator::invalidIndex;
    m_hoverSensorIdx = Simulator::invalidIndex;
    segmentHoverTimer.stop();
  }

  void showAddTrainDialog(size_t segmentIndex);

private slots:
  void tick();
};

#endif
