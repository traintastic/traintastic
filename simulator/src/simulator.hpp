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

#ifndef TRAINTASTIC_SIMULATOR_SIMULATOR_HPP
#define TRAINTASTIC_SIMULATOR_SIMULATOR_HPP

#include <QObject>
#include <list>
#include <optional>
#include <traintastic/enum/decoderprotocol.hpp>
#include <traintastic/enum/direction.hpp>
#include "color.hpp"

class QTcpServer;
class QTcpSocket;

class Connection;

namespace SimulatorProtocol {
struct Message;
}

class Simulator : public QObject
{
  Q_OBJECT

  friend class Connection;

public:
  struct TrackSegment
  {
    enum Type
    {
      Straight,
      Curve,
    } type;

    QString id;
    float x;
    float y;
    float length;
    float rotation;
    float radius;
    float angle;
    size_t occupied = 0;
    uint16_t sensorChannel = 0;
    std::optional<uint16_t> sensorAddress;
  };

  struct Point
  {
    float x;
    float y;
  };

  struct RailVehicle
  {
    Color color;
    Point positionFront;
    Point positionRear;
    float length;

    int segmentIndex = 0;
    float distanceFront = 0.0f;

    float distanceRear() const
    {
      return distanceFront - length;
    }
  };

  struct Train
  {
    static constexpr float couplingLength = 4.0f;

    std::vector<RailVehicle> vehicles;
    Direction direction = Direction::Forward;
    float speed = 0.0f;
    float speedMax = 10.0f;
    bool speedOrDirectionChanged = false;
    std::optional<DecoderProtocol> protocol;
    std::optional<uint16_t> address;

    void addVehicle(float length, Color color)
    {
      const float distance = vehicles.empty() ? 0.0 : vehicles.back().distanceRear() - couplingLength;
      auto& vehicle = vehicles.emplace_back(RailVehicle{});
      vehicle.length = length;
      vehicle.distanceFront = distance;
      vehicle.color = color;
    }

    float length() const
    {
      float sum = (vehicles.size() - 1) * couplingLength;
      for(const auto& vehicle : vehicles)
      {
        sum += vehicle.length;
      }
      return sum;
    }

    void setDirection(Direction value)
    {
      if(direction != value)
      {
        direction = value;
        speedOrDirectionChanged = true;
      }
    }

    void setSpeed(float value)
    {
      value = std::clamp(value, 0.0f, speedMax);
      if(speed != value)
      {
        speed = value;
        speedOrDirectionChanged = true;
      }
    }
  };

  Simulator(const QString& filename, QObject* parent);

  std::vector<TrackSegment>& trackSegments()
  {
    return m_trackSegments;
  }

  std::vector<Train>& trains()
  {
    return m_trains;
  }

  bool powerOn() const;
  void setPowerOn(bool value);

signals:
  void tick();
  void powerOnChanged(bool value);

protected:
  void timerEvent(QTimerEvent* event) override;

private:
  static constexpr uint fps = 30;

  QTcpServer* m_server;
  QList<Connection*> m_connections;
  std::vector<TrackSegment> m_trackSegments;
  std::vector<Train> m_trains;
  bool m_powerOn = false;

  void load(const QString& filename);
  void loadTrackPlan(const QJsonArray& trackPlan);
  void loadTrains(const QJsonArray& trains);

  void updateTrainPositions();

  void send(const SimulatorProtocol::Message& message);
  void receive(const SimulatorProtocol::Message& message);
};

constexpr Simulator::Point operator+(const Simulator::Point lhs, const Simulator::Point rhs)
{
  return {lhs.x + rhs.x, lhs.y + rhs.y};
}

constexpr Simulator::Point operator-(const Simulator::Point lhs, const Simulator::Point rhs)
{
  return {lhs.x - rhs.x, lhs.y - rhs.y};
}

constexpr Simulator::Point operator/(const Simulator::Point divided, const int divisor)
{
  return {divided.x / divisor, divided.y / divisor};
}

#endif
