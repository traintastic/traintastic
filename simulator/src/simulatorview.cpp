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
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>

namespace {
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

}

SimulatorView::SimulatorView(QWidget* parent)
  : QOpenGLWidget(parent)
{
  setFocusPolicy(Qt::StrongFocus); // for key stuff
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

  if(m_simulator)
  {
    const size_t count = m_simulator->staticData.trackSegments.size();
    for(size_t i = 0; i < count; ++i)
    {
      const auto& segment = m_simulator->staticData.trackSegments[i];
      if(segment.type == Simulator::TrackSegment::Type::Turnout)
      {
        m_turnouts.emplace_back(Turnout{i, segment.points});
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
  }

  update();
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
  glLoadIdentity();

  if(m_simulator) [[likely]]
  {
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

    if(segment.type == Simulator::TrackSegment::Type::Straight)
    {
      glRotatef(qRadiansToDegrees(segment.rotation), 0, 0, 1);

      glBegin(GL_LINES);
      glVertex2f(0, 0);
      glVertex2f(segment.straight.length, 0);
      glEnd();
    }
    else if(segment.type == Simulator::TrackSegment::Type::Curve)
    {
      float startAngle = segment.rotation;
      if(segment.curve.angle < 0)
      {
        startAngle += M_PIf;
      }
      float endAngle = segment.curve.angle;
      int numSegments = qCeil(std::abs(segment.curve.angle) / (M_PIf / 36.0f)); // Smooth curve
      float step = endAngle / numSegments;
      float prevX = segment.curve.radius * sinf(startAngle);
      float prevY = segment.curve.radius * -cosf(startAngle);

      glBegin(GL_LINE_STRIP);
      for(int i = 1; i <= numSegments; i++)
      {
        float angle = startAngle + i * step;
        float x = segment.curve.radius * sinf(angle);
        float y = segment.curve.radius * -cosf(angle);
        glVertex2f(prevX, prevY);
        glVertex2f(x, y);
        prevX = x;
        prevY = y;
      }
      glEnd();
    }
    else if(segment.type == Simulator::TrackSegment::Type::Turnout)
    {
      assert(segment.turnout.index < m_stateData.turnouts.size());
      const auto state = m_stateData.turnouts[segment.turnout.index].state;

      glRotatef(qRadiansToDegrees(segment.rotation), 0, 0, 1);

      if(state == Simulator::TurnoutState::State::Thrown)
      {
        glBegin(GL_LINES);
        glVertex2f(segment.straight.length, 0);
        glVertex2f(0, 0);
        glEnd();

        glColor3f(0.0f, 1.0f, 1.0f);
      }

      const float rotation = segment.curve.angle < 0 ? 0.0f : M_PIf;

      int numSegments = qCeil(std::abs(segment.curve.angle) / (M_PIf / 36.0f)); // Smooth curve
      float step = segment.curve.angle / numSegments;
      float prevX = 0.0f;
      float prevY = 0.0f;
      const float cx = segment.curve.radius * sinf(rotation);
      const float cy = segment.curve.radius * -cosf(rotation);

      glBegin(GL_LINE_STRIP);
      for(int i = 1; i <= numSegments; i++)
      {
        float angle = rotation + i * step;
        float x = cx - segment.curve.radius * sinf(angle);
        float y = cy - segment.curve.radius * -cosf(angle);
        glVertex2f(prevX, prevY);
        glVertex2f(x, y);
        prevX = x;
        prevY = y;
      }
      glEnd();

      if(state == Simulator::TurnoutState::State::Closed)
      {
        glColor3f(0.0f, 1.0f, 1.0f);

        glBegin(GL_LINES);
        glVertex2f(0, 0);
        glVertex2f(segment.straight.length, 0);
        glEnd();
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
  const float zoomFactor = (event->angleDelta().y() < 0) ? 0.9f : 1.1f; // Zoom in or out
  m_zoomLevel = std::clamp(m_zoomLevel * zoomFactor, 0.1f, 10.0f);
  updateProjection();
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
