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

#include "simulatorview.hpp"
#include <numbers>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>

#include <QPainter>

namespace
{

constexpr auto pi = std::numbers::pi_v<float>;

struct ColorF
{
  float red;
  float green;
  float blue;
};

inline static const std::array<ColorF, 17> colors{{
  {0.00f, 0.00f, 0.00f}, //	None
  {0.00f, 0.00f, 0.00f}, //	Black
  {0.75f, 0.75f, 0.75f}, //	Silver
  {0.50f, 0.50f, 0.50f}, //	Gray
  {1.00f, 1.00f, 1.00f}, // White
  {0.50f, 0.00f, 0.00f}, // Maroon
  {1.00f, 0.00f, 0.00f}, // Red
  {0.50f, 0.00f, 0.50f}, // Purple
  {1.00f, 0.00f, 1.00f}, // Fuchsia
  {0.00f, 0.50f, 0.00f}, // Green
  {0.00f, 1.00f, 0.00f}, // Lime
  {0.50f, 0.50f, 0.00f}, // Olive
  {1.00f, 1.00f, 0.00f}, // Yellow
  {0.00f, 0.00f, 0.50f}, // Navy
  {0.00f, 0.00f, 1.00f}, // Blue
  {0.00f, 0.50f, 0.50f}, // Teal
  {0.00f, 1.00f, 1.00f}, // Aqua
}};

float crossProduct(Simulator::Point p1, Simulator::Point p2, Simulator::Point p3)
{
  return (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
}

bool isPointInTriangle(std::span<const Simulator::Point, 3> triangle, const Simulator::Point point)
{
  const float cross1 = crossProduct(triangle[0], triangle[1], point);
  const float cross2 = crossProduct(triangle[1], triangle[2], point);
  const float cross3 = crossProduct(triangle[2], triangle[0], point);

  const bool hasNeg = (cross1 < 0) || (cross2 < 0) || (cross3 < 0);
  const bool hasPos = (cross1 > 0) || (cross2 > 0) || (cross3 > 0);

  return !(hasNeg && hasPos);
}

void drawStraight(const Simulator::TrackSegment& segment)
{
  glBegin(GL_LINES);
  glVertex2f(0, 0);
  glVertex2f(segment.straight.length, 0);
  glEnd();
}

void drawCurve(const Simulator::TrackSegment& segment, size_t curveIndex)
{
  const auto& curve = segment.curves[curveIndex];
  const float rotation = curve.angle < 0 ? 0.0f : pi;

  int numSegments = qCeil(std::abs(curve.angle) / (pi / 36.0f)); // Smooth curve
  float step = curve.angle / numSegments;
  const float cx = curve.radius * sinf(rotation);
  const float cy = curve.radius * -cosf(rotation);

  glBegin(GL_LINE_STRIP);
  glVertex2f(0.0f, 0.0f);
  for(int i = 1; i <= numSegments; i++)
  {
    float angle = rotation + i * step;
    float x = cx - curve.radius * sinf(angle);
    float y = cy - curve.radius * -cosf(angle);
    glVertex2f(x, y);
  }
  glEnd();
}

}

SimulatorView::SimulatorView(QWidget* parent)
  : QOpenGLWidget(parent)
{
  setFocusPolicy(Qt::StrongFocus); // for key stuff
}

SimulatorView::~SimulatorView()
{
  setSimulator({});
}

Simulator* SimulatorView::simulator() const
{
  return m_simulator.get();
}

void SimulatorView::setSimulator(std::shared_ptr<Simulator> value)
{
  if(m_simulator)
  {
    m_simulatorConnections.clear();
    m_simulator->stop();
  }

  m_simulator = std::move(value);
  m_turnouts.clear();
  m_images.clear();

  if(m_simulator)
  {
    const size_t count = m_simulator->staticData.trackSegments.size();
    for(size_t i = 0; i < count; ++i)
    {
      const auto& segment = m_simulator->staticData.trackSegments[i];
      if(segment.type == Simulator::TrackSegment::Type::Turnout || segment.type == Simulator::TrackSegment::Type::TurnoutCurved)
      {
        m_turnouts.emplace_back(Turnout{i, std::span<const Simulator::Point, 3>(segment.points.data(), 3)});
      }
    }

    m_stateData = m_simulator->stateData();
    emit powerOnChanged(m_stateData.powerOn);

    m_simulatorConnections.emplace_back(m_simulator->onTick.connect(
      [this]()
      {
        QMetaObject::invokeMethod(this, "tick", Qt::QueuedConnection);
      }));

    m_simulator->enableServer();
    m_simulator->start();

    for(const auto &imgRef : m_simulator->staticData.images)
    {
      Image item;
      item.ref = imgRef;

      if(!item.img.load(QString::fromStdString(item.ref.fileName)))
        continue;

      m_images.push_back(item);

      // const QSizeF imgSize = QSizeF(item.img.size()) * item.ref.ratio;

      // const float cosRotation = std::cos(item.ref.rotation);
      // const float sinRotation = std::sin(item.ref.rotation);
      // Simulator::updateView(m_simulator->staticData.view,
      //                       {item.ref.origin.x + imgSize.width() * cosRotation, item.ref.origin.y + imgSize.width() * sinRotation}); // top right
      // Simulator::updateView(m_simulator->staticData.view,
      //            {item.ref.origin.x - imgSize.height() * sinRotation, item.ref.origin.y + imgSize.height() * cosRotation}); // bottom left
      // Simulator::updateView(m_simulator->staticData.view,
      //            {item.ref.origin.x - imgSize.height() * sinRotation + imgSize.width() * cosRotation, item.ref.origin.y + imgSize.height() * cosRotation + imgSize.width() * sinRotation}); // bottom right
    }
  }

  update();
}

void SimulatorView::zoomIn()
{
  setZoomLevel(m_zoomLevel * zoomFactorIn);
}

void SimulatorView::zoomOut()
{
  setZoomLevel(m_zoomLevel * zoomFactorOut);
}

void SimulatorView::zoomToFit()
{
  if(!m_simulator) [[unlikely]]
  {
    return;
  }

  // Make it fit:
  const float zoomLevelX = width() / m_simulator->staticData.view.width();
  const float zoomLevelY = height() / m_simulator->staticData.view.height();
  const float zoomLevel = std::min(zoomLevelX, zoomLevelY);

  // Center it:
  m_cameraX = m_simulator->staticData.view.left - (width() / zoomLevel - m_simulator->staticData.view.width()) / 2;
  m_cameraY = m_simulator->staticData.view.top - (height() / zoomLevel - m_simulator->staticData.view.height()) / 2;

  setZoomLevel(zoomLevel);
}

void SimulatorView::initializeGL()
{
  initializeOpenGLFunctions();
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

void SimulatorView::resizeGL(int w, int h)
{
  makeCurrent();
  glViewport(0, 0, w, h);
  updateProjection();
}

void SimulatorView::paintGL()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if(m_images.size() > 0)
  {
      QPainter p;
      p.begin(this);

      p.scale(m_zoomLevel, m_zoomLevel);
      p.translate(-m_cameraX, -m_cameraY);

      const QTransform trasf = p.transform();

      for(const auto &image : m_images)
      {
          p.translate(image.ref.origin.x, image.ref.origin.y);
          p.rotate(qRadiansToDegrees(image.ref.rotation));
          p.scale(image.ref.ratio, image.ref.ratio);
          p.drawImage(QPoint(), image.img);
          p.setTransform(trasf);
      }

      p.end();
  }

  glLoadIdentity();

  if(m_simulator) [[likely]]
  {
    drawMisc();
    drawTracks();
    drawTrains();
  }

}

void SimulatorView::drawTracks()
{
  assert(m_simulator);
  for(const auto& segment : m_simulator->staticData.trackSegments)
  {
    if(m_showTrackOccupancy && segment.hasSensor() && m_stateData.sensors[segment.sensor.index].value)
    {
      glColor3f(1.0f, 0.0f, 0.0f); // Red if occupied
    }
    else
    {
      glColor3f(0.8f, 0.8f, 0.8f);
    }

    glPushMatrix();
    glTranslatef(segment.origin().x, segment.origin().y, 0);
    glRotatef(qRadiansToDegrees(segment.rotation), 0, 0, 1);

    if(segment.type == Simulator::TrackSegment::Type::Straight)
    {
      drawStraight(segment);
    }
    else if(segment.type == Simulator::TrackSegment::Type::Curve)
    {
      drawCurve(segment, 0);
    }
    else if(segment.type == Simulator::TrackSegment::Type::Turnout)
    {
      assert(segment.turnout.index < m_stateData.turnouts.size());
      const auto state = m_stateData.turnouts[segment.turnout.index].state;

      switch(state)
      {
        case Simulator::TurnoutState::State::Closed:
          drawCurve(segment, 0);
          glColor3f(0.0f, 1.0f, 1.0f);
          drawStraight(segment);
          break;

        case Simulator::TurnoutState::State::Thrown:
          drawStraight(segment);
          glColor3f(0.0f, 1.0f, 1.0f);
          drawCurve(segment, 0);
          break;

        default:
          assert(false);
          break;
      }
    }
    else if(segment.type == Simulator::TrackSegment::Type::TurnoutCurved)
    {
      assert(segment.turnout.index < m_stateData.turnouts.size());
      const auto state = m_stateData.turnouts[segment.turnout.index].state;

      switch(state)
      {
        case Simulator::TurnoutState::State::Closed:
          drawCurve(segment, 1);
          glColor3f(0.0f, 1.0f, 1.0f);
          drawCurve(segment, 0);
          break;

        case Simulator::TurnoutState::State::Thrown:
          drawCurve(segment, 0);
          glColor3f(0.0f, 1.0f, 1.0f);
          drawCurve(segment, 1);
          break;

        default:
          assert(false);
          break;
      }
    }
    else if(segment.type == Simulator::TrackSegment::Type::Turnout3Way)
    {
      assert(segment.turnout.index < m_stateData.turnouts.size());
      const auto state = m_stateData.turnouts[segment.turnout.index].state;

      switch(state)
      {
        case Simulator::TurnoutState::State::Closed:
          drawCurve(segment, 0);
          drawCurve(segment, 1);
          glColor3f(0.0f, 1.0f, 1.0f);
          drawStraight(segment);
          break;

        case Simulator::TurnoutState::State::ThrownLeft:
          drawStraight(segment);
          drawCurve(segment, 1);
          glColor3f(0.0f, 1.0f, 1.0f);
          drawCurve(segment, 0);
          break;

        case Simulator::TurnoutState::State::ThrownRight:
          drawStraight(segment);
          drawCurve(segment, 0);
          glColor3f(0.0f, 1.0f, 1.0f);
          drawCurve(segment, 1);
          break;

        default:
          assert(false);
          break;
      }
    }
    glPopMatrix();
  }
}

void SimulatorView::drawTrains()
{
  assert(m_simulator);

  const float trainWidth = m_simulator->staticData.trainWidth;

  const size_t railVehicleCount = m_simulator->staticData.vehicles.size();
  for(size_t i = 0; i < railVehicleCount; ++i)
  {
    const auto& vehicle = m_stateData.vehicles[i];
    const float length = m_simulator->staticData.vehicles[i].length;

    const auto center = (vehicle.front.position + vehicle.rear.position) / 2;
    const auto delta = vehicle.front.position - vehicle.rear.position;
    const float angle = atan2f(delta.y, delta.x);

    glPushMatrix();
    glTranslatef(center.x, center.y, 0);
    glRotatef(qRadiansToDegrees(angle), 0, 0, 1);

    const auto& color = colors[static_cast<size_t>(m_simulator->staticData.vehicles[i].color)];
    glColor3f(color.red, color.green, color.blue);
    glBegin(GL_QUADS);
    glVertex2f(-length / 2, -trainWidth / 2);
    glVertex2f(length / 2, -trainWidth / 2);
    glVertex2f(length / 2, trainWidth / 2);
    glVertex2f(-length / 2, trainWidth / 2);
    glEnd();

    glPopMatrix();
  }
}

void SimulatorView::drawMisc()
{
  assert(m_simulator);

  for(const auto& item : m_simulator->staticData.misc)
  {
    const auto& color = colors[static_cast<size_t>(item.color)];
    glColor3f(color.red, color.green, color.blue);

    switch(item.type)
    {
      case Simulator::Misc::Type::Rectangle:
        glPushMatrix();
        glTranslatef(item.origin.x, item.origin.y, 0.0f);
        glRotatef(qRadiansToDegrees(item.rotation), 0.0f, 0.0f, 1.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(item.width, 0.0f);
        glVertex2f(item.width, item.height);
        glVertex2f(0.0f, item.height);
        glEnd();
        glPopMatrix();
        break;
    }
  }
}

void SimulatorView::keyPressEvent(QKeyEvent* event)
{
  if(!m_simulator) [[unlikely]]
  {
    return QWidget::keyPressEvent(event);
  }

  switch(event->key())
  {
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
      if(static_cast<size_t>(event->key() - Qt::Key_1) < m_simulator->staticData.trains.size())
      {
        m_trainIndex = event->key() - Qt::Key_1;
      }
      break;

    case Qt::Key_Up:
      m_simulator->applyTrainSpeedDelta(m_trainIndex, m_simulator->staticData.trains[m_trainIndex].speedMax / 20);
      break;

    case Qt::Key_Down:
      m_simulator->applyTrainSpeedDelta(m_trainIndex, -m_simulator->staticData.trains[m_trainIndex].speedMax / 20);
      break;

    case Qt::Key_Right:
      m_simulator->setTrainDirection(m_trainIndex, false);
      break;

    case Qt::Key_Left:
      m_simulator->setTrainDirection(m_trainIndex, true);
      break;

    case Qt::Key_Space:
      m_simulator->setTrainSpeed(m_trainIndex, 0.0f);
      break;

    case Qt::Key_Escape:
      m_simulator->stopAllTrains();
      break;

    case Qt::Key_P:
      m_simulator->togglePowerOn();
      break;

    default:
      return QWidget::keyPressEvent(event);
  }
}

void SimulatorView::mousePressEvent(QMouseEvent* event)
{
  if(event->button() == Qt::LeftButton)
  {
    m_leftClickMousePos = event->pos();
  }
  if(event->button() == Qt::RightButton)
  {
    m_rightMousePos = event->pos();
    setCursor(Qt::ClosedHandCursor);
  }
}

void SimulatorView::mouseMoveEvent(QMouseEvent* event)
{
  if(event->buttons() & Qt::RightButton)
  {
    const auto diff = m_rightMousePos - event->pos();

    m_cameraX += diff.x() / m_zoomLevel;
    m_cameraY += diff.y() / m_zoomLevel;

    m_rightMousePos = event->pos();
    updateProjection();
  }
}

void SimulatorView::mouseReleaseEvent(QMouseEvent* event)
{
  if(event->button() == Qt::LeftButton)
  {
    auto diff = m_leftClickMousePos - event->pos();
    if(qAbs(diff.x()) <= 2 && qAbs(diff.y()) <= 2)
    {
      mouseLeftClick({m_cameraX + m_leftClickMousePos.x() / m_zoomLevel, m_cameraY + m_leftClickMousePos.y() / m_zoomLevel});
    }
  }
  if(event->button() == Qt::RightButton)
  {
    setCursor(Qt::ArrowCursor);
  }
}

void SimulatorView::wheelEvent(QWheelEvent* event)
{
  if(event->angleDelta().y() < 0)
  {
    zoomOut();
  }
  else
  {
    zoomIn();
  }
}

void SimulatorView::mouseLeftClick(QPointF pos)
{
  const Simulator::Point point(pos.x(), pos.y());
  for(const auto& turnout : m_turnouts)
  {
    if(isPointInTriangle(turnout.points, point))
    {
      m_simulator->toggleTurnoutState(turnout.segmentIndex);
      update();
      break;
    }
  }
}

void SimulatorView::setZoomLevel(float value)
{
  m_zoomLevel = std::clamp(value, zoomLevelMin, zoomLevelMax);
  updateProjection();
}

void SimulatorView::updateProjection()
{
  makeCurrent();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  const float viewWidth = width() / m_zoomLevel;
  const float viewHeight = height() / m_zoomLevel;

  glOrtho(m_cameraX, m_cameraX + viewWidth, m_cameraY + viewHeight, m_cameraY, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  update();
}

void SimulatorView::tick()
{
  m_stateDataPrevious.powerOn = m_stateData.powerOn;
  m_stateData = m_simulator->stateData();

  if(m_stateDataPrevious.powerOn != m_stateData.powerOn)
  {
    emit powerOnChanged(m_stateData.powerOn);
  }

  update();
}
