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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_SIMULATOR_SIMULATOR_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_SIMULATOR_SIMULATOR_HPP

#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/signals2/signal.hpp>
#include <nlohmann/json.hpp>
#include <traintastic/enum/color.hpp>
#include <traintastic/enum/decoderprotocol.hpp>

namespace SimulatorProtocol {
  struct Message;
}

class SimulatorConnection;

class Simulator : public std::enable_shared_from_this<Simulator>
{
public:
  static constexpr auto invalidIndex = std::numeric_limits<size_t>::max();
  static constexpr uint16_t defaultChannel = 0;
  static constexpr auto invalidAddress = std::numeric_limits<uint16_t>::max();

  struct Point
  {
    float x;
    float y;

    bool isFinite() const
    {
      return std::isfinite(x) && std::isfinite(y);
    }
  };
  static constexpr Point invalidPoint{std::numeric_limits<float>::signaling_NaN(), std::numeric_limits<float>::signaling_NaN()};

  struct TrackSegment
  {
    static constexpr size_t connectorCount = 4;

    enum class Type
    {
      Straight,
      Curve,
      Turnout,
      TurnoutCurved,
      Turnout3Way,
      SingleSlipTurnout,
      DoubleSlipTurnout,
    } type;

    std::string m_id;
    std::array<size_t, connectorCount> nextSegmentIndex{{invalidIndex, invalidIndex, invalidIndex, invalidIndex}};
    std::array<Point, connectorCount> points{{invalidPoint, invalidPoint, invalidPoint, invalidPoint}};
    float rotation;

    struct Straight
    {
      float length;
    } straight, straight2; // TODO: make array

    struct Curve
    {
      Point center = invalidPoint;
      float radius;
      float angle;
      float length;
    };
    std::array<Curve, 2> curves;

    struct Sensor
    {
      size_t index = invalidIndex;
      uint16_t channel = 0;
      uint16_t address = invalidAddress;
    } sensor;

    struct Turnout
    {
      size_t index = invalidIndex;
      uint16_t channel = 0;
      std::array<uint16_t, 2> addresses{{invalidAddress, invalidAddress}};
    } turnout;

    Point origin() const
    {
      return points[0];
    }

    bool hasSensor() const
    {
      return sensor.index != invalidIndex;
    }
  };

  struct Sensor
  {
    uint16_t channel = defaultChannel;
    uint16_t address = invalidAddress;
  };

  struct Vehicle
  {
    Color color;
    float length;
  };

  struct Train
  {
    std::vector<size_t> vehicleIndexes;
    float length = 0.0f;
    float speedMax = 10.0f;
    DecoderProtocol protocol = DecoderProtocol::None;
    uint16_t address = invalidAddress;
  };

  struct Misc
  {
    enum class Type
    {
      Rectangle,
    } type;

    Point origin;
    float rotation = 0.0f;
    float height = 0.0f;
    float width = 0.0f;
    Color color;
  };

  struct StaticData
  {
    struct View
    {
      float top = std::numeric_limits<float>::infinity();
      float left = std::numeric_limits<float>::infinity();
      float bottom = -std::numeric_limits<float>::infinity();
      float right = -std::numeric_limits<float>::infinity();

      float height() const
      {
        return bottom - top;
      }

      float width() const
      {
        return right - left;
      }
    } view;
    std::vector<TrackSegment> trackSegments;
    std::vector<Sensor> sensors;
    std::vector<Vehicle> vehicles;
    std::vector<Train> trains;
    std::vector<Misc> misc;

    std::unordered_map<std::string, size_t> trackSegmentId;

    float trainWidth = 10.0f;
    float trainCouplingLength = 4.0f;
  };

  struct SensorState
  {
    size_t occupied = 0;
    bool value = false;
  };

  struct TurnoutState
  {
    enum class State : uint8_t
    {
      Unknown = 0,
      Closed = 1,
      Thrown = 2,
      ThrownLeft = 3,
      ThrownRight = 4,
      ClosedLeft = 5,
      ClosedRight = 6
    };

    State state = State::Closed;
    uint8_t coils = 0;
  };

  struct TrainState
  {
    float speed = 0.0f;
    bool reverse = false;
    bool speedOrDirectionChanged = false;
  };

  struct VehicleState
  {
    struct Face
    {
      Point position = invalidPoint;
      size_t segmentIndex = invalidIndex;
      bool segmentDirectionInverted = false;
      float distance = 0.0f;
    };

    Face front;
    Face rear;
  };

  struct StateData
  {
    float tickActive = 0.0f;
    float tickLoad = 0.0f;
    bool powerOn = false;
    std::vector<SensorState> sensors;
    std::vector<TurnoutState> turnouts;
    std::vector<TrainState> trains;
    std::vector<VehicleState> vehicles;
  };

private:
  StateData m_stateData;

public:
  const StaticData staticData;
  boost::signals2::signal<void()> onTick;

  explicit Simulator(const nlohmann::json& world);
  ~Simulator();

  StateData stateData() const;

  void enableServer(bool localhostOnly = true, uint16_t port = 5741);

  uint16_t serverPort() const;

  void start(bool discoverable = false);
  void stop();

  void setPowerOn(bool powerOn);
  void togglePowerOn();

  void setTrainDirection(size_t trainIndex, bool reverse);
  void setTrainSpeed(size_t trainIndex, float speed);
  void applyTrainSpeedDelta(size_t trainIndex, float delta);
  void stopAllTrains();

  void setTurnoutState(size_t segmentIndex, TurnoutState::State state);
  void toggleTurnoutState(size_t segmentIndex, bool setUnknown);

  void send(const SimulatorProtocol::Message& message);
  void receive(const SimulatorProtocol::Message& message, size_t fromConnId);
  void removeConnection(const std::shared_ptr<SimulatorConnection>& connection);

private:
  constexpr static auto tickRate = std::chrono::milliseconds(1000 / 30);
  constexpr static auto handShakeRate = std::chrono::milliseconds(1000);

  boost::asio::io_context m_ioContext;
  boost::asio::steady_timer m_tickTimer;
  boost::asio::steady_timer m_handShakeTimer;
  boost::asio::ip::tcp::acceptor m_acceptor;
  boost::asio::ip::udp::socket m_socketUDP;
  std::array<char, 8> m_udpBuffer;
  boost::asio::ip::udp::endpoint m_remoteEndpoint;

  std::thread m_thread;
  mutable std::mutex m_stateMutex;
  bool m_serverEnabled = false;
  bool m_serverLocalHostOnly = true;
  static constexpr uint16_t defaultPort = 5741; // UDP Discovery
  uint16_t m_serverPort = 5741;

  size_t lastConnectionId = 0;
  std::list<std::shared_ptr<SimulatorConnection>> m_connections;

  void accept();
  void doReceive();
  void onConnectionRemoved(const std::shared_ptr<SimulatorConnection> &);

  void tick();
  void handShake();

  void updateTrainPositions();
  bool updateVehiclePosition(VehicleState::Face& face, const float speed);
  void updateSensors();

  bool isStraight(const TrackSegment& segment);
  bool isCurve(const TrackSegment& segment, size_t& curveIndex);

  static StaticData load(const nlohmann::json& world, StateData& stateData);
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
