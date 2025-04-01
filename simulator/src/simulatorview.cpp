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

inline static const std::array<ColorF, 145> colors{{
  {0.94f, 0.97f, 1.00f}, //	Alice Blue
  {0.98f, 0.92f, 0.84f}, //	Antique White
  {0.00f, 1.00f, 1.00f}, //	Aqua
  {0.50f, 1.00f, 0.83f}, //	Aquamarine
  {0.94f, 1.00f, 1.00f}, //	Azure
  {0.96f, 0.96f, 0.86f}, //	Beige
  {1.00f, 0.89f, 0.77f}, //	Bisque
  {0.00f, 0.00f, 0.00f}, //	Black
  {1.00f, 0.92f, 0.80f}, //	Blanched Almond
  {0.00f, 0.00f, 1.00f}, //	Blue
  {0.54f, 0.17f, 0.89f}, //	Blue Violet
  {0.65f, 0.16f, 0.16f}, //	Brown
  {0.87f, 0.72f, 0.53f}, //	Burlywood
  {0.37f, 0.62f, 0.63f}, //	Cadet Blue
  {0.50f, 1.00f, 0.00f}, //	Chartreuse
  {0.82f, 0.41f, 0.12f}, //	Chocolate
  {1.00f, 0.50f, 0.31f}, //	Coral
  {0.39f, 0.58f, 0.93f}, //	Cornflower Blue
  {1.00f, 0.97f, 0.86f}, //	Cornsilk
  {0.86f, 0.08f, 0.24f}, //	Crimson
  {0.00f, 1.00f, 1.00f}, //	Cyan
  {0.00f, 0.00f, 0.55f}, //	Dark Blue
  {0.00f, 0.55f, 0.55f}, //	Dark Cyan
  {0.72f, 0.53f, 0.04f}, //	Dark Goldenrod
  {0.66f, 0.66f, 0.66f}, //	Dark Gray
  {0.00f, 0.39f, 0.00f}, //	Dark Green
  {0.74f, 0.72f, 0.42f}, //	Dark Khaki
  {0.55f, 0.00f, 0.55f}, //	Dark Magenta
  {0.33f, 0.42f, 0.18f}, //	Dark Olive Green
  {1.00f, 0.55f, 0.00f}, //	Dark Orange
  {0.60f, 0.20f, 0.80f}, //	Dark Orchid
  {0.55f, 0.00f, 0.00f}, //	Dark Red
  {0.91f, 0.59f, 0.48f}, //	Dark Salmon
  {0.56f, 0.74f, 0.56f}, //	Dark Sea Green
  {0.28f, 0.24f, 0.55f}, //	Dark Slate Blue
  {0.18f, 0.31f, 0.31f}, //	Dark Slate Gray
  {0.00f, 0.81f, 0.82f}, //	Dark Turquoise
  {0.58f, 0.00f, 0.83f}, //	Dark Violet
  {1.00f, 0.08f, 0.58f}, //	Deep Pink
  {0.00f, 0.75f, 1.00f}, //	Deep Sky Blue
  {0.41f, 0.41f, 0.41f}, //	Dim Gray
  {0.12f, 0.56f, 1.00f}, //	Dodger Blue
  {0.70f, 0.13f, 0.13f}, //	Firebrick
  {1.00f, 0.98f, 0.94f}, //	Floral White
  {0.13f, 0.55f, 0.13f}, //	Forest Green
  {1.00f, 0.00f, 1.00f}, //	Fuchsia
  {0.86f, 0.86f, 0.86f}, //	Gainsboro[a]
  {0.97f, 0.97f, 1.00f}, //	Ghost White
  {1.00f, 0.84f, 0.00f}, //	Gold
  {0.85f, 0.65f, 0.13f}, //	Goldenrod
  {0.75f, 0.75f, 0.75f}, //	Gray
  {0.50f, 0.50f, 0.50f}, //	Web Gray
  {0.00f, 1.00f, 0.00f}, //	Green
  {0.00f, 0.50f, 0.00f}, //	Web Green
  {0.68f, 1.00f, 0.18f}, //	Green Yellow
  {0.94f, 1.00f, 0.94f}, //	Honeydew
  {1.00f, 0.41f, 0.71f}, //	Hot Pink
  {0.80f, 0.36f, 0.36f}, //	Indian Red
  {0.29f, 0.00f, 0.51f}, //	Indigo
  {1.00f, 1.00f, 0.94f}, //	Ivory
  {0.94f, 0.90f, 0.55f}, //	Khaki
  {0.90f, 0.90f, 0.98f}, //	Lavender
  {1.00f, 0.94f, 0.96f}, //	Lavender Blush
  {0.49f, 0.99f, 0.00f}, //	Lawn Green
  {1.00f, 0.98f, 0.80f}, //	Lemon Chiffon
  {0.68f, 0.85f, 0.90f}, //	Light Blue
  {0.94f, 0.50f, 0.50f}, //	Light Coral
  {0.88f, 1.00f, 1.00f}, //	Light Cyan
  {0.98f, 0.98f, 0.82f}, //	Light Goldenrod
  {0.83f, 0.83f, 0.83f}, //	Light Gray
  {0.56f, 0.93f, 0.56f}, //	Light Green
  {1.00f, 0.71f, 0.76f}, //	Light Pink
  {1.00f, 0.63f, 0.48f}, //	Light Salmon
  {0.13f, 0.70f, 0.67f}, //	Light Sea Green
  {0.53f, 0.81f, 0.98f}, //	Light Sky Blue
  {0.47f, 0.53f, 0.60f}, //	Light Slate Gray
  {0.69f, 0.77f, 0.87f}, //	Light Steel Blue
  {1.00f, 1.00f, 0.88f}, //	Light Yellow
  {0.00f, 1.00f, 0.00f}, //	Lime
  {0.20f, 0.80f, 0.20f}, //	Lime Green
  {0.98f, 0.94f, 0.90f}, //	Linen
  {1.00f, 0.00f, 1.00f}, //	Magenta
  {0.69f, 0.19f, 0.38f}, //	Maroon
  {0.50f, 0.00f, 0.00f}, //	Web Maroon
  {0.40f, 0.80f, 0.67f}, //	Medium Aquamarine
  {0.00f, 0.00f, 0.80f}, //	Medium Blue
  {0.73f, 0.33f, 0.83f}, //	Medium Orchid
  {0.58f, 0.44f, 0.86f}, //	Medium Purple
  {0.24f, 0.70f, 0.44f}, //	Medium Sea Green
  {0.48f, 0.41f, 0.93f}, //	Medium Slate Blue
  {0.00f, 0.98f, 0.60f}, //	Medium Spring Green
  {0.28f, 0.82f, 0.80f}, //	Medium Turquoise
  {0.78f, 0.08f, 0.52f}, //	Medium Violet Red
  {0.10f, 0.10f, 0.44f}, //	Midnight Blue
  {0.96f, 1.00f, 0.98f}, //	Mint Cream
  {1.00f, 0.89f, 0.88f}, //	Misty Rose
  {1.00f, 0.89f, 0.71f}, //	Moccasin
  {1.00f, 0.87f, 0.68f}, //	Navajo White
  {0.00f, 0.00f, 0.50f}, //	Navy Blue
  {0.99f, 0.96f, 0.90f}, //	Old Lace
  {0.50f, 0.50f, 0.00f}, //	Olive
  {0.42f, 0.56f, 0.14f}, //	Olive Drab
  {1.00f, 0.65f, 0.00f}, //	Orange
  {1.00f, 0.27f, 0.00f}, //	Orange Red
  {0.85f, 0.44f, 0.84f}, //	Orchid
  {0.93f, 0.91f, 0.67f}, //	Pale Goldenrod
  {0.60f, 0.98f, 0.60f}, //	Pale Green
  {0.69f, 0.93f, 0.93f}, //	Pale Turquoise
  {0.86f, 0.44f, 0.58f}, //	Pale Violet Red
  {1.00f, 0.94f, 0.84f}, //	Papaya Whip
  {1.00f, 0.85f, 0.73f}, //	Peach Puff
  {0.80f, 0.52f, 0.25f}, //	Peru
  {1.00f, 0.75f, 0.80f}, //	Pink
  {0.87f, 0.63f, 0.87f}, //	Plum
  {0.69f, 0.88f, 0.90f}, //	Powder Blue
  {0.63f, 0.13f, 0.94f}, //	Purple
  {0.50f, 0.00f, 0.50f}, //	Web Purple
  {0.40f, 0.20f, 0.60f}, //	Rebecca Purple
  {1.00f, 0.00f, 0.00f}, //	Red
  {0.74f, 0.56f, 0.56f}, //	Rosy Brown
  {0.25f, 0.41f, 0.88f}, //	Royal Blue
  {0.55f, 0.27f, 0.07f}, //	Saddle brown
  {0.98f, 0.50f, 0.45f}, //	Salmon
  {0.96f, 0.64f, 0.38f}, //	Sandy Brown
  {0.18f, 0.55f, 0.34f}, //	Sea Green
  {1.00f, 0.96f, 0.93f}, //	Seashell
  {0.63f, 0.32f, 0.18f}, //	Sienna
  {0.75f, 0.75f, 0.75f}, //	Silver
  {0.53f, 0.81f, 0.92f}, //	Sky Blue
  {0.42f, 0.35f, 0.80f}, //	Slate Blue
  {0.44f, 0.50f, 0.56f}, //	Slate Gray
  {1.00f, 0.98f, 0.98f}, //	Snow
  {0.00f, 1.00f, 0.50f}, //	Spring Green
  {0.27f, 0.51f, 0.71f}, //	Steel Blue
  {0.82f, 0.71f, 0.55f}, //	Tan
  {0.00f, 0.50f, 0.50f}, //	Teal
  {0.85f, 0.75f, 0.85f}, //	Thistle
  {1.00f, 0.39f, 0.28f}, //	Tomato
  {0.25f, 0.88f, 0.82f}, //	Turquoise
  {0.93f, 0.51f, 0.93f}, //	Violet
  {0.96f, 0.87f, 0.70f}, //	Wheat
  {1.00f, 1.00f, 1.00f}, //	White
  {0.96f, 0.96f, 0.96f}, //	White Smoke
  {1.00f, 1.00f, 0.00f}, //	Yellow
  {0.60f, 0.80f, 0.20f}, //	Yellow Green
}};

float crossProduct(Simulator::Point p1, Simulator::Point p2, Simulator::Point p3)
{
  return (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
}

bool isPointInTriangle(const Simulator::Point triangle[3], const Simulator::Point point)
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
  m_turnouts.clear();
  if(m_simulator)
  {
    for(const auto& segment : m_simulator->trackSegments())
    {
      if(segment.type == Simulator::TrackSegment::Type::Turnout)
      {
        m_turnouts.emplace_back(Turnout{segment.index, {origin(segment), straightEnd(segment), curveEnd(segment)}});
      }
    }
    connect(m_simulator, &Simulator::tick, this, qOverload<>(&SimulatorView::update));
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
  for(const auto& segment : m_simulator->trackSegments())
  {
    if(m_showTrackOccupancy && m_simulator->isSensorOccupied(segment.sensorIndex))
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
      glVertex2f(segment.straight.length, 0);
      glEnd();
    }
    else if(segment.type == Simulator::TrackSegment::Curve)
    {
      float startAngle = segment.rotation;
      if(segment.curve.angle < 0)
      {
        startAngle += 180;
      }
      float endAngle = segment.curve.angle;
      int numSegments = qCeil(std::abs(segment.curve.angle) / 5); // Smooth curve
      float step = endAngle / numSegments;
      float prevX = segment.curve.radius * sinf(qDegreesToRadians(startAngle));
      float prevY = segment.curve.radius * -cosf(qDegreesToRadians(startAngle));

      glBegin(GL_LINE_STRIP);
      for(int i = 1; i <= numSegments; i++)
      {
        float angle = startAngle + i * step;
        float x = segment.curve.radius * sinf(qDegreesToRadians(angle));
        float y = segment.curve.radius * -cosf(qDegreesToRadians(angle));
        glVertex2f(prevX, prevY);
        glVertex2f(x, y);
        prevX = x;
        prevY = y;
      }
      glEnd();
    }
    else if(segment.type == Simulator::TrackSegment::Turnout)
    {
      glRotatef(segment.rotation, 0, 0, 1);

      if(segment.turnout.thrown)
      {
        glBegin(GL_LINES);
        glVertex2f(segment.straight.length, 0);
        glVertex2f(0, 0);
        glEnd();

        glColor3f(0.0f, 1.0f, 1.0f);
      }

      const float rotation = segment.curve.angle < 0 ? 0.0f : 180.0f;

      int numSegments = qCeil(std::abs(segment.curve.angle) / 5); // Smooth curve
      float step = segment.curve.angle / numSegments;
      float prevX = 0.0f;
      float prevY = 0.0f;
      const float cx = segment.curve.radius * sinf(qDegreesToRadians(rotation));
      const float cy = segment.curve.radius * -cosf(qDegreesToRadians(rotation));

      glBegin(GL_LINE_STRIP);
      for(int i = 1; i <= numSegments; i++)
      {
        float angle = rotation + i * step;
        float x = cx - segment.curve.radius * sinf(qDegreesToRadians(angle));
        float y = cy - segment.curve.radius * -cosf(qDegreesToRadians(angle));
        glVertex2f(prevX, prevY);
        glVertex2f(x, y);
        prevX = x;
        prevY = y;
      }
      glEnd();

      if(!segment.turnout.thrown)
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

  const float trainWidth = m_simulator->trainWidth();

  for(const auto& train : m_simulator->trains())
  {
    for(const auto& vehicle : train.vehicles)
    {
      const auto center = (vehicle.front.position + vehicle.rear.position) / 2;
      const auto delta = vehicle.front.position - vehicle.rear.position;
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

  auto& train = m_simulator->trains()[m_trainIndex];
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
      if(static_cast<size_t>(event->key() - Qt::Key_1) < m_simulator->trains().size())
      {
        m_trainIndex = event->key() - Qt::Key_1;
      }
      break;
    case Qt::Key_Up:
      train.setSpeed(train.speed + 0.5f);
      break;
    case Qt::Key_Down:
      train.setSpeed(train.speed - 0.5f);
      break;
    case Qt::Key_Right:
      train.setDirection(Direction::Forward);
      break;
    case Qt::Key_Left:
      train.setDirection(Direction::Reverse);
      break;
    case Qt::Key_Space:
      train.setSpeed(0.0f);
      break;
    case Qt::Key_Escape: // stop all trains
      for(auto& t : m_simulator->trains())
      {
        t.setSpeed(0.0f);
      }
      break;

    case Qt::Key_P:
      m_simulator->setPowerOn(!m_simulator->powerOn());
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
      auto& segment = m_simulator->trackSegments()[turnout.segmentIndex];
      assert(segment.type == Simulator::TrackSegment::Type::Turnout);
      m_simulator->setTurnoutThrow(segment.index, !segment.turnout.thrown);
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
