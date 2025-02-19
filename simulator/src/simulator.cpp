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

Simulator::Color colorFromString(const QString& value)
{
  if(value == "red")
  {
    return Simulator::Color::Red;
  }
  if(value == "green")
  {
    return Simulator::Color::Green;
  }
  if(value == "blue")
  {
    return Simulator::Color::Blue;
  }
  return static_cast<Simulator::Color>(0);
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
      float centerX = curX - segment.radius * sinf(qDegreesToRadians(curRotation));
      float centerY = curY + segment.radius * cosf(qDegreesToRadians(curRotation));

      segment.x = centerX;
      segment.y = centerY;

      curX = centerX + segment.radius * sinf(qDegreesToRadians(curRotation + segment.angle));
      curY = centerY - segment.radius * cosf(qDegreesToRadians(curRotation + segment.angle));
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
      vehicle.distanceFront += train.speed;

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
          float centerX = segment->x;
          float centerY = segment->y;
          float angle = segment->rotation + (distance / segment->length) * segment->angle;

          position.x = centerX + segment->radius * sinf(qDegreesToRadians(angle));
          position.y = centerY - segment->radius * cosf(qDegreesToRadians(angle));
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
          vehicle.segmentIndex = m_trackSegments.size() - 1;
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
      if(train.speed > 0)
      {
        if(frontSegmentNow->occupied == 0 && frontSegmentNow->sensorAddress)
        {
          send(SimulatorProtocol::SensorChanged(frontSegmentNow->sensorChannel, frontSegmentNow->sensorAddress.value(), true));
        }
        frontSegmentNow->occupied++;
      }
      else if(train.speed < 0)
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
      if(train.speed < 0)
      {
        if(rearSegmentNow->occupied == 0 && rearSegmentNow->sensorAddress)
        {
          send(SimulatorProtocol::SensorChanged(rearSegmentNow->sensorChannel, rearSegmentNow->sensorAddress.value(), true));
        }
        rearSegmentNow->occupied++;
      }
      else if(train.speed > 0)
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
    case OpCode::LocomotiveSpeedDirection:
      {
        const auto& m = static_cast<const LocomotiveSpeedDirection&>(message);
        for(auto& train : m_trains)
        {
          if(train.address && *train.address == m.address && (!train.protocol || *train.protocol == m.protocol))
          {
            if(m.emergencyStop)
            {
              train.speed = 0.0f;
            }
            else
            {
              train.speed = (train.speedMax * m.speed) / std::numeric_limits<decltype(m.speed)>::max();
              if(m.direction == Direction::Reverse)
              {
                train.speed = -train.speed;
              }
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
