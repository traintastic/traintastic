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
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
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

  startTimer(std::chrono::milliseconds(1000) / fps, Qt::PreciseTimer);
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

void Simulator::timerEvent(QTimerEvent* /*event*/)
{
  updateTrainPositions();
  emit tick();
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
}

void Simulator::loadTrackPlan(const QJsonArray& trackPlan)
{
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

    TrackSegment segment;
    segment.id = obj["id"].toString();

    QString type = obj["type"].toString();
    if(type == "straight")
    {
      segment.type = TrackSegment::Straight;
      segment.length = obj["length"].toDouble();
    }
    else if(type == "curve")
    {
      segment.type = TrackSegment::Curve;
      segment.radius = obj["radius"].toDouble();
      segment.angle = obj["angle"].toDouble();
      segment.length = qAbs(segment.radius * qDegreesToRadians(segment.angle));
    }

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

    segment.x = curX;
    segment.y = curY;
    segment.rotation = curRotation;

    if(segment.type == TrackSegment::Straight)
    {
      curX += segment.length * cosf(qDegreesToRadians(curRotation));
      curY += segment.length * sinf(qDegreesToRadians(curRotation));
    }
    else if(segment.type == TrackSegment::Curve)
    {
      const float curAngle = (segment.angle < 0) ? (curRotation + 180) : curRotation;

      // Calc circle center:
      segment.x = curX - segment.radius * sinf(qDegreesToRadians(curAngle));
      segment.y = curY + segment.radius * cosf(qDegreesToRadians(curAngle));

      // Calc end point:
      curX = segment.x + segment.radius * sinf(qDegreesToRadians(curAngle + segment.angle));
      curY = segment.y - segment.radius * cosf(qDegreesToRadians(curAngle + segment.angle));

      curRotation += segment.angle;
    }

    if(obj.contains("sensor_channel"))
    {
      segment.sensorChannel = obj["sensor_channel"].toInt();
    }
    if(obj.contains("sensor_address"))
    {
      segment.sensorAddress = obj["sensor_address"].toInt();
    }

    m_trackSegments.emplace_back(std::move(segment));
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
    int segmentIndex = -1;

    if(object.contains("track_id"))
    {
      const auto trackId = object["track_id"].toString();
      for(size_t i = 0; i < m_trackSegments.size(); ++i)
      {
        if(m_trackSegments[i].id == trackId)
        {
          segmentIndex = static_cast<int>(i);
          break;
        }
      }
    }

    if(segmentIndex < 0)
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
      train.vehicles.back().segmentIndex = segmentIndex;
    }

    if(!train.vehicles.empty())
    {
      // center train in segment and mark it occupied:
      auto& segment = m_trackSegments[train.vehicles.front().segmentIndex];
      segment.occupied++;
      const float segmentLength = segment.length;
      const float move = segmentLength - (segmentLength - train.length()) / 2;
      for(auto& vehicle : train.vehicles)
      {
        vehicle.distanceFront += move;
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

    const float speed = m_powerOn ? ((train.direction == Direction::Forward) ? train.speed : -train.speed) : 0.0f;

    auto getFrontSegment = [this, &train]() -> TrackSegment*
    {
      return &m_trackSegments[train.vehicles.front().segmentIndex];
    };

    auto getRearSegment = [this, &train]() -> TrackSegment*
    {
      auto& rear = train.vehicles.back();
      if(rear.distanceRear() >= 0)
      {
        return &m_trackSegments[rear.segmentIndex];
      }
      return &m_trackSegments[rear.segmentIndex > 0 ? rear.segmentIndex - 1 : m_trackSegments.size() - 1];
    };

    auto* frontSegment = getFrontSegment();
    auto* rearSegment = getRearSegment();

    for(auto& vehicle : train.vehicles)
    {
      vehicle.distanceFront += speed;

      auto computePosition = [this, segmentIndex = vehicle.segmentIndex](Point& position, float distance)
      {
        TrackSegment* segment = &m_trackSegments[segmentIndex];
        if(distance < 0)
        {
          segment = &m_trackSegments[segmentIndex > 0 ? segmentIndex - 1 : m_trackSegments.size() - 1];
          distance += segment->length;
        }

        if(segment->type == TrackSegment::Straight)
        {
          // Move forward along the straight track
          position.x = segment->x + distance * cosf(qDegreesToRadians(segment->rotation));
          position.y = segment->y + distance * sinf(qDegreesToRadians(segment->rotation));
        }
        else if(segment->type == TrackSegment::Curve)
        {
          // Move along circular path
          float angle = segment->rotation + (distance / segment->length) * segment->angle;
          if(segment->angle < 0)
          {
            angle += 180;
          }

          position.x = segment->x + segment->radius * sinf(qDegreesToRadians(angle));
          position.y = segment->y - segment->radius * cosf(qDegreesToRadians(angle));
        }
      };

      computePosition(vehicle.positionFront, vehicle.distanceFront);
      computePosition(vehicle.positionRear, vehicle.distanceRear());

      // Move to next segment when reaching the end
      TrackSegment& segment = m_trackSegments[vehicle.segmentIndex];
      if(vehicle.distanceFront >= segment.length)
      {
        vehicle.distanceFront -= segment.length;
        vehicle.segmentIndex++;
        if(vehicle.segmentIndex >= (int)m_trackSegments.size())
        {
          vehicle.segmentIndex = 0; // Loop track
        }
      }
      else if(vehicle.distanceFront < 0)
      {
        if(vehicle.segmentIndex == 0)
        {
          vehicle.segmentIndex = static_cast<int>(m_trackSegments.size()) - 1;
        }
        else
        {
          vehicle.segmentIndex--;
        }
        TrackSegment& prevSegment = m_trackSegments[vehicle.segmentIndex];
        vehicle.distanceFront += prevSegment.length;
      }
    }

    if(auto* frontSegmentNow = getFrontSegment(); frontSegment != frontSegmentNow)
    {
      if(train.direction == Direction::Forward)
      {
        if(frontSegmentNow->occupied == 0 && frontSegmentNow->sensorAddress)
        {
          send(SimulatorProtocol::SensorChanged(frontSegmentNow->sensorChannel, frontSegmentNow->sensorAddress.value(), true));
        }
        frontSegmentNow->occupied++;
      }
      else if(train.direction == Direction::Reverse)
      {
        if(frontSegment->occupied > 0)
        {
          frontSegment->occupied--;
          if(frontSegment->occupied == 0 && frontSegment->sensorAddress)
          {
            send(SimulatorProtocol::SensorChanged(frontSegment->sensorChannel, frontSegment->sensorAddress.value(), false));
          }
        }
      }
    }

    if(auto* rearSegmentNow = getRearSegment(); rearSegment != rearSegmentNow)
    {
      if(train.direction == Direction::Reverse)
      {
        if(rearSegmentNow->occupied == 0 && rearSegmentNow->sensorAddress)
        {
          send(SimulatorProtocol::SensorChanged(rearSegmentNow->sensorChannel, rearSegmentNow->sensorAddress.value(), true));
        }
        rearSegmentNow->occupied++;
      }
      else if(train.direction == Direction::Forward)
      {
        if(rearSegment->occupied > 0)
        {
          rearSegment->occupied--;
          if(rearSegment->occupied == 0 && rearSegment->sensorAddress)
          {
            send(SimulatorProtocol::SensorChanged(rearSegment->sensorChannel, rearSegment->sensorAddress.value(), false));
          }
        }
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
    default:
      break;
  }
}
