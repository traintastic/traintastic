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
#include <QBasicTimer>
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
  static constexpr auto invalidIndex = std::numeric_limits<size_t>::max();

  struct TrackSegment
  {
    enum Type
    {
      Straight,
      Curve,
      Turnout,
    } type;

    size_t index = invalidIndex;
    size_t nextSegmentIndex[2] = {invalidIndex, invalidIndex};

    float x;
    float y;
    float rotation;

    struct
    {
      float length;
    } straight;

    struct
    {
      float radius;
      float angle;
      float length;
    } curve;

    struct
    {
      std::optional<uint16_t> address;
      bool thrown = false;
      size_t thrownSegmentIndex = invalidIndex;
    } turnout;

    size_t sensorIndex = invalidIndex;

    float length() const
    {
      switch(type)
      {
        case Type::Straight:
          return straight.length;

        case Type::Curve:
          return curve.length;

        case Type::Turnout:
          return turnout.thrown ? curve.length : straight.length;
      }
      return std::numeric_limits<float>::signaling_NaN();
    }

    size_t getNextSegmentIndex(bool directionPositive)
    {
      if(!directionPositive)
      {
        return nextSegmentIndex[0];
      }
      if(type == Type::Turnout && turnout.thrown)
      {
        return turnout.thrownSegmentIndex;
      }
      return nextSegmentIndex[1];
    }
  };

  struct Point
  {
    float x;
    float y;
  };

  struct RailVehicle
  {
    struct Face
    {
      Point position;
      size_t segmentIndex;
      bool segmentDirectionInverted = false;
      float distance;
    };

    Color color;
    float length;
    Face front;
    Face rear;
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
      const float distance = vehicles.empty() ? 0.0 : vehicles.back().rear.distance - couplingLength;
      auto& vehicle = vehicles.emplace_back(RailVehicle{});
      vehicle.length = length;
      vehicle.front.distance = distance;
      vehicle.rear.distance = distance - length;
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

  float trainWidth() const
  {
    return m_trainWidth;
  }

  bool isSensorOccupied(size_t sensorIndex) const;

  bool powerOn() const;
  void setPowerOn(bool value);

  void setTurnoutThrow(size_t index, bool thrown);

signals:
  void tick();
  void powerOnChanged(bool value);

protected:
  void timerEvent(QTimerEvent *ev) override;

private:
  struct Sensor
  {
    size_t occupied = 0;
    uint16_t channel = 0;
    uint16_t address = 0;
    bool value = false;
  };

  static constexpr uint fps = 30;

  QTcpServer* m_server;
  QList<Connection*> m_connections;
  std::vector<TrackSegment> m_trackSegments;
  std::unordered_map<QString, size_t> m_trackSegmentId;
  std::vector<Sensor> m_sensors;
  std::vector<Train> m_trains;

  QBasicTimer m_tickTimer;
  float m_trainWidth = 10.0f;
  bool m_powerOn = false;

  void load(const QString& filename);
  void loadTrackPlan(const QJsonArray& trackPlan);
  void loadTrains(const QJsonArray& trains);

  void updateTrainPositions();
  bool updateVehiclePosition(RailVehicle::Face& face, float speed);
  void updateSensors();

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

Simulator::Point origin(const Simulator::TrackSegment& segment);
Simulator::Point straightEnd(const Simulator::TrackSegment& segment);
Simulator::Point curveEnd(const Simulator::TrackSegment& segment);
Simulator::Point end(const Simulator::TrackSegment& segment);

#endif
