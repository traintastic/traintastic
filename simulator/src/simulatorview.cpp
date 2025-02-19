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

inline static const std::array<ColorF, 3> colors{{
  {1.0f, 0.0f, 0.0f}, // Red
  {0.0f, 1.0f, 0.0f}, // Green
  {0.0f, 0.0f, 1.0f}, // Blue
}};
}

SimulatorView::SimulatorView(QWidget* parent)
  : QOpenGLWidget(parent)
{
  setFocusPolicy(Qt::StrongFocus); // for key stuff
}

Simulator* SimulatorView::simulator() const
{
  return m_simulator;
}

void SimulatorView::setSimulator(Simulator* value)
{
  if(m_simulator)
  {
    delete m_simulator;
  }
  if(value)
  {
    value->setParent(this); // take ownership
  }
  m_simulator = value;
  connect(m_simulator, &Simulator::tick, this, qOverload<>(&SimulatorView::update));
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
  for(const auto& segment : m_simulator->trackSegments())
  {
    if(m_showTrackOccupancy && segment.occupied && segment.sensorAddress)
    {
      glColor3f(1.0f, 0.0f, 0.0f); // Red if occupied
    }
    else
    {
      glColor3f(0.8f, 0.8f, 0.8f);
    }

    glPushMatrix();
    glTranslatef(segment.x, segment.y, 0);

    if(segment.type == Simulator::TrackSegment::Straight)
    {
      glRotatef(segment.rotation, 0, 0, 1);

      glBegin(GL_LINES);
      glVertex2f(0, 0);
      glVertex2f(segment.length, 0);
      glEnd();
    }
    else if(segment.type == Simulator::TrackSegment::Curve)
    {
      float startAngle = segment.rotation;
      float endAngle = segment.angle;
      int numSegments = qCeil(segment.angle / 5); // Smooth curve
      float step = endAngle / numSegments;
      float prevX = segment.radius * sinf(qDegreesToRadians(startAngle));
      float prevY = segment.radius * -cosf(qDegreesToRadians(startAngle));

      glBegin(GL_LINE_STRIP);
      for(int i = 1; i <= numSegments; i++)
      {
        float angle = startAngle + i * step;
        float x = segment.radius * sinf(qDegreesToRadians(angle));
        float y = segment.radius * -cosf(qDegreesToRadians(angle));
        glVertex2f(prevX, prevY);
        glVertex2f(x, y);
        prevX = x;
        prevY = y;
      }
      glEnd();
    }
    else if(segment.type == Simulator::TrackSegment::Turnout)
    {
      // glBegin(GL_LINES);
      // glVertex2f(0, 0);
      // glVertex2f(100, 0);
      // glVertex2f(100, 0);
      // glVertex2f(150, 50);
      // glEnd();
    }

    glPopMatrix();
  }
}

void SimulatorView::drawTrains()
{
  assert(m_simulator);

  static constexpr float trainWidth = 10.0f;

  for(const auto& train : m_simulator->trains())
  {
    for(const auto& vehicle : train.vehicles)
    {
      const auto center = (vehicle.positionFront + vehicle.positionRear) / 2;
      const auto delta = vehicle.positionFront - vehicle.positionRear;
      const float angle = qRadiansToDegrees(atan2f(delta.y, delta.x));

      glPushMatrix();
      glTranslatef(center.x, center.y, 0);
      glRotatef(angle, 0, 0, 1);

      const auto& color = colors[static_cast<size_t>(vehicle.color)];
      glColor3f(color.red, color.green, color.blue);
      glBegin(GL_QUADS);
      glVertex2f(-vehicle.length / 2, -trainWidth / 2);
      glVertex2f(vehicle.length / 2, -trainWidth / 2);
      glVertex2f(vehicle.length / 2, trainWidth / 2);
      glVertex2f(-vehicle.length / 2, trainWidth / 2);
      glEnd();

      glPopMatrix();
    }
  }
}

void SimulatorView::keyPressEvent(QKeyEvent* event)
{
  if(!m_simulator) [[unlikely]]
  {
    return QWidget::keyPressEvent(event);
  }

  auto& train = m_simulator->trains().front();
  switch(event->key())
  {
    case Qt::Key_Up:
      train.speed += 0.5f;
      break;
    case Qt::Key_Down:
      train.speed -= 0.5f;
      break;
    case Qt::Key_Space:
      train.speed = 0;
      break;
    default:
      return QWidget::keyPressEvent(event);
  }

  train.speed = std::clamp(train.speed, -train.speedMax, train.speedMax);
}

void SimulatorView::mousePressEvent(QMouseEvent* event)
{
  if(event->button() == Qt::LeftButton)
  {
    m_lastMousePos = event->pos();
  }
}

void SimulatorView::mouseMoveEvent(QMouseEvent* event)
{
  if(event->buttons() & Qt::LeftButton)
  {
    const auto diff = m_lastMousePos - event->pos();

    m_cameraX += diff.x() / m_zoomLevel;
    m_cameraY += diff.y() / m_zoomLevel;

    m_lastMousePos = event->pos();
    updateProjection();
  }
}

void SimulatorView::wheelEvent(QWheelEvent* event)
{
  const float zoomFactor = (event->angleDelta().y() < 0) ? 0.9f : 1.1f; // Zoom in or out
  m_zoomLevel = std::clamp(m_zoomLevel * zoomFactor, 0.1f, 10.0f);
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
