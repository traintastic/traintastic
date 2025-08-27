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

#include <QPainter>
#include <QToolTip>
#include <QGuiApplication>

#include <QDir>
#include <QFileInfo>

#include <QMenu>
#include <QClipboard>

#include <QVector>

namespace
{

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

bool lineContains(const QPointF& pos,
                  const QPointF& a, const QPointF& b,
                  float &distanceOut,
                  const float tolerance = 1.0)
{
    QPointF topLeft = a, bottomRight = b;
    if(topLeft.x() > bottomRight.x())
        std::swap(topLeft.rx(), bottomRight.rx());
    if(topLeft.y() < bottomRight.y())
        std::swap(topLeft.ry(), bottomRight.ry());


    if(std::abs(a.y() - b.y()) < 0.0001)
    {
        // Horizontal
        const float yDist = std::abs(a.y() - pos.y());
        const float leftDist = topLeft.x() - pos.x();
        const float rightDist = pos.x() - bottomRight.x();
        if(yDist < tolerance &&
                (topLeft.x() - tolerance) <= pos.x() && (bottomRight.x() + tolerance) >= pos.x())
        {
            const float minOutDist = -std::min(leftDist, rightDist);
            distanceOut = yDist;
            if(minOutDist > yDist)
                distanceOut = minOutDist;
            return true;
        }

        return false;
    }

    if(std::abs(a.x() - b.x()) < 0.0001)
    {
        // Vertical
        return std::abs(a.x() - pos.x()) < tolerance &&
                (a.y() - tolerance) <= pos.y() && (b.y() + tolerance) >= pos.y();

        const float xDist = std::abs(a.x() - pos.x());
        const float leftDist = pos.y() - topLeft.y();
        const float rightDist = bottomRight.y() - pos.y();
        if(xDist < tolerance &&
                (a.y() - tolerance) <= pos.y() && (b.y() + tolerance) >= pos.y())
        {
            const float minOutDist = -std::min(leftDist, rightDist);
            distanceOut = xDist;
            if(minOutDist > xDist)
                distanceOut = minOutDist;
            return true;
        }

        return false;
    }

    // Diagonal
    const float resultingY  = a.y() + (pos.x() - a.x()) * (b.y() - a.y()) / (b.x() - a.x());
    distanceOut = std::abs(resultingY - pos.y());
    return distanceOut <= tolerance;
}

size_t getSegmentAt(const Simulator::Point &point, const Simulator::StaticData &data)
{
  size_t bestIdx = Simulator::invalidIndex;
  float bestDistance = 0.0;

  for (size_t idx = 0; idx < data.trackSegments.size(); idx++)
  {
    const auto &segment = data.trackSegments.at(idx);

    switch (segment.type)
    {
      case Simulator::TrackSegment::Type::Turnout:
      case Simulator::TrackSegment::Type::TurnoutCurved:
      case Simulator::TrackSegment::Type::Turnout3Way:
      {
        std::span<const Simulator::Point, 3> points(
            {segment.points[0], segment.points[1], segment.points[2]});

        if (isPointInTriangle(points, point))
          return idx;

        if (segment.type == Simulator::TrackSegment::Type::Turnout3Way)
        {
          // Check also other curve
          const Simulator::Point arr[3] = {segment.points[0],
                                           segment.points[1],
                                           segment.points[3]};
          if (isPointInTriangle(arr, point))
            return idx;
        }

        continue;
      }
      case Simulator::TrackSegment::Type::Straight:
      {
        QPointF pos(point.x, point.y);

        QPointF a(segment.points[0].x, segment.points[0].y);
        QPointF b(segment.points[1].x, segment.points[1].y);

        QRectF br;
        br.setTop(segment.points[0].y);
        br.setLeft(segment.points[0].x);
        br.setBottom(segment.points[1].y);
        br.setRight(segment.points[1].x);
        br = br.normalized();

        const QRectF brAdj = br.adjusted(-2.5, -2.5, 2.5, 2.5);

        if (br.width() > 0.0001 && br.height() > 0.0001 && !brAdj.contains(pos))
          continue;

        float segDistance = 0;
        if (!lineContains(pos, a, b, segDistance, 5))
          continue;

        if (bestIdx == Simulator::invalidIndex || segDistance < bestDistance)
        {
          bestIdx = idx;
          bestDistance = segDistance;
        }
        continue;
      }
      case Simulator::TrackSegment::Type::Curve:
      {
        const Simulator::Point center = segment.curves[0].center;
        const Simulator::Point diff = point - center;
        const float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);

        if (std::abs(distance - segment.curves[0].radius) > 5)
          continue;

        if (bestIdx != Simulator::invalidIndex && distance > bestDistance)
          continue;

        // Y coordinate is swapped
        float angle = std::atan2(-diff.y, diff.x);

        float rotation = segment.rotation;
        if (rotation < 0)
          rotation += 2 * pi;

        const float curveAngle = segment.curves[0].angle;
        float angleMax = -rotation + pi / 2.0 * (curveAngle > 0 ? 1 : -1);
        float angleMin = -rotation - curveAngle + pi / 2.0 * (curveAngle > 0 ? 1 : -1);

        if (curveAngle < 0)
          std::swap(angleMin, angleMax);

        if (angleMin < 0)
        {
          angleMin += 2 * pi;
          angleMax += 2 * pi;
        }

        if (angleMin < 0 && angle < 0)
        {
          angleMin += 2 * pi;
          angleMax += 2 * pi;
        }

        if (angle < 0)
        {
          angle += 2 * pi;
        }

        // TODO: really ugly...
        if (angleMin <= angleMax)
        {
          // min -> max
          if (angle < angleMin || angle > angleMax)
          {
            // Try again with + 2 * pi
            const float angleBis = angle += 2 * pi;
            if (angleBis < angleMin || angleBis > angleMax)
            {
                // Try again with - 2 * pi
                const float angleTer = angle -= 2 * pi;
                if (angleTer < angleMin || angleTer > angleMax)
                {
                    continue;
                }
            }
          }
        }
        else
        {
          // 0 -> min, max -> 2 * pi
          if (angle > angleMin && angle < angleMax)
          {
            // Try again with + 2 * pi
            const float angleBis = angle += 2 * pi;
            if (angleBis > angleMin && angleBis < angleMax)
            {
                // Try again with - 2 * pi
                const float angleTer = angle -= 2 * pi;
                if (angleTer > angleMin && angleTer < angleMax)
                {
                    continue;
                }
            }
          }
        }

        bestIdx = idx;
        bestDistance = distance;
        continue;
      }
      default:
        break;
      }
    }

    return bestIdx;
}

void drawStraight(const Simulator::TrackSegment& segment, QPainter *painter)
{
  painter->drawLine(0, 0, segment.straight.length, 0);
}

void drawCurve(const Simulator::TrackSegment& segment, size_t curveIndex, QPainter *painter)
{
  const auto& curve = segment.curves[curveIndex];
  const float rotation = curve.angle < 0 ? 0.0f : pi;

  int numSegments = qCeil(curve.length / 10.0); // Smooth curve
  float step = curve.angle / numSegments;
  const float cx = curve.radius * sinf(rotation);
  const float cy = curve.radius * -cosf(rotation);

  QVector<QPointF> pointVec;
  pointVec.reserve(numSegments + 1);
  pointVec.append({0, 0});

  for(int i = 1; i <= numSegments; i++)
  {
    float angle = rotation + i * step;
    float x = cx - curve.radius * sinf(angle);
    float y = cy - curve.radius * -cosf(angle);
    pointVec.append({x, y});
  }

  painter->drawPolyline(pointVec.data(), pointVec.size());
}

}

SimulatorView::SimulatorView(QWidget* parent)
  : QOpenGLWidget(parent)
{
  setFocusPolicy(Qt::StrongFocus); // for key stuff

  // 800 ms turnout blink
  turnoutBlinkTimer.start(std::chrono::milliseconds(800), Qt::PreciseTimer, this);

  setContextMenuPolicy(Qt::DefaultContextMenu);

  setMouseTracking(true);
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

void SimulatorView::loadExtraImages(const nlohmann::json& world,
                                    const QString& imagesFile,
                                    QStringList &namesOut)
{
  m_extraImages.clear();

  const QDir fileDir = QFileInfo(imagesFile).absoluteDir();

  if(auto images = world.find("images"); images != world.end() && images->is_array())
  {
    for(const auto& object : *images)
    {
      if(!object.is_object())
      {
        continue;
      }

      Simulator::ImageRef item;

      item.origin.x = object.value("x", std::numeric_limits<float>::quiet_NaN());
      item.origin.y = object.value("y", std::numeric_limits<float>::quiet_NaN());
      item.fileName = object.value<std::string_view>("file", {});
      item.rotation = deg2rad(object.value("rotation", 0.0f));
      item.opacity = object.value("opacity", 1.0);

      const float pxCount = object.value("n_px", std::numeric_limits<float>::quiet_NaN());
      const float mtCount = object.value("n_mt", std::numeric_limits<float>::quiet_NaN());

      if(!item.origin.isFinite() || item.fileName.empty() || pxCount == 0)
      {
        continue;
      }

      item.ratio = mtCount / pxCount;

      Image img;
      img.ref = item;

      QString fileName = QString::fromStdString(img.ref.fileName);
      QFileInfo info(fileName);
      if(info.isRelative())
      {
        // Treat as relative to image JSON file
        fileName = fileDir.absoluteFilePath(fileName);
      }

      if(!img.img.load(fileName))
        continue;

      m_extraImages.push_back(img);

      namesOut.append(info.fileName());
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

void SimulatorView::setCamera(const Simulator::Point &cameraPt)
{
  m_cameraX = cameraPt.x;
  m_cameraY = cameraPt.y;
  updateProjection();
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

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.scale(m_zoomLevel, m_zoomLevel);
    painter.translate(-m_cameraX, -m_cameraY);

    const QTransform trasf = painter.transform();

    if(!m_images.empty() || !m_extraImages.empty())
    {
        for(const auto &image : m_extraImages)
        {
            if(!image.visible)
                continue;
            painter.setOpacity(image.ref.opacity);
            painter.translate(image.ref.origin.x, image.ref.origin.y);
            painter.rotate(qRadiansToDegrees(image.ref.rotation));
            painter.scale(image.ref.ratio, image.ref.ratio);
            painter.drawImage(QPoint(), image.img);
            painter.setTransform(trasf);
        }

        for(const auto &image : m_images)
        {
            painter.setOpacity(image.ref.opacity);
            painter.translate(image.ref.origin.x, image.ref.origin.y);
            painter.rotate(qRadiansToDegrees(image.ref.rotation));
            painter.scale(image.ref.ratio, image.ref.ratio);
            painter.drawImage(QPoint(), image.img);
            painter.setTransform(trasf);
        }

        painter.setOpacity(1);
    }


    if(m_simulator) [[likely]]
    {
        drawMisc(&painter);
        drawTracks(&painter);
        drawTrackObjects(&painter);
        drawTrains(&painter);
    }
}

void SimulatorView::drawTracks(QPainter *painter)
{
    assert(m_simulator);

    QPen trackPen(QColor(204, 204, 204), 1.1);
    trackPen.setCapStyle(Qt::FlatCap);

    QPen trackPenOccupied = trackPen;
    trackPenOccupied.setColor(QColor(255, 0, 0));

    QPen trackPenGreen = trackPen;
    trackPenGreen.setColor(QColor(0, 255, 0));

    QPen trackPenPurple = trackPen;
    trackPenPurple.setColor(QColor(128, 0, 255));

    QPen trackPenCyan = trackPen;
    trackPenCyan.setColor(QColor(0, 255, 255));
    trackPenCyan.setWidthF(0.5);
    trackPenCyan.setCosmetic(false);

    QPen trackPenYellow = trackPenCyan;
    trackPenYellow.setColor(Qt::darkYellow);

    const QTransform trasf = painter->transform();

    size_t idx = 0;
    for(const auto& segment : m_simulator->staticData.trackSegments)
    {
        if(m_showTrackOccupancy && segment.hasSensor() && m_stateData.sensors[segment.sensor.index].value)
        {
            // Red if occupied
            painter->setPen(trackPenOccupied);
        }
        else
        {
            painter->setPen(trackPen);
        }

        if(!m_stateData.powerOn)
        {
            // Red if not powered for better contrast
            painter->setPen(trackPenOccupied);

            if(idx == m_hoverSegmentIdx)
            {
                // Green on hover
                painter->setPen(trackPenGreen);
            }
            else if(segment.hasSensor() && segment.sensor.index == m_hoverSensorIdx)
            {
                // Blue on same hovered sensor
                painter->setPen(trackPenPurple);
            }
        }

        painter->translate(segment.origin().x, segment.origin().y);
        painter->rotate(qRadiansToDegrees(segment.rotation));

        if(segment.type == Simulator::TrackSegment::Type::Straight)
        {
            drawStraight(segment, painter);
        }
        else if(segment.type == Simulator::TrackSegment::Type::Curve)
        {
            drawCurve(segment, 0, painter);
        }
        else if(segment.type == Simulator::TrackSegment::Type::Turnout)
        {
            drawCurve(segment, 0, painter);
            drawStraight(segment, painter);
        }
        else if(segment.type == Simulator::TrackSegment::Type::TurnoutCurved)
        {
            drawCurve(segment, 1, painter);
            drawCurve(segment, 0, painter);
        }
        else if(segment.type == Simulator::TrackSegment::Type::Turnout3Way)
        {
            drawCurve(segment, 0, painter);
            drawCurve(segment, 1, painter);
            drawStraight(segment, painter);
        }

        painter->setTransform(trasf);
        idx++;
    }

    // Redraw on top turnout current state
    for(const auto& segment : m_simulator->staticData.trackSegments)
    {
        if(segment.type == Simulator::TrackSegment::Type::Straight ||
            segment.type == Simulator::TrackSegment::Type::Curve)
            continue;

        painter->translate(segment.origin().x, segment.origin().y);
        painter->rotate(qRadiansToDegrees(segment.rotation));

        assert(segment.turnout.index < m_stateData.turnouts.size());
        const auto state = m_stateData.turnouts[segment.turnout.index].state;

        if(m_showTrackOccupancy && segment.hasSensor() && m_stateData.sensors[segment.sensor.index].value)
        {
            // Cyan contrast on red
            painter->setPen(trackPenCyan);
        }
        else
        {
            // Dark yellow contrast on white
            painter->setPen(trackPenYellow);
        }

        if(segment.type == Simulator::TrackSegment::Type::Turnout)
        {
            switch(state)
            {
            case Simulator::TurnoutState::State::Unknown:
                // Blink cyan or normal
                if(turnoutBlinkState)
                {
                    drawCurve(segment, 0, painter);
                    drawStraight(segment, painter);
                }
                break;

            case Simulator::TurnoutState::State::Closed:
                drawStraight(segment, painter);
                break;

            case Simulator::TurnoutState::State::Thrown:
                drawCurve(segment, 0, painter);
                break;

            default:
                assert(false);
                break;
            }
        }
        else if(segment.type == Simulator::TrackSegment::Type::TurnoutCurved)
        {
            switch(state)
            {
            case Simulator::TurnoutState::State::Unknown:
                // Blink cyan or normal
                if(turnoutBlinkState)
                {
                    drawCurve(segment, 0, painter);
                    drawCurve(segment, 1, painter);
                }
                break;

            case Simulator::TurnoutState::State::Closed:
                drawCurve(segment, 0, painter);
                break;

            case Simulator::TurnoutState::State::Thrown:
                drawCurve(segment, 1, painter);
                break;

            default:
                assert(false);
                break;
            }
        }
        else if(segment.type == Simulator::TrackSegment::Type::Turnout3Way)
        {
            switch(state)
            {
            case Simulator::TurnoutState::State::Unknown:
                // Blink cyan or normal
                if(turnoutBlinkState)
                {
                    drawCurve(segment, 0, painter);
                    drawCurve(segment, 1, painter);
                    drawStraight(segment, painter);
                }
                break;

            case Simulator::TurnoutState::State::Closed:
                drawStraight(segment, painter);
                break;

            case Simulator::TurnoutState::State::ThrownLeft:
                drawCurve(segment, 0, painter);
                break;

            case Simulator::TurnoutState::State::ThrownRight:
                drawCurve(segment, 1, painter);
                break;

            default:
                assert(false);
                break;
            }
        }

        painter->setTransform(trasf);
        idx++;
    }
}

void SimulatorView::drawTrackObjects(QPainter *painter)
{
    assert(m_simulator);

    QColor positionSensorActive = Qt::red;
    QColor positionSensorInactive = Qt::darkGreen;

    const QPen signalMastPen(Qt::gray, 1.1);
    const QPen signalLightPen(Qt::gray, 0.5);

    const QTransform trasf = painter->transform();

    size_t idx = 0;
    for(const auto& segment : m_simulator->staticData.trackSegments)
    {
        using Object = Simulator::TrackSegment::Object;

        for(const Object& obj : segment.objects)
        {
            painter->translate(obj.pos.x, obj.pos.y);
            painter->rotate(qRadiansToDegrees(obj.rotation + obj.dirForward ? 0 : pi));

            switch (obj.type)
            {
            case Object::Type::PositionSensor:
            {
                QColor color = positionSensorInactive;
                if(m_stateData.sensors[obj.sensorIndex].value)
                    color = positionSensorActive;

                QRectF r;
                r.setSize(QSizeF(4, 2));
                r.moveCenter(QPointF(0, obj.lateralDiff));
                painter->fillRect(r, color);
                break;
            }
            case Object::Type::MainSignal:
            {
                auto signIt = m_stateData.mainSignals.find(obj.signalName);
                if(signIt == m_stateData.mainSignals.end())
                    continue;

                Simulator::MainSignal *signal = signIt->second;
                const int mastLength = 8 + 6 * (signal->lights.size() - 1);

                painter->setPen(signalMastPen);
                painter->drawLine(QLineF(0, obj.lateralDiff,
                                         mastLength, obj.lateralDiff));

                QRectF lightRect;
                lightRect.setSize(QSizeF(5, 5));
                lightRect.moveCenter(QPointF(10, obj.lateralDiff));

                for(size_t i = 0; i < signal->lights.size(); i++)
                {
                    painter->setBrush(Qt::red);
                    painter->drawEllipse(lightRect);

                    lightRect.moveLeft(lightRect.left() + 6);
                }

                break;
            }
            default:
                break;
            }


            painter->setTransform(trasf);
        }

        idx++;
    }
}

void SimulatorView::drawTrains(QPainter *painter)
{
    assert(m_simulator);

    const QTransform trasf = painter->transform();

    const float trainWidth = m_simulator->staticData.trainWidth;

    painter->setPen(Qt::NoPen);

    for(auto it : m_stateData.vehicles)
    {
        const auto* vehicle = it.second;
        const auto& vehicleState = vehicle->state;
        const float length = vehicle->length;

        const auto center = (vehicleState.front.position + vehicleState.rear.position) / 2;
        const auto delta = vehicleState.front.position - vehicleState.rear.position;
        const float angle = atan2f(delta.y, delta.x);

        painter->translate(center.x, center.y);
        painter->rotate(qRadiansToDegrees(angle));

        const auto& color = colors[static_cast<size_t>(vehicle->color)];
        QRectF veichleRect(-length / 2, -trainWidth / 2,
                           length, trainWidth);
        painter->fillRect(veichleRect,
                          QColor(color.red * 255, color.green * 255, color.blue * 255));

        painter->setTransform(trasf);
    }
}

void SimulatorView::drawMisc(QPainter *painter)
{
  assert(m_simulator);

  QPen miscPen;
  miscPen.setWidth(1);
  miscPen.setCosmetic(false);

  const QTransform trasf = painter->transform();

  for(const auto& item : m_simulator->staticData.misc)
  {
    const auto& color = colors[static_cast<size_t>(item.color)];
    miscPen.setColor(QColor(color.red * 255, color.green * 255, color.blue * 255));
    painter->setPen(miscPen);

    switch(item.type)
    {
      case Simulator::Misc::Type::Rectangle:
        painter->translate(item.origin.x, item.origin.y);
        painter->rotate(qRadiansToDegrees(item.rotation));

        painter->drawRect(QRectF(0, 0, item.width, item.height));

        painter->setTransform(trasf);
        break;
    }
  }
}

bool SimulatorView::event(QEvent *e)
{
    if(e->type() == QEvent::ToolTip)
    {
        QHelpEvent *ev = static_cast<QHelpEvent *>(e);
        showItemTooltip(mapToSim(ev->pos()), ev);
        return true;
    }

    return QOpenGLWidget::event(e);
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
    {
      const size_t trainIndex = static_cast<size_t>(event->key() - Qt::Key_1);
      std::lock_guard<std::recursive_mutex> lock(m_simulator->stateMutex());
      Simulator::Train *train = m_simulator->getTrainAt(trainIndex);
      if(train)
      {
        m_trainIndex = event->key() - Qt::Key_1;
      }
      break;
    }
    case Qt::Key_Up:
    {
      std::lock_guard<std::recursive_mutex> lock(m_simulator->stateMutex());
      Simulator::Train *train = m_simulator->getTrainAt(m_trainIndex);
      if(train)
        m_simulator->applyTrainSpeedDelta(train, train->speedMax / 20);
      break;
    }
    case Qt::Key_Down:
    {
      std::lock_guard<std::recursive_mutex> lock(m_simulator->stateMutex());
      Simulator::Train *train = m_simulator->getTrainAt(m_trainIndex);
      if(train)
        m_simulator->applyTrainSpeedDelta(train, -train->speedMax / 20);
      break;
    }
    case Qt::Key_Right:
    case Qt::Key_Left:
    {
      std::lock_guard<std::recursive_mutex> lock(m_simulator->stateMutex());
      Simulator::Train *train = m_simulator->getTrainAt(m_trainIndex);
      if(train)
      {
        bool dir = (event->key() == Qt::Key_Left);
        if(m_simulator->isTrainDirectionInverted(train))
          dir = !dir;
        m_simulator->setTrainDirection(train, dir);
      }
      break;
    }

    case Qt::Key_Space:
    {
      std::lock_guard<std::recursive_mutex> lock(m_simulator->stateMutex());
      Simulator::Train *train = m_simulator->getTrainAt(m_trainIndex);
      if(train)
        m_simulator->setTrainSpeed(train, 0.0f);
      break;
    }
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
    resetSegmentHover();
  }
  if(event->button() == Qt::RightButton)
  {
    m_rightMousePos = event->pos();

    if(event->modifiers() != Qt::ControlModifier)
      setCursor(Qt::ClosedHandCursor);

    resetSegmentHover();
  }
}

void SimulatorView::mouseMoveEvent(QMouseEvent* event)
{
  if(event->buttons() & Qt::RightButton && event->modifiers() != Qt::ControlModifier)
  {
    const auto diff = m_rightMousePos - event->pos();

    m_cameraX += diff.x() / m_zoomLevel;
    m_cameraY += diff.y() / m_zoomLevel;

    m_rightMousePos = event->pos();
    updateProjection();
  }
  else if(event->buttons() == Qt::NoButton && m_simulator && !m_stateData.powerOn)
  {
    // Refresh hovered segment every 100 ms
    m_lastHoverPos = mapToSim(event->pos());
    if(!segmentHoverTimer.isActive())
        segmentHoverTimer.start(std::chrono::milliseconds(100), this);
  }
}

void SimulatorView::mouseReleaseEvent(QMouseEvent* event)
{
  if(event->button() == Qt::LeftButton)
  {
    auto diff = m_leftClickMousePos - event->pos();
    if(std::abs(diff.x()) <= 2 && std::abs(diff.y()) <= 2)
    {
      const bool shiftPressed = event->modifiers().testFlag(Qt::ShiftModifier);
      mouseLeftClick(mapToSim(m_leftClickMousePos), shiftPressed);
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

void SimulatorView::timerEvent(QTimerEvent *e)
{
  if(e->timerId() == turnoutBlinkTimer.timerId())
  {
    turnoutBlinkState = !turnoutBlinkState;
    update();
    return;
  }
  else if(e->timerId() == segmentHoverTimer.timerId())
  {
    size_t newHoverSegment = getSegmentAt(m_lastHoverPos, m_simulator->staticData);
    if(m_hoverSegmentIdx != newHoverSegment)
    {
      m_hoverSegmentIdx = newHoverSegment;
      if(m_hoverSegmentIdx != Simulator::invalidIndex)
      {
          const auto& segment = m_simulator->staticData.trackSegments[m_hoverSegmentIdx];
          m_hoverSensorIdx = segment.sensor.index;
      }
      else
      {
          // Reset also sensor if not hover
          m_hoverSensorIdx = Simulator::invalidIndex;
      }
    }
  }

  QOpenGLWidget::timerEvent(e);
}

void SimulatorView::contextMenuEvent(QContextMenuEvent *e)
{
  if(e->modifiers() != Qt::ControlModifier)
    return; // Use control to distinguish from right click pan

  const Simulator::Point point = mapToSim(e->pos());
  const size_t idx = getSegmentAt(point, m_simulator->staticData);
  if(idx == Simulator::invalidIndex)
    return;

  QMenu *m = new QMenu(this);
  QAction *copySegData = m->addAction(tr("Copy segment data"));
  QAction *result = m->exec(e->globalPos());
  if(result == copySegData)
  {
    const auto obj = copySegmentData(idx);
    QGuiApplication::clipboard()->setText(QString::fromStdString(obj.dump(2)));
  }
}

void SimulatorView::mouseLeftClick(const Simulator::Point &point, bool shiftPressed)
{
  for(const auto& turnout : m_turnouts)
  {
    if(isPointInTriangle(turnout.points, point))
    {
      m_simulator->toggleTurnoutState(turnout.segmentIndex, shiftPressed);
      update();
      break;
    }
  }
}

void SimulatorView::showItemTooltip(const Simulator::Point &point, QHelpEvent *ev)
{
  if (QGuiApplication::keyboardModifiers().testFlag(Qt::ShiftModifier))
  {
    // Cursor position tooltip
    QString text = tr("X: %1\n"
                      "Y: %2")
                       .arg(point.x)
                       .arg(point.y);
    QToolTip::showText(ev->globalPos(), text, this);
    return;
  }

  const auto &data_ = m_simulator->staticData;
  QString text;

  int ptCount = 0;
  auto addPt = [&ptCount, &text](const QString &name, const Simulator::Point &p)
  {
      ptCount++;
      QString ptText
          = tr("%1: %2; %3").arg(name.isEmpty() ? QString::number(ptCount) : name).arg(p.x).arg(p.y);
      text.append("<br>");
      text.append(ptText);
  };

  const size_t idx = getSegmentAt(point, data_);
  if (idx == Simulator::invalidIndex)
  {
    QToolTip::hideText();
    return;
  }

  const auto &segment = data_.trackSegments.at(idx);
  switch (segment.type)
  {
  case Simulator::TrackSegment::Type::Straight:
  {
    text = tr("Straight: <b>%1</b><br>").arg(QString::fromStdString(segment.m_id));
    addPt("orig", segment.points[0]);
    addPt("end", segment.points[1]);
    text.append("<br>");
    text.append(tr("rotation: %1<br>").arg(qRadiansToDegrees(segment.rotation)));
    text.append("<br>");
    text.append(tr("strai_l: %1<br>").arg(segment.straight.length));
    break;
  }
  case Simulator::TrackSegment::Type::Curve:
  {
    text = tr("Curve: <b>%1</b><br>").arg(QString::fromStdString(segment.m_id));
    addPt("orig", segment.points[0]);
    addPt("end", segment.points[1]);
    addPt("center", segment.curves[0].center);
    text.append("<br>");
    text.append(tr("rotation: %1<br>").arg(qRadiansToDegrees(segment.rotation)));
    text.append("<br>");
    text.append(tr("radius: %1<br>").arg(segment.curves[0].radius));
    text.append(tr("angle: %1<br>").arg(qRadiansToDegrees(segment.curves[0].angle)));
    text.append(tr("curve_l: %1<br>").arg(segment.curves[0].length));
    break;
  }
  case Simulator::TrackSegment::Type::Turnout:
  case Simulator::TrackSegment::Type::Turnout3Way:
  case Simulator::TrackSegment::Type::TurnoutCurved:
  {
    QLatin1String segTypeName = QLatin1String("Turnout");
    if(segment.type == Simulator::TrackSegment::Type::Turnout3Way)
      segTypeName = QLatin1String("Turnout 3 Way");
    else if(segment.type == Simulator::TrackSegment::Type::TurnoutCurved)
      segTypeName = QLatin1String("Turnout Curved");

    text = tr("%1: <b>%2</b><br>").arg(segTypeName, QString::fromStdString(segment.m_id));
    addPt("orig", segment.points[0]);

    if(segment.type == Simulator::TrackSegment::Type::Turnout3Way)
    {
      addPt("straight", segment.points[1]);
      addPt("curve 0", segment.points[2]);
      addPt("curve 1", segment.points[3]);
    }
    else if(segment.type == Simulator::TrackSegment::Type::TurnoutCurved)
    {
      addPt("curve 0", segment.points[1]);
      addPt("curve 1", segment.points[2]);
    }
    else
    {
      addPt("straight", segment.points[1]);
      addPt("curve", segment.points[2]);
    }

    text.append("<br>");
    text.append(tr("rotation: %1<br>").arg(qRadiansToDegrees(segment.rotation)));
    text.append("<br>");

    if(segment.type == Simulator::TrackSegment::Type::Turnout3Way)
    {
      text.append(tr("radius 0: %1<br>").arg(segment.curves[0].radius));
      text.append(tr("angle 0: %1<br>").arg(qRadiansToDegrees(segment.curves[0].angle)));
      text.append(tr("curve_l 0: %1<br>").arg(segment.curves[0].length));
      text.append("<br>");
      text.append(tr("radius 1: %1<br>").arg(segment.curves[1].radius));
      text.append(tr("angle 1: %1<br>").arg(qRadiansToDegrees(segment.curves[1].angle)));
      text.append(tr("curve_l 1: %1<br>").arg(segment.curves[1].length));
      text.append("<br>");
      text.append(tr("strai_l: %1<br>").arg(segment.straight.length));
    }
    else if(segment.type == Simulator::TrackSegment::Type::TurnoutCurved)
    {
      text.append(tr("radius 0: %1<br>").arg(segment.curves[0].radius));
      text.append(tr("angle 0: %1<br>").arg(qRadiansToDegrees(segment.curves[0].angle)));
      text.append(tr("curve_l 0: %1<br>").arg(segment.curves[0].length));
      text.append("<br>");
      text.append(tr("radius 1: %1<br>").arg(segment.curves[1].radius));
      text.append(tr("angle 1: %1<br>").arg(qRadiansToDegrees(segment.curves[1].angle)));
      text.append(tr("curve_l 1: %1<br>").arg(segment.curves[1].length));
    }
    else
    {
      text.append(tr("radius: %1<br>").arg(segment.curves[0].radius));
      text.append(tr("angle: %1<br>").arg(qRadiansToDegrees(segment.curves[0].angle)));
      text.append(tr("curve_l: %1<br>").arg(segment.curves[0].length));
      text.append("<br>");
      text.append(tr("strai_l: %1<br>").arg(segment.straight.length));
    }

    text.append("<br>");
    text.append(tr("turnout addr: <b>%1</b><br>"
                   "turnout chan: <b><i>%2</i></b>")
                    .arg(segment.turnout.addresses[0])
                    .arg(segment.turnout.channel));
    break;
  }
  case Simulator::TrackSegment::Type::SingleSlipTurnout:
  case Simulator::TrackSegment::Type::DoubleSlipTurnout:
  {
    // TODO
    text = "TODO";
    break;
  }
  }

  if (segment.hasSensor())
  {
    const auto &sensor = data_.sensors.at(segment.sensor.index);
    text.append("<br>");
    text.append(tr("sensor addr: <b>%1</b><br>"
                   "sensor channel: <b>%2</b><br>")
                    .arg(sensor.address)
                    .arg(sensor.channel));
  }

  QToolTip::showText(ev->globalPos(), text, this);
}

void SimulatorView::setZoomLevel(float value)
{
  m_zoomLevel = std::clamp(value, zoomLevelMin, zoomLevelMax);
  updateProjection();
}

void SimulatorView::setImageVisible(int idx, bool val)
{
  if(idx < 0 || size_t(idx) >= m_extraImages.size())
    return;

  m_extraImages[idx].visible = val;
}

nlohmann::json SimulatorView::copySegmentData(size_t segmentIdx) const
{
    if(segmentIdx == Simulator::invalidIndex)
        return {};

    const auto& segment = m_simulator->staticData.trackSegments[segmentIdx];

    nlohmann::json obj;
    obj["id"] = segment.m_id;
    obj["x"] = segment.origin().x;
    obj["y"] = segment.origin().y;
    obj["rotation"] = qRadiansToDegrees(segment.rotation);

    return obj;
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
