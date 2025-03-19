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

#include "simulator.hpp"
#include <cassert>
#include <ranges>
#include <QtMath>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimerEvent>
#include <traintastic/simulator/protocol.hpp>

namespace {

Color colorFromString(const QString& name)
{
  static const std::array<QLatin1StringView, 145> names = {{QLatin1StringView("Alice_Blue"),
    QLatin1StringView("antique_white"),
    QLatin1StringView("aqua"),
    QLatin1StringView("aquamarine"),
    QLatin1StringView("azure"),
    QLatin1StringView("beige"),
    QLatin1StringView("bisque"),
    QLatin1StringView("black"),
    QLatin1StringView("blanched_almond"),
    QLatin1StringView("blue"),
    QLatin1StringView("blue_violet"),
    QLatin1StringView("brown"),
    QLatin1StringView("burlywood"),
    QLatin1StringView("cadet_blue"),
    QLatin1StringView("chartreuse"),
    QLatin1StringView("chocolate"),
    QLatin1StringView("coral"),
    QLatin1StringView("cornflower_blue"),
    QLatin1StringView("cornsilk"),
    QLatin1StringView("crimson"),
    QLatin1StringView("cyan"),
    QLatin1StringView("dark_blue"),
    QLatin1StringView("dark_cyan"),
    QLatin1StringView("dark_goldenrod"),
    QLatin1StringView("dark_gray"),
    QLatin1StringView("dark_green"),
    QLatin1StringView("dark_khaki"),
    QLatin1StringView("dark_magenta"),
    QLatin1StringView("dark_olive_green"),
    QLatin1StringView("dark_orange"),
    QLatin1StringView("dark_orchid"),
    QLatin1StringView("dark_red"),
    QLatin1StringView("dark_salmon"),
    QLatin1StringView("dark_sea_green"),
    QLatin1StringView("dark_slate_blue"),
    QLatin1StringView("dark_slate_gray"),
    QLatin1StringView("dark_turquoise"),
    QLatin1StringView("dark_violet"),
    QLatin1StringView("deep_pink"),
    QLatin1StringView("deep_sky_blue"),
    QLatin1StringView("dim_gray"),
    QLatin1StringView("dodger_blue"),
    QLatin1StringView("firebrick"),
    QLatin1StringView("floral_white"),
    QLatin1StringView("forest_green"),
    QLatin1StringView("fuchsia"),
    QLatin1StringView("gainsboro"),
    QLatin1StringView("ghost_white"),
    QLatin1StringView("gold"),
    QLatin1StringView("goldenrod"),
    QLatin1StringView("gray"),
    QLatin1StringView("web_gray"),
    QLatin1StringView("green"),
    QLatin1StringView("web_green"),
    QLatin1StringView("green_yellow"),
    QLatin1StringView("honeydew"),
    QLatin1StringView("hot_pink"),
    QLatin1StringView("indian_red"),
    QLatin1StringView("indigo"),
    QLatin1StringView("ivory"),
    QLatin1StringView("khaki"),
    QLatin1StringView("lavender"),
    QLatin1StringView("lavender_blush"),
    QLatin1StringView("lawn_green"),
    QLatin1StringView("lemon_chiffon"),
    QLatin1StringView("light_blue"),
    QLatin1StringView("light_coral"),
    QLatin1StringView("light_cyan"),
    QLatin1StringView("light_goldenrod"),
    QLatin1StringView("light_gray"),
    QLatin1StringView("light_green"),
    QLatin1StringView("light_pink"),
    QLatin1StringView("light_salmon"),
    QLatin1StringView("light_sea_green"),
    QLatin1StringView("light_sky_blue"),
    QLatin1StringView("light_slate_gray"),
    QLatin1StringView("light_steel_blue"),
    QLatin1StringView("light_yellow"),
    QLatin1StringView("lime"),
    QLatin1StringView("lime_green"),
    QLatin1StringView("linen"),
    QLatin1StringView("magenta"),
    QLatin1StringView("maroon"),
    QLatin1StringView("web_maroon"),
    QLatin1StringView("medium_aquamarine"),
    QLatin1StringView("medium_blue"),
    QLatin1StringView("medium_orchid"),
    QLatin1StringView("medium_purple"),
    QLatin1StringView("medium_sea_green"),
    QLatin1StringView("medium_slate_blue"),
    QLatin1StringView("medium_spring_green"),
    QLatin1StringView("medium_turquoise"),
    QLatin1StringView("medium_violet_red"),
    QLatin1StringView("midnight_blue"),
    QLatin1StringView("mint_cream"),
    QLatin1StringView("misty_rose"),
    QLatin1StringView("moccasin"),
    QLatin1StringView("navajo_white"),
    QLatin1StringView("navy_blue"),
    QLatin1StringView("old_lace"),
    QLatin1StringView("olive"),
    QLatin1StringView("olive_drab"),
    QLatin1StringView("orange"),
    QLatin1StringView("orange_red"),
    QLatin1StringView("orchid"),
    QLatin1StringView("pale_goldenrod"),
    QLatin1StringView("pale_green"),
    QLatin1StringView("pale_turquoise"),
    QLatin1StringView("pale_violet_red"),
    QLatin1StringView("papaya_whip"),
    QLatin1StringView("peach_puff"),
    QLatin1StringView("peru"),
    QLatin1StringView("pink"),
    QLatin1StringView("plum"),
    QLatin1StringView("powder_blue"),
    QLatin1StringView("purple"),
    QLatin1StringView("web_purple"),
    QLatin1StringView("rebecca_purple"),
    QLatin1StringView("red"),
    QLatin1StringView("rosy_brown"),
    QLatin1StringView("royal_blue"),
    QLatin1StringView("saddle_brown"),
    QLatin1StringView("salmon"),
    QLatin1StringView("sandy_brown"),
    QLatin1StringView("sea_green"),
    QLatin1StringView("seashell"),
    QLatin1StringView("sienna"),
    QLatin1StringView("silver"),
    QLatin1StringView("sky_blue"),
    QLatin1StringView("slate_blue"),
    QLatin1StringView("slate_gray"),
    QLatin1StringView("snow"),
    QLatin1StringView("spring_green"),
    QLatin1StringView("steel_blue"),
    QLatin1StringView("tan"),
    QLatin1StringView("teal"),
    QLatin1StringView("thistle"),
    QLatin1StringView("tomato"),
    QLatin1StringView("turquoise"),
    QLatin1StringView("violet"),
    QLatin1StringView("wheat"),
    QLatin1StringView("white"),
    QLatin1StringView("white_smoke"),
    QLatin1StringView("yellow"),
    QLatin1StringView("yellow_green")}};

  if(auto it = std::find(names.begin(), names.end(), name); it != names.end())
  {
    return static_cast<Color>(it - names.begin());
  }
  return Color::Magenta;
}

}

Simulator::Point origin(const Simulator::TrackSegment& segment)
{
  if(segment.type == Simulator::TrackSegment::Type::Curve)
  {
    const float angle = (segment.curve.angle < 0) ? (segment.rotation + 180) : segment.rotation;
    return {segment.x + segment.curve.radius * sinf(qDegreesToRadians(angle)), segment.y - segment.curve.radius * cosf(qDegreesToRadians(angle))};
  }
  return {segment.x, segment.y};
}

Simulator::Point straightEnd(const Simulator::TrackSegment& segment)
{
  assert(segment.type == Simulator::TrackSegment::Type::Straight || segment.type == Simulator::TrackSegment::Type::Turnout);
  return {segment.x + segment.straight.length * cosf(qDegreesToRadians(segment.rotation)),
    segment.y + segment.straight.length * sinf(qDegreesToRadians(segment.rotation))};
}

Simulator::Point curveEnd(const Simulator::TrackSegment& segment)
{
  assert(segment.type == Simulator::TrackSegment::Type::Curve || segment.type == Simulator::TrackSegment::Type::Turnout);

  const float angle = (segment.curve.angle < 0) ? (segment.rotation + 180) : segment.rotation;
  float cx;
  float cy;

  if(segment.type == Simulator::TrackSegment::Type::Curve)
  {
    cx = segment.x;
    cy = segment.y;
  }
  else
  {
    cx = segment.x - segment.curve.radius * sinf(qDegreesToRadians(angle));
    cy = segment.y + segment.curve.radius * cosf(qDegreesToRadians(angle));
  }
  return {cx + segment.curve.radius * sinf(qDegreesToRadians(angle + segment.curve.angle)),
    cy - segment.curve.radius * cosf(qDegreesToRadians(angle + segment.curve.angle))};
}

Simulator::Point end(const Simulator::TrackSegment& segment)
{
  if(segment.type == Simulator::TrackSegment::Type::Curve)
  {
    return curveEnd(segment);
  }
  return straightEnd(segment);
}

namespace {

bool pointsClose(Simulator::Point a, Simulator::Point b)
{
  return std::abs(a.x - b.x) <= 1.0f && std::abs(a.y - b.y) <= 1.0f;
}

}

class Connection
{
public:
  Connection(Simulator& simulator, QTcpSocket* socket)
    : m_simulator{simulator}
    , m_socket(socket)
  {
    QObject::connect(m_socket,
      &QTcpSocket::readyRead,
      [this]()
      {
        m_readBuffer.append(m_socket->readAll());
        while(m_readBuffer.size() > 1)
        {
          const auto& message = *reinterpret_cast<const SimulatorProtocol::Message*>(m_readBuffer.data());
          if(message.size > m_readBuffer.size())
          {
            break;
          }
          m_simulator.receive(message);
          m_readBuffer.erase(m_readBuffer.begin(), m_readBuffer.begin() + message.size);
        }
      });
    QObject::connect(m_socket,
      &QTcpSocket::disconnected,
      [this]()
      {
        m_simulator.m_connections.removeOne(this);
        delete this;
      });
  }

  void send(const SimulatorProtocol::Message& message)
  {
    m_socket->write(reinterpret_cast<const char*>(&message), message.size);
  }

private:
  Simulator& m_simulator;
  QTcpSocket* m_socket;
  QByteArray m_readBuffer;
};

Simulator::Simulator(const QString& filename, QObject* parent)
  : QObject(parent)
  , m_server{new QTcpServer(this)}
{
  Q_UNUSED(filename);
  m_server->listen(QHostAddress::Any, 5741);
  connect(m_server,
    &QTcpServer::newConnection,
    [this]()
    {
      m_connections.emplace_back(new Connection(*this, m_server->nextPendingConnection()));
    });

  load(filename);

#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
  m_tickTimer.start(1000 / fps, Qt::PreciseTimer, this);
#else
  m_tickTimer.start(std::chrono::milliseconds(1000) / fps, Qt::PreciseTimer, this);
#endif
}

bool Simulator::powerOn() const
{
  return m_powerOn;
}

void Simulator::setPowerOn(bool value)
{
  if(m_powerOn != value)
  {
    m_powerOn = value;
    send(SimulatorProtocol::Power(m_powerOn));
    emit powerOnChanged(m_powerOn);
  }
}

void Simulator::setTurnoutThrow(size_t index, bool thrown)
{
  auto& turnout = m_trackSegments[index].turnout;
  if(turnout.thrown != thrown)
  {
    turnout.thrown = thrown;
    if(turnout.address)
    {
      send(SimulatorProtocol::AccessorySetState(0, *turnout.address, turnout.thrown ? 1 : 2));
    }
  }
}

void Simulator::timerEvent(QTimerEvent *ev)
{
  if(ev->timerId() == m_tickTimer.timerId())
  {
    updateTrainPositions();
    updateSensors();
    emit tick();
    return;
  }

  QObject::timerEvent(ev);
}

void Simulator::load(const QString& filename)
{
  QFile file(filename);
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    qWarning() << "Failed to open JSON file:" << filename;
    return;
  }

  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  if(!doc.isObject())
  {
    qWarning() << "Invalid JSON format: Root must be an object";
    return;
  }

  const auto root = doc.object();
  if(root.contains("trackplan"))
  {
    loadTrackPlan(root["trackplan"].toArray({}));
  }
  if(root.contains("trains"))
  {
    loadTrains(root["trains"].toArray({}));
  }
  if(root.contains("train_width"))
  {
    m_trainWidth = root["train_width"].toDouble();
  }
}

void Simulator::loadTrackPlan(const QJsonArray& trackPlan)
{
  enum class Side
  {
    Origin = 0,
    End = 1,
    TurnoutThrown = 2,
  };

  m_trackSegments.reserve(trackPlan.size());

  Side lastSide = Side::Origin;
  size_t lastSegmentIndex = invalidIndex;
  float curX = 0;
  float curY = 0;
  float curRotation = 0;

  for(const QJsonValue& value : trackPlan)
  {
    if(!value.isObject())
    {
      continue;
    }
    QJsonObject obj = value.toObject();

    Side side = Side::Origin;
    TrackSegment segment;

    if(auto id = obj["id"].toString(); !id.isEmpty())
    {
      m_trackSegmentId.emplace(std::move(id), m_trackSegments.size());
    }

    QString type = obj["type"].toString();
    if(type == "straight")
    {
      segment.type = TrackSegment::Straight;
    }
    else if(type == "curve")
    {
      segment.type = TrackSegment::Curve;
    }
    else if(type == "turnout")
    {
      segment.type = TrackSegment::Turnout;

      if(obj.contains("address"))
      {
        segment.turnout.address = obj["address"].toInt();
      }

      const QString sideStr = obj["side"].toString();
      if(sideStr == "straight")
      {
        side = Side::End;
      }
      else if(sideStr == "curve")
      {
        side = Side::TurnoutThrown;
      }
    }

    if(segment.type == TrackSegment::Straight || segment.type == TrackSegment::Turnout)
    {
      segment.straight.length = obj["length"].toDouble();
    }
    if(segment.type == TrackSegment::Curve || segment.type == TrackSegment::Turnout)
    {
      segment.curve.radius = obj["radius"].toDouble();
      segment.curve.angle = obj["angle"].toDouble();
      segment.curve.length = qAbs(segment.curve.radius * qDegreesToRadians(segment.curve.angle));
    }

    if(obj.contains("start"))
    {
      auto start = obj["start"].toString();
      if(auto it = m_trackSegmentId.find(start); it != m_trackSegmentId.end())
      {
        auto& startSegment = m_trackSegments[it->second];
        if(startSegment.nextSegmentIndex[0] == invalidIndex)
        {
          const auto pt = origin(startSegment);
          curX = pt.x;
          curY = pt.y;
          curRotation = startSegment.rotation;

          lastSide = Side::Origin;
          lastSegmentIndex = startSegment.index;
        }
        else if(startSegment.nextSegmentIndex[1] == invalidIndex)
        {
          const auto pt = end(startSegment);
          curX = pt.x;
          curY = pt.y;
          curRotation = startSegment.rotation;
          if(startSegment.type == TrackSegment::Curve)
          {
            curRotation += startSegment.curve.angle;
          }

          lastSide = Side::Origin;
          lastSegmentIndex = startSegment.index;
        }
        else if(startSegment.type == TrackSegment::Type::Turnout && startSegment.turnout.thrownSegmentIndex == invalidIndex)
        {
          const auto pt = curveEnd(startSegment);
          curX = pt.x;
          curY = pt.y;
          curRotation = startSegment.rotation + startSegment.curve.angle;

          lastSide = Side::TurnoutThrown;
          lastSegmentIndex = startSegment.index;
        }
        else
        {
          throw std::runtime_error("start track element is already fully connected");
        }
      }
      else
      {
        throw std::runtime_error("start contains unknown id");
      }
    }
    else
    {
      if(obj.contains("x"))
      {
        curX = obj["x"].toDouble();
      }
      if(obj.contains("y"))
      {
        curY = obj["y"].toDouble();
      }
      if(obj.contains("rotation"))
      {
        curRotation = obj["rotation"].toDouble();
      }
    }

    if(segment.type == TrackSegment::Type::Turnout && side == Side::End)
    {
      segment.rotation = curRotation + 180.0f;
      if(segment.rotation >= 360.0f)
      {
        segment.rotation -= 360.0f;
      }
      segment.x = curX + segment.straight.length * cosf(qDegreesToRadians(curRotation));
      segment.y = curY + segment.straight.length * sinf(qDegreesToRadians(curRotation));
      curX = segment.x;
      curY = segment.y;
    }
    else if(segment.type == TrackSegment::Type::Turnout && side == Side::TurnoutThrown)
    {
      assert(false); // TODO: implement
    }
    else
    {
      segment.x = curX;
      segment.y = curY;
      segment.rotation = curRotation;

      if(segment.type == TrackSegment::Straight || segment.type == TrackSegment::Turnout)
      {
        curX += segment.straight.length * cosf(qDegreesToRadians(curRotation));
        curY += segment.straight.length * sinf(qDegreesToRadians(curRotation));
      }
      else if(segment.type == TrackSegment::Curve)
      {
        const float curAngle = (segment.curve.angle < 0) ? (curRotation + 180) : curRotation;

        // Calc circle center:
        segment.x = curX - segment.curve.radius * sinf(qDegreesToRadians(curAngle));
        segment.y = curY + segment.curve.radius * cosf(qDegreesToRadians(curAngle));

        // Calc end point:
        curX = segment.x + segment.curve.radius * sinf(qDegreesToRadians(curAngle + segment.curve.angle));
        curY = segment.y - segment.curve.radius * cosf(qDegreesToRadians(curAngle + segment.curve.angle));

        curRotation += segment.curve.angle;
      }
    }

    if(obj.contains("sensor_channel"))
    {
      segment.sensorChannel = obj["sensor_channel"].toInt();
    }
    if(obj.contains("sensor_address"))
    {
      segment.sensorAddress = obj["sensor_address"].toInt();
    }

    segment.index = m_trackSegments.size();
    if(lastSegmentIndex != invalidIndex)
    {
      switch(side)
      {
        case Side::Origin:
          segment.nextSegmentIndex[0] = lastSegmentIndex;
          break;

        case Side::End:
          segment.nextSegmentIndex[1] = lastSegmentIndex;
          break;

        case Side::TurnoutThrown:
          assert(segment.type == TrackSegment::Type::Turnout);
          segment.turnout.thrownSegmentIndex = lastSegmentIndex;
          break;

        default:
          assert(false);
          break;
      }

      switch(lastSide)
      {
        case Side::Origin:
          m_trackSegments[lastSegmentIndex].nextSegmentIndex[1] = m_trackSegments.size();
          break;

        case Side::End:
          m_trackSegments[lastSegmentIndex].nextSegmentIndex[0] = m_trackSegments.size();
          break;

        case Side::TurnoutThrown:
          assert(m_trackSegments[lastSegmentIndex].type == TrackSegment::Type::Turnout);
          m_trackSegments[lastSegmentIndex].turnout.thrownSegmentIndex = m_trackSegments.size();
          break;

        default:
          assert(false);
          break;
      }
    }
    m_trackSegments.emplace_back(std::move(segment));

    lastSide = side;
    lastSegmentIndex = m_trackSegments.size() - 1;
  }

  // Connect open ends:
  {
    auto findSegment = [this](size_t start, Point point, size_t& index)
    {
      const size_t count = m_trackSegments.size();
      for(size_t i = start + 1; i < count; ++i)
      {
        auto& segment = m_trackSegments[i];
        if(segment.nextSegmentIndex[0] == invalidIndex && pointsClose(point, origin(segment)))
        {
          segment.nextSegmentIndex[0] = start;
          index = i;
          return;
        }
        if(segment.nextSegmentIndex[1] == invalidIndex && pointsClose(point, end(segment)))
        {
          segment.nextSegmentIndex[1] = start;
          index = i;
          return;
        }
        if(segment.type == Simulator::TrackSegment::Type::Turnout && segment.turnout.thrownSegmentIndex == invalidIndex &&
          pointsClose(point, curveEnd(segment)))
        {
          segment.turnout.thrownSegmentIndex = start;
          index = i;
          return;
        }
      }
    };

    const size_t count = m_trackSegments.size() - 1;
    for(size_t i = 0; i < count; ++i)
    {
      auto& segment = m_trackSegments[i];
      if(segment.nextSegmentIndex[0] == invalidIndex)
      {
        findSegment(segment.index, origin(segment), segment.nextSegmentIndex[0]);
      }
      else if(segment.nextSegmentIndex[1] == invalidIndex)
      {
        findSegment(segment.index, end(segment), segment.nextSegmentIndex[1]);
      }
      else if(segment.type == Simulator::TrackSegment::Type::Turnout && segment.turnout.thrownSegmentIndex == invalidIndex)
      {
        findSegment(segment.index, curveEnd(segment), segment.turnout.thrownSegmentIndex);
      }
    }
  }
}

void Simulator::loadTrains(const QJsonArray& array)
{
  for(const auto& value : array)
  {
    if(!value.isObject())
    {
      continue;
    }
    const auto object = value.toObject();
    if(!object.contains("vehicles"))
    {
      continue;
    }

    Train train;
    size_t segmentIndex = invalidIndex;

    if(const auto trackId = object["track_id"].toString(); !trackId.isEmpty())
    {
      if(auto it = m_trackSegmentId.find(trackId); it != m_trackSegmentId.end())
      {
        segmentIndex = it->second;
      }
    }

    if(segmentIndex == invalidIndex)
    {
      segmentIndex = 0; // in case there is no free segment
      for(size_t i = 0; i < m_trackSegments.size(); ++i)
      {
        if(!m_trackSegments[i].occupied)
        {
          segmentIndex = static_cast<int>(i);
          break;
        }
      }
    }

    if(object.contains("address"))
    {
      train.address = object["address"].toInt();
    }
    if(object.contains("protocol"))
    {
      // train.protocol = object["protocol"].toString();
    }

    for(const auto& v : object["vehicles"].toArray())
    {
      if(!v.isObject())
      {
        continue;
      }
      const auto vehicle = v.toObject();
      train.addVehicle(vehicle["length"].toDouble(20.0), colorFromString(vehicle["color"].toString()));
      train.vehicles.back().front.segmentIndex = segmentIndex;
      train.vehicles.back().rear.segmentIndex = segmentIndex;
    }

    if(!train.vehicles.empty())
    {
      // center train in segment and mark it occupied:
      auto& segment = m_trackSegments[train.vehicles.front().front.segmentIndex];
      segment.occupied = train.vehicles.size() * 2;
      if(segment.sensorAddress)
      {
        segment.sensorValue = (segment.occupied != 0);
      }
      const float segmentLength = segment.length();
      const float move = segmentLength - (segmentLength - train.length()) / 2;
      for(auto& vehicle : train.vehicles)
      {
        vehicle.front.distance += move;
        vehicle.rear.distance += move;
      }
      m_trains.emplace_back(std::move(train));
    }
  }
}

void Simulator::updateTrainPositions()
{
  if(m_trackSegments.empty()) [[unlikely]]
  {
    return;
  }

  for(auto& train : m_trains)
  {
    if(train.address && train.speedOrDirectionChanged)
    {
      send(SimulatorProtocol::LocomotiveSpeedDirection(*train.address,
        train.protocol ? *train.protocol : DecoderProtocol::None,
        static_cast<uint8_t>(std::clamp<float>(std::round(std::numeric_limits<uint8_t>::max() * (train.speed / train.speedMax)),
          std::numeric_limits<uint8_t>::min(),
          std::numeric_limits<uint8_t>::max())),
        train.direction,
        false));
      train.speedOrDirectionChanged = false;
    }

    const float speed = m_powerOn ? train.speed : 0.0f;

    if(train.direction == Direction::Forward)
    {
      for(auto& vehicle : train.vehicles)
      {
        if(!updateVehiclePosition(vehicle.front, speed) || !updateVehiclePosition(vehicle.rear, speed))
        {
          train.speed = 0.0f;
          train.speedOrDirectionChanged = true;
          break;
        }
      }
    }
    else if(train.direction == Direction::Reverse)
    {
      for(auto& vehicle : train.vehicles | std::views::reverse)
      {
        if(!updateVehiclePosition(vehicle.rear, -speed) || !updateVehiclePosition(vehicle.front, -speed))
        {
          train.speed = 0.0f;
          train.speedOrDirectionChanged = true;
          break;
        }
      }
    }
  }
}

bool Simulator::updateVehiclePosition(RailVehicle::Face& face, const float speed)
{
  float distance = face.distance + (face.segmentDirectionInverted ? -speed : speed);

  // Move to next segment when reaching the end:
  {
    auto& segment = m_trackSegments[face.segmentIndex];

    if(distance >= segment.length())
    {
      const auto nextSegmentIndex = segment.getNextSegmentIndex(true);
      if(nextSegmentIndex == invalidIndex)
      {
        return false; // no next segment
      }

      assert(segment.occupied != 0);
      segment.occupied--;

      face.segmentIndex = nextSegmentIndex;
      auto& nextSegment = m_trackSegments[face.segmentIndex];

      if(nextSegment.nextSegmentIndex[0] == segment.index)
      {
        distance -= segment.length();
        face.segmentDirectionInverted = (speed < 0);
      }
      else
      {
        distance = (nextSegment.length() + segment.length()) - distance;
        face.segmentDirectionInverted = (speed > 0);
      }

      nextSegment.occupied++;
    }
    else if(distance < 0)
    {
      const auto nextSegmentIndex = segment.getNextSegmentIndex(false);
      if(nextSegmentIndex == invalidIndex)
      {
        return false; // no next segment
      }

      assert(segment.occupied != 0);
      segment.occupied--;

      face.segmentIndex = nextSegmentIndex;
      auto& nextSegment = m_trackSegments[face.segmentIndex];

      if(nextSegment.nextSegmentIndex[0] == segment.index)
      {
        distance = -distance;
        face.segmentDirectionInverted = (speed < 0);
      }
      else
      {
        distance += nextSegment.length();
        face.segmentDirectionInverted = (speed > 0);
      }

      nextSegment.occupied++;
    }
  }

  // Calculate position:
  {
    auto& segment = m_trackSegments[face.segmentIndex];

    if(segment.type == TrackSegment::Straight || (segment.type == TrackSegment::Turnout && !segment.turnout.thrown))
    {
      face.position.x = segment.x + distance * cosf(qDegreesToRadians(segment.rotation));
      face.position.y = segment.y + distance * sinf(qDegreesToRadians(segment.rotation));
    }
    else if(segment.type == TrackSegment::Curve || (segment.type == TrackSegment::Turnout && segment.turnout.thrown))
    {
      float angle = segment.rotation + (distance / segment.curve.length) * segment.curve.angle;
      if(segment.curve.angle < 0)
      {
        angle += 180;
      }

      if(segment.type == TrackSegment::Curve)
      {
        face.position.x = segment.x + segment.curve.radius * sinf(qDegreesToRadians(angle));
        face.position.y = segment.y - segment.curve.radius * cosf(qDegreesToRadians(angle));
      }
      else
      {
        const float rotation = segment.curve.angle < 0 ? segment.rotation + 180 : segment.rotation;
        const float cx = segment.x - segment.curve.radius * sinf(qDegreesToRadians(rotation));
        const float cy = segment.y + segment.curve.radius * cosf(qDegreesToRadians(rotation));

        face.position.x = cx + segment.curve.radius * sinf(qDegreesToRadians(angle));
        face.position.y = cy - segment.curve.radius * cosf(qDegreesToRadians(angle));
      }
    }
  }

  face.distance = distance;

  return true;
}

void Simulator::updateSensors()
{
  for(auto& segment : m_trackSegments)
  {
    if(segment.sensorAddress)
    {
      const bool sensorValue = m_powerOn && (segment.occupied != 0);
      if(segment.sensorValue != sensorValue)
      {
        segment.sensorValue = sensorValue;
        send(SimulatorProtocol::SensorChanged(segment.sensorChannel, segment.sensorAddress.value(), segment.sensorValue));
      }
    }
  }
}

void Simulator::send(const SimulatorProtocol::Message& message)
{
  for(auto* connection : m_connections)
  {
    connection->send(message);
  }
}

void Simulator::receive(const SimulatorProtocol::Message& message)
{
  using namespace SimulatorProtocol;

  switch(message.opCode)
  {
    case OpCode::Power:
      {
        const bool powerOn = static_cast<const Power&>(message).powerOn;
        if(m_powerOn != powerOn)
        {
          m_powerOn = powerOn;
          emit powerOnChanged(m_powerOn);
        }
        break;
      }
    case OpCode::LocomotiveSpeedDirection:
      {
        const auto& m = static_cast<const LocomotiveSpeedDirection&>(message);
        for(auto& train : m_trains)
        {
          if(train.address && *train.address == m.address && (!train.protocol || *train.protocol == m.protocol))
          {
            train.direction = m.direction;
            if(m.emergencyStop)
            {
              train.speed = 0.0f;
            }
            else
            {
              train.speed = (train.speedMax * m.speed) / std::numeric_limits<decltype(m.speed)>::max();
            }
            break;
          }
        }
        break;
      }
    case OpCode::AccessorySetState:
      {
        const auto& m = static_cast<const AccessorySetState&>(message);
        if(m.state == 1 || m.state == 2)
        {
          for(auto& segment : m_trackSegments)
          {
            if(segment.type == TrackSegment::Type::Turnout && segment.turnout.address && *segment.turnout.address == m.address)
            {
              if(m.state == 1)
              {
                segment.turnout.thrown = true;
              }
              else if(m.state == 2)
              {
                segment.turnout.thrown = false;
              }
              break;
            }
          }
        }
        break;
      }
    default:
      break;
  }
}
