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
#include <numbers>
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
#include <traintastic/utils/stringhash.hpp>
#include <traintastic/utils/stringequal.hpp>

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

    struct Object
    {
        enum class AllowedDirections
        {
            Both = 0,
            Forward = 1,
            Backwards = 2
        };

        enum class Type
        {
            PositionSensor = 0,
            MainSignal = 1
        };

        Point pos;
        float rotation;
        float position = 0.0f;
        float lateralDiff = 3.0f;

        size_t sensorIndex = invalidIndex;
        AllowedDirections allowedDirection = AllowedDirections::Both;
        bool dirForward = true;
        Type type = Type::PositionSensor;
        std::string_view signalName;
    };
    std::vector<Object> objects;

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

  struct ImageRef
  {
    std::string fileName;
    Point origin;
    float rotation = 0.0f;
    float ratio = 0.0f;
    float opacity = 1.0f;
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
    std::vector<Misc> misc;
    std::vector<ImageRef> images;
    std::unordered_map<std::string, size_t> trackSegmentId;

    float trainWidth = 10.0f;
    float trainCouplingLength = 4.0f;
    float worldScale = 1.0f;
  };

  struct SensorState
  {
    size_t occupied = 0;
    bool value = false;
    size_t maxTime = 0;
    size_t curTime = 0;
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

  struct MainSignal
  {
    std::string name;
    size_t ownerConnectionId = invalidIndex;
    double maxSpeed = 0;

    struct Light
    {
        enum class Color
        {
            Red = 0,
            Yellow,
            Green
        } color = Color::Red;

        enum class State
        {
            Off = 0,
            On,
            BlikOn,
            BlinkReverseOn
        } state = State::Off;
    };

    std::vector<Light> lights;
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

  struct Train;

  struct Vehicle
  {
    std::string name;
    Color color;
    float length;
    Train *activeTrain = nullptr;
    VehicleState state;
  };

  struct Train
  {
    std::string name;

    struct VehicleItem
    {
      Vehicle *vehicle;
      bool reversed = false;
    };

    std::vector<VehicleItem> vehicles;
    float length = 0.0f;
    float speedMax = 0.0f;
    DecoderProtocol protocol = DecoderProtocol::None;
    uint16_t address = invalidAddress;
    TrainState state;

    ~Train()
    {
      for(const VehicleItem& item : vehicles)
      {
        assert(item.vehicle->activeTrain == this);
        item.vehicle->activeTrain = nullptr;
      }
    }
  };

  struct StateData
  {
    float tickActive = 0.0f;
    float tickLoad = 0.0f;
    bool powerOn = false;
    std::vector<SensorState> sensors;
    std::vector<TurnoutState> turnouts;
    std::unordered_map<std::string, Train *, StringHash, StringEqual> trains;
    std::unordered_map<std::string, Vehicle *, StringHash, StringEqual> vehicles;
    std::unordered_map<std::string, MainSignal *, StringHash, StringEqual> mainSignals;
  };

private:
  StateData m_stateData;

public:
  const StaticData staticData;
  boost::signals2::signal<void()> onTick;

  explicit Simulator(const nlohmann::json& world);
  ~Simulator();

  static void updateView(Simulator::StaticData::View& view, Simulator::Point point);
  static void updateView(Simulator::StaticData::View& view,
                         const Simulator::TrackSegment::Curve& curve, float startAngle);

  StateData stateData() const;

  void enableServer(bool localhostOnly = true, uint16_t port = 5742);

  uint16_t serverPort() const;

  void start();
  void stop();

  void setPowerOn(bool powerOn);
  void togglePowerOn();

  Train *getTrainAt(size_t trainIndex) const;

  bool isTrainDirectionInverted(Train *train);
  void setTrainDirection(Train *train, bool reverse);
  void setTrainSpeed(Train *train, float speed);
  void applyTrainSpeedDelta(Train *train, float delta);
  void stopAllTrains();

  void setTurnoutState(size_t segmentIndex, TurnoutState::State state);
  void toggleTurnoutState(size_t segmentIndex, bool setUnknown);

  void send(const SimulatorProtocol::Message& message);
  void receive(const SimulatorProtocol::Message& message);
  void removeConnection(const std::shared_ptr<SimulatorConnection>& connection);

  inline std::recursive_mutex& stateMutex() { return m_stateMutex; }

private:
  constexpr static auto tickRate = std::chrono::milliseconds(1000 / 30);

  constexpr static float defaultSpeedKmH = 200;
  constexpr static float defaultSpeedMeterPerSecond = defaultSpeedKmH / 3.6;
  constexpr static float defaultSpeedTickRate = defaultSpeedMeterPerSecond / 1000 * tickRate.count();

  boost::asio::io_context m_ioContext;
  boost::asio::steady_timer m_tickTimer;
  boost::asio::ip::tcp::acceptor m_acceptor;
  boost::asio::ip::udp::socket m_socketUDP;
  std::array<char, 8> m_udpBuffer;
  boost::asio::ip::udp::endpoint m_remoteEndpoint;

  std::thread m_thread;
  mutable std::recursive_mutex m_stateMutex;
  bool m_serverEnabled = false;
  bool m_serverLocalHostOnly = true;
  static constexpr uint16_t defaultPort = 5741; // UDP Discovery
  uint16_t m_serverPort = 5742;

  size_t lastConnectionId = 0;
  std::list<std::shared_ptr<SimulatorConnection>> m_connections;

  void accept();
  void doReceive();

  void sendInitialState(const std::shared_ptr<SimulatorConnection>& connection);

  void tick();

  void updateTrainPositions();
  bool updateVehiclePosition(VehicleState::Face& face, const float speed);
  void updateSensors();

  bool isStraight(const TrackSegment& segment);
  bool isCurve(const TrackSegment& segment, size_t& curveIndex);

  static StaticData load(const nlohmann::json& world, StateData& stateData);
  static void loadTrackplan(const nlohmann::json &world,
                            StaticData &data, StateData &stateData);
  static void loadTrackObjects(const nlohmann::json &track,
                               StaticData &data, StateData &stateData,
                               TrackSegment &segment);
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

namespace
{

constexpr auto pi = std::numbers::pi_v<float>;

constexpr float deg2rad(float degrees)
{
  return degrees * static_cast<float>(std::numbers::pi / 180);
}

}

#endif
