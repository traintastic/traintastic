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
#include "simulatorconnection.hpp"
#include <numbers>
#include <ranges>
#include "protocol.hpp"

namespace
{

constexpr auto pi = std::numbers::pi_v<float>;

constexpr float deg2rad(float degrees)
{
  return degrees * static_cast<float>(std::numbers::pi / 180);
}

#ifndef NDEBUG
constexpr size_t getStraightCount(Simulator::TrackSegment::Type type)
{
  using Type = Simulator::TrackSegment::Type;
  switch(type)
  {
    case Type::Curve:
    case Type::TurnoutCurved:
      return 0;

    case Type::Straight:
    case Type::Turnout:
    case Type::Turnout3Way:
      return 1;
  }
  return 0;
}
#endif

constexpr size_t getCurveCount(Simulator::TrackSegment::Type type)
{
  using Type = Simulator::TrackSegment::Type;
  switch(type)
  {
    case Type::Straight:
      return 0;

    case Type::Curve:
    case Type::Turnout:
      return 1;

    case Type::TurnoutCurved:
    case Type::Turnout3Way:
      return 2;
  }
  return 0;
}

Simulator::Point straightEnd(const Simulator::TrackSegment& segment)
{
  assert(1 == getStraightCount(segment.type));
  return {
    segment.points[0].x + segment.straight.length * std::cos(segment.rotation), segment.points[0].y + segment.straight.length * std::sin(segment.rotation)};
}

Simulator::Point curveEnd(const Simulator::TrackSegment& segment, size_t curveIndex = 0)
{
  assert(curveIndex < getCurveCount(segment.type));

  const auto& curve = segment.curves[curveIndex];
  const float angle = (curve.angle < 0) ? (segment.rotation + pi) : segment.rotation;

  return {curve.center.x + curve.radius * std::sin(angle + curve.angle),
    curve.center.y - curve.radius * std::cos(angle + curve.angle)};
}

#ifndef _MSC_VER // std::abs is not constexpr :(
constexpr
#endif
bool pointsClose(Simulator::Point a, Simulator::Point b)
{
  return std::abs(a.x - b.x) <= 1.0f && std::abs(a.y - b.y) <= 1.0f;
}

constexpr size_t getConnectorCount(Simulator::TrackSegment::Type type)
{
  using Type = Simulator::TrackSegment::Type;
  switch(type)
  {
    case Type::Straight:
    case Type::Curve:
      return 2;

    case Type::Turnout:
    case Type::TurnoutCurved:
      return 3;

    case Type::Turnout3Way:
      return 4;
  }
  return 0;
}

float getSegmentLength(const Simulator::TrackSegment& segment, const Simulator::StateData& stateData)
{
  using Type = Simulator::TrackSegment::Type;
  using State = Simulator::TurnoutState::State;

  switch(segment.type)
  {
    case Type::Straight:
      return segment.straight.length;

    case Type::Curve:
      return segment.curves[0].length;

    case Type::Turnout:
      switch(stateData.turnouts[segment.turnout.index].state)
      {
        case State::Closed:
          return segment.straight.length;

        case State::Thrown:
          return segment.curves[0].length;

        default: // all others are invalid
          break;
      }
      break;

    case Type::TurnoutCurved:
      switch(stateData.turnouts[segment.turnout.index].state)
      {
        case State::Closed:
          return segment.curves[0].length;

        case State::Thrown:
          return segment.curves[1].length;

        default: // all others are invalid
          break;
      }
      break;

    case Type::Turnout3Way:
      switch(stateData.turnouts[segment.turnout.index].state)
      {
        case State::Closed:
          return segment.straight.length;

        case State::ThrownLeft:
          return segment.curves[0].length;

        case State::ThrownRight:
          return segment.curves[1].length;

        default: // all others are invalid
          break;
      }
      break;
  }
  return 0.0f;
}

size_t getNextSegmentIndex(const Simulator::TrackSegment& segment, bool directionPositive, const Simulator::StateData& stateData)
{
  using State = Simulator::TurnoutState::State;
  if(!directionPositive)
  {
    return segment.nextSegmentIndex[0];
  }
  if((segment.type == Simulator::TrackSegment::Type::Turnout || segment.type == Simulator::TrackSegment::Type::TurnoutCurved) &&
      stateData.turnouts[segment.turnout.index].state == Simulator::TurnoutState::State::Thrown)
  {
    return segment.nextSegmentIndex[2];
  }
  if(segment.type == Simulator::TrackSegment::Type::Turnout3Way)
  {
    const auto state = stateData.turnouts[segment.turnout.index].state;
    if(state == State::ThrownLeft)
    {
      return segment.nextSegmentIndex[2];
    }
    else if(state == State::ThrownRight)
    {
      return segment.nextSegmentIndex[3];
    }
    assert(state == State::Closed);
  }
  return segment.nextSegmentIndex[1];
}

template<typename T>
std::optional<T> stringToEnum(std::string_view value)
{
  auto it = std::find_if(EnumValues<T>::value.begin(), EnumValues<T>::value.end(),
    [&value](const auto& n)
    {
      return n.second == value;
    });

  if(it != EnumValues<T>::value.end())
  {
    return it->first;
  }
  return std::nullopt;
}

}

Simulator::Simulator(const nlohmann::json& world)
  : staticData(load(world, m_stateData))
  , m_tickTimer{m_ioContext}
  , m_acceptor{m_ioContext}
{
}

Simulator::~Simulator()
{
  stop();
}

Simulator::StateData Simulator::stateData() const
{
  std::lock_guard<std::mutex> lock(m_stateMutex);
  return m_stateData;
}

void Simulator::enableServer(bool localhostOnly, uint16_t port)
{
  m_serverEnabled = true;
  m_serverLocalHostOnly = localhostOnly;
  m_serverPort = port;
}

uint16_t Simulator::serverPort() const
{
  return m_acceptor.local_endpoint().port();
}

void Simulator::start()
{
  m_thread = std::thread(
    [this]()
    {
      if(m_serverEnabled)
      {
        boost::system::error_code ec;
        boost::asio::ip::tcp::endpoint endpoint(m_serverLocalHostOnly ? boost::asio::ip::address_v4::loopback() : boost::asio::ip::address_v4::any(), m_serverPort);

        m_acceptor.open(endpoint.protocol(), ec);

        m_acceptor.bind(endpoint, ec);

        m_acceptor.listen(5, ec);

        accept();
      }
      tick();
      m_ioContext.run();
    });
}

void Simulator::stop()
{
  m_acceptor.cancel();
  while(!m_connections.empty())
  {
    auto connection =  m_connections.back();
    m_connections.pop_back();
    connection->stop();
  }

  m_tickTimer.cancel();
  if(m_thread.joinable())
  {
    m_thread.join();
  }
}

void Simulator::setPowerOn(bool powerOn)
{
  std::lock_guard<std::mutex> lock(m_stateMutex);
  if(m_stateData.powerOn != powerOn)
  {
    m_stateData.powerOn = powerOn;
    send(SimulatorProtocol::Power(m_stateData.powerOn));
  }
}

void Simulator::togglePowerOn()
{
  std::lock_guard<std::mutex> lock(m_stateMutex);
  m_stateData.powerOn = !m_stateData.powerOn;
  send(SimulatorProtocol::Power(m_stateData.powerOn));
}

void Simulator::setTrainDirection(size_t trainIndex, bool reverse)
{
  if(trainIndex < staticData.trains.size())
  {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    auto& train = m_stateData.trains[trainIndex];
    if(train.reverse != reverse)
    {
      train.reverse = reverse;
      train.speedOrDirectionChanged = true;
    }
  }
}

void Simulator::setTrainSpeed(size_t trainIndex, float speed)
{
  if(trainIndex < staticData.trains.size())
  {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    auto& train = m_stateData.trains[trainIndex];
    speed = std::clamp(speed, 0.0f, staticData.trains[trainIndex].speedMax);
    if(train.speed != speed)
    {
      train.speed = speed;
      train.speedOrDirectionChanged = true;
    }
  }
}

void Simulator::applyTrainSpeedDelta(size_t trainIndex, float delta)
{
  if(trainIndex < staticData.trains.size())
  {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    auto& train = m_stateData.trains[trainIndex];
    const float speed = std::clamp(train.speed + delta, 0.0f, staticData.trains[trainIndex].speedMax);
    if(train.speed != speed)
    {
      train.speed = speed;
      train.speedOrDirectionChanged = true;
    }
  }
}

void Simulator::stopAllTrains()
{
  std::lock_guard<std::mutex> lock(m_stateMutex);
  for(auto& train : m_stateData.trains)
  {
    if(train.speed != 0.0f)
    {
      train.speed = 0.0f;
      train.speedOrDirectionChanged = true;
    }
  }
}

void Simulator::setTurnoutState(size_t segmentIndex, TurnoutState::State state)
{
  using Type = TrackSegment::Type;
  using State = TurnoutState::State;

  if(segmentIndex < staticData.trackSegments.size() && staticData.trackSegments[segmentIndex].turnout.index != invalidIndex)
  {
    const auto& segment = staticData.trackSegments[segmentIndex];
    std::lock_guard<std::mutex> lock(m_stateMutex);
    auto& turnout = m_stateData.turnouts[segment.turnout.index];
    if(turnout.state != state)
    {
      if(segment.type == Type::Turnout || segment.type == Type::TurnoutCurved)
      {
        if(segment.turnout.addresses[0] != invalidAddress)
        {
          send(SimulatorProtocol::AccessorySetState(0, segment.turnout.addresses[0], turnout.state == State::Thrown ? 1 : 2));
        }
      }
      else if(segment.type == Type::Turnout3Way)
      {



        return; // FIXME
      }
      turnout.state = state;
    }
  }
}

void Simulator::toggleTurnoutState(size_t segmentIndex)
{
  using Type = TrackSegment::Type;
  using State = TurnoutState::State;

  if(segmentIndex < staticData.trackSegments.size() && staticData.trackSegments[segmentIndex].turnout.index != invalidIndex)
  {
    const auto& segment = staticData.trackSegments[segmentIndex];

    Simulator::TurnoutState::State state;
    {
      std::lock_guard<std::mutex> lock(m_stateMutex);
      state = m_stateData.turnouts[segment.turnout.index].state;
    }

    if(segment.type == Type::Turnout || segment.type == Type::TurnoutCurved)
    {
      state = (state != TurnoutState::State::Closed) ? TurnoutState::State::Closed : TurnoutState::State::Thrown;
    }
    else if(segment.type == Type::Turnout3Way)
    {
      if(state == State::Closed)
      {
        state = State::ThrownLeft;
      }
      else if(state == State::ThrownLeft)
      {
        state = State::ThrownRight;
      }
      else
      {
        assert(state == State::ThrownRight);
        state = State::Closed;
      }
    }

    setTurnoutState(segmentIndex, state);
  }
}

void Simulator::send(const SimulatorProtocol::Message& message)
{
  for(const auto& connection : m_connections)
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
      std::lock_guard<std::mutex> lock(m_stateMutex);
      m_stateData.powerOn = static_cast<const Power&>(message).powerOn;
      break;
    }
    case OpCode::LocomotiveSpeedDirection:
    {
      const auto& m = static_cast<const LocomotiveSpeedDirection&>(message);
      const auto count = staticData.trains.size();
      for(size_t i = 0; i < count; ++i)
      {
        const auto& train = staticData.trains[i];
        if(m.address == train.address && (m.protocol == train.protocol || train.protocol == DecoderProtocol::None))
        {
          std::lock_guard<std::mutex> lock(m_stateMutex);
          const float speed = std::clamp(train.speedMax * (m.speed / 255.0f), 0.0f, train.speedMax);
          const bool reverse = m.direction == Direction::Reverse;
          auto& trainState = m_stateData.trains[i];
          if(trainState.speed != speed || trainState.reverse != reverse)
          {
            trainState.speed = speed;
            trainState.reverse = reverse;
          }
          break;
        }
      }
      break;
    }
    case OpCode::AccessorySetState:
    {
      const auto& m = static_cast<const AccessorySetState&>(message);
      const auto count = staticData.trackSegments.size();
      for(size_t i = 0; i < count; ++i)
      {
        const auto& segment = staticData.trackSegments[i];
        if((segment.type == Simulator::TrackSegment::Type::Turnout || segment.type == Simulator::TrackSegment::Type::TurnoutCurved) &&
            m.address == segment.turnout.addresses[0])
        {
          std::lock_guard<std::mutex> lock(m_stateMutex);
          auto& turnoutState = m_stateData.turnouts[segment.turnout.index];
          if(m.state == 1)
          {
            turnoutState.state = Simulator::TurnoutState::State::Thrown;
          }
          else if(m.state == 2)
          {
            turnoutState.state = Simulator::TurnoutState::State::Closed;
          }
          break;
        }
        else if(segment.type == Simulator::TrackSegment::Type::Turnout3Way && (m.address == segment.turnout.addresses[0] || m.address == segment.turnout.addresses[1]))
        {
          // Left has preference over right, if other behavior is required add it to the json config as option.
          //
          //  |/ = 2nd address
          // \|  = 1st address
          //
          // TurnoutState::coils = 0 0 0 0 RC RT LC LT

          std::lock_guard<std::mutex> lock(m_stateMutex);
          auto& turnoutState = m_stateData.turnouts[segment.turnout.index];
          if(m.address == segment.turnout.addresses[0])
          {
            turnoutState.coils &= 0x0C;
            turnoutState.coils |= (m.state & 0x03);
          }
          else if(m.address == segment.turnout.addresses[1])
          {
            turnoutState.coils &= 0x03;
            turnoutState.coils |= (m.state & 0x03) << 2;
          }

          switch(turnoutState.coils)
          {
            case 0x0A:
              turnoutState.state = Simulator::TurnoutState::State::Closed;
              break;

            case 0x05:
            case 0x09:
              turnoutState.state = Simulator::TurnoutState::State::ThrownLeft;
              break;

            case 0x06:
              turnoutState.state = Simulator::TurnoutState::State::ThrownRight;
              break;
          }
          break;
        }
      }
      break;
    }
    case OpCode::SensorChanged:
      break; // only sent by simulator
  }
}

void Simulator::removeConnection(const std::shared_ptr<SimulatorConnection>& connection)
{
  if(auto it = std::find(m_connections.begin(), m_connections.end(), connection); it != m_connections.end())
  {
    m_connections.erase(it);
  }
}

void Simulator::accept()
{
  m_acceptor.async_accept(
    [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
    {
      if(!ec)
      {
        m_connections.emplace_back(std::make_shared<SimulatorConnection>(shared_from_this(), std::move(socket)))->start();
        accept();
      }
    });
}

void Simulator::tick()
{
  m_tickTimer.expires_after(tickRate);
  m_tickTimer.async_wait(
    [this](std::error_code ec)
    {
      if(!ec)
      {
        tick();
      }
    });

  {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    const auto start = std::chrono::high_resolution_clock::now();
    updateTrainPositions();
    updateSensors();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start);
    m_stateData.tickActive = duration.count() / 1e6f;
    m_stateData.tickLoad = static_cast<float>(100 * duration.count()) / static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(tickRate).count());
  }

  onTick();
}

void Simulator::updateTrainPositions()
{
  if(staticData.trackSegments.empty()) [[unlikely]]
  {
    return;
  }

  const size_t trainCount = staticData.trains.size();
  for(size_t i = 0; i < trainCount; ++i)
  {
    const auto& train = staticData.trains[i];
    auto& trainState = m_stateData.trains[i];

    if(train.address != invalidAddress && trainState.speedOrDirectionChanged)
    {
      send(SimulatorProtocol::LocomotiveSpeedDirection(train.address,
        train.protocol,
        static_cast<uint8_t>(std::clamp<float>(std::round(std::numeric_limits<uint8_t>::max() * (trainState.speed / train.speedMax)),
          std::numeric_limits<uint8_t>::min(),
          std::numeric_limits<uint8_t>::max())),
        trainState.reverse ? Direction::Reverse : Direction::Forward,
        false));
      trainState.speedOrDirectionChanged = false;
    }

    const float speed = m_stateData.powerOn ? trainState.speed : 0.0f;

    if(!trainState.reverse)
    {
      for(const auto& vehicleIndex : train.vehicleIndexes)
      {
        auto& vehicleState = m_stateData.vehicles[vehicleIndex];
        if(!updateVehiclePosition(vehicleState.front, speed) || !updateVehiclePosition(vehicleState.rear, speed))
        {
          trainState.speed = 0.0f;
          trainState.speedOrDirectionChanged = true;
          break;
        }
      }
    }
    else // reverse
    {
      for(const auto& vehicleIndex : train.vehicleIndexes | std::views::reverse)
      {
        auto& vehicleState = m_stateData.vehicles[vehicleIndex];
        if(!updateVehiclePosition(vehicleState.rear, -speed) || !updateVehiclePosition(vehicleState.front, -speed))
        {
          trainState.speed = 0.0f;
          trainState.speedOrDirectionChanged = true;
          break;
        }
      }
    }
  }
}

bool Simulator::updateVehiclePosition(VehicleState::Face& face, const float speed)
{
  float distance = face.distance + (face.segmentDirectionInverted ? -speed : speed);

  // Move to next segment when reaching the end:
  {
    const size_t faceSegmentIndexBefore = face.segmentIndex;
    const auto& segment = staticData.trackSegments[face.segmentIndex];
    const auto segmentLength = getSegmentLength(segment, m_stateData);

    if(distance >= segmentLength)
    {
      const auto nextSegmentIndex = getNextSegmentIndex(segment, true, m_stateData);
      if(nextSegmentIndex == invalidIndex)
      {
        return false; // no next segment
      }

      if(segment.sensor.index != invalidIndex)
      {
        auto& sensor = m_stateData.sensors[segment.sensor.index];
        assert(sensor.occupied != 0);
        sensor.occupied--;
      }

      face.segmentIndex = nextSegmentIndex;
      auto& nextSegment = staticData.trackSegments[face.segmentIndex];

      if(nextSegment.nextSegmentIndex[0] == faceSegmentIndexBefore)
      {
        distance -= segmentLength;
        face.segmentDirectionInverted = (speed < 0);
      }
      else
      {
        distance = (getSegmentLength(nextSegment, m_stateData) + segmentLength) - distance;
        face.segmentDirectionInverted = (speed > 0);
      }

      if(nextSegment.sensor.index != invalidIndex)
      {
        m_stateData.sensors[nextSegment.sensor.index].occupied++;
      }
    }
    else if(distance < 0)
    {
      const auto nextSegmentIndex = getNextSegmentIndex(segment, false, m_stateData);
      if(nextSegmentIndex == invalidIndex)
      {
        return false; // no next segment
      }

      if(segment.sensor.index != invalidIndex)
      {
        auto& sensor = m_stateData.sensors[segment.sensor.index];
        assert(sensor.occupied != 0);
        sensor.occupied--;
      }

      face.segmentIndex = nextSegmentIndex;
      auto& nextSegment = staticData.trackSegments[face.segmentIndex];

      if(nextSegment.nextSegmentIndex[0] == faceSegmentIndexBefore)
      {
        distance = -distance;
        face.segmentDirectionInverted = (speed < 0);
      }
      else
      {
        distance += getSegmentLength(nextSegment, m_stateData);
        face.segmentDirectionInverted = (speed > 0);
      }

      if(nextSegment.sensor.index != invalidIndex)
      {
        m_stateData.sensors[nextSegment.sensor.index].occupied++;
      }
    }
  }

  // Calculate position:
  {
    auto& segment = staticData.trackSegments[face.segmentIndex];
    size_t curveIndex = invalidIndex;

    if(isStraight(segment))
    {
      face.position.x = segment.points[0].x + distance * std::cos(segment.rotation);
      face.position.y = segment.points[0].y + distance * std::sin(segment.rotation);
    }
    else if(isCurve(segment, curveIndex))
    {
      assert(curveIndex != invalidIndex);
      const auto& curve = segment.curves[curveIndex];

      float angle = segment.rotation + (distance / curve.length) * curve.angle;
      if(curve.angle < 0)
      {
        angle += pi;
      }

      face.position.x = curve.center.x + curve.radius * std::sin(angle);
      face.position.y = curve.center.y - curve.radius * std::cos(angle);
    }
    else
    {
      assert(false);
    }
  }

  face.distance = distance;

  return true;
}

void Simulator::updateSensors()
{
  const size_t count = staticData.sensors.size();
  for(size_t i = 0; i < count; ++i)
  {
    const auto& sensor = staticData.sensors[i];
    auto& sensorState = m_stateData.sensors[i];
    const bool sensorValue = m_stateData.powerOn && (sensorState.occupied != 0);
    if(sensorState.value != sensorValue)
    {
      sensorState.value = sensorValue;
      send(SimulatorProtocol::SensorChanged(sensor.channel, sensor.address, sensorState.value));
    }
  }
}

bool Simulator::isStraight(const TrackSegment& segment)
{
  using Type = TrackSegment::Type;
  using State = TurnoutState::State;

  switch(segment.type)
  {
    case Type::Straight:
      return true;

    case Type::Curve:
    case Type::TurnoutCurved:
      return false;

    case Type::Turnout:
    case Type::Turnout3Way:
      return m_stateData.turnouts[segment.turnout.index].state == State::Closed;
  }
  assert(false);
  return false;
}

bool Simulator::isCurve(const TrackSegment& segment, size_t& curveIndex)
{
  using Type = TrackSegment::Type;
  using State = TurnoutState::State;

  switch(segment.type)
  {
    case Type::Straight:
      return false;

    case Type::Curve:
      curveIndex = 0;
      return true;

    case Type::Turnout:
      if(m_stateData.turnouts[segment.turnout.index].state == State::Thrown)
      {
        curveIndex = 0;
        return true;
      }
      return false;

    case Type::TurnoutCurved:
      curveIndex = m_stateData.turnouts[segment.turnout.index].state == State::Closed ? 0 : 1;
      return true;

    case Type::Turnout3Way:
      switch(m_stateData.turnouts[segment.turnout.index].state)
      {
        case State::Closed:
          return false;

        case State::ThrownLeft:
          curveIndex = 0;
          return true;

        case State::ThrownRight:
          curveIndex = 1;
          return true;

        default:
          break;
      }
      break;
  }
  assert(false);
  return false;
}

Simulator::StaticData Simulator::load(const nlohmann::json& world, StateData& stateData)
{
  StaticData data;

  std::unordered_map<std::string_view, size_t> trackSegmentId;

  if(auto trackPlan = world.find("trackplan"); trackPlan != world.end() && trackPlan->is_array())
  {
    enum class Side
    {
      Origin = 0,
      End = 1,
      TurnoutThrown = 2,
    };

    data.trackSegments.reserve(trackPlan->size());

    Side lastSide = Side::Origin;
    size_t lastSegmentIndex = invalidIndex;
    Point curPoint{0.0f, 0.0f};
    float curRotation = 0;

    for(const auto& obj : *trackPlan)
    {
      if(!obj.is_object())
      {
        continue;
      }

      Side side = Side::Origin;
      size_t nextPointIndex = 1;
      TrackSegment segment;

      if(auto id = obj.value<std::string_view>("id", {}); !id.empty())
      {
        trackSegmentId.emplace(std::move(id), data.trackSegments.size());
      }

      const auto type = obj.value<std::string_view>("type", {});
      if(type == "straight")
      {
        segment.type = TrackSegment::Type::Straight;
      }
      else if(type == "curve")
      {
        segment.type = TrackSegment::Type::Curve;
      }
      else if(type == "turnout")
      {
        segment.type = TrackSegment::Type::Turnout;
      }
      else if(type == "turnout_curved")
      {
        segment.type = TrackSegment::Type::TurnoutCurved;
      }
      else if(type == "turnout_3way")
      {
        segment.type = TrackSegment::Type::Turnout3Way;
      }
      else
      {
        throw std::runtime_error("unknown track element type");
      }

      if(segment.type == TrackSegment::Type::Straight || segment.type == TrackSegment::Type::Turnout || segment.type == TrackSegment::Type::Turnout3Way)
      {
        segment.straight.length = obj.value("length", segment.straight.length);
      }
      if(segment.type == TrackSegment::Type::Curve ||
          segment.type == TrackSegment::Type::Turnout ||
          segment.type == TrackSegment::Type::TurnoutCurved)
      {
        segment.curves[0].radius = obj.value("radius", 100.0f);
        segment.curves[0].angle = deg2rad(obj.value("angle", 45.0f));
      }
      if(segment.type == TrackSegment::Type::TurnoutCurved)
      {
        segment.curves[1].radius = obj.value("radius_2", 100.0f);
        segment.curves[1].angle = deg2rad(obj.value("angle_2", 45.0f));
      }
      if(segment.type == TrackSegment::Type::Turnout3Way)
      {
        const float angleMin = 0.0f;
        const float angleMax = 90.0f;
        const float radius = std::max(1.0f, obj.value("radius", 100.0f));
        const float angle = std::clamp(obj.value("angle", 45.0f), angleMin, angleMax);

        segment.curves[0].radius = std::max(1.0f,obj.value("radius_left", radius));
        segment.curves[0].angle = deg2rad(-std::clamp(obj.value("angle_left", angle), angleMin, angleMax));

        segment.curves[1].radius = std::max(1.0f,obj.value("radius_right", radius));
        segment.curves[1].angle = deg2rad(std::clamp(obj.value("angle_right", angle), angleMin, angleMax));
      }
      if(segment.type == TrackSegment::Type::Turnout || segment.type == TrackSegment::Type::TurnoutCurved || segment.type == TrackSegment::Type::Turnout3Way)
      {
        if(segment.type == TrackSegment::Type::Turnout3Way)
        {
          const auto address = obj.value("address", invalidAddress);
          segment.turnout.addresses[0] = obj.value("address_left", address);
          segment.turnout.addresses[1] = obj.value("address_right", (address != invalidAddress) ? (address + 1) : invalidAddress);
        }
        else
        {
          segment.turnout.addresses[0] = obj.value("address", invalidAddress);
        }
        segment.turnout.index = stateData.turnouts.size();
        auto& turnoutState = stateData.turnouts.emplace_back(TurnoutState{});
        if(segment.type == TrackSegment::Type::Turnout3Way)
        {
          turnoutState.coils = 0x0A; // both closed
        }

        if(auto it = obj.find("side"); it != obj.end())
        {
          if(it->is_number_unsigned())
          {
            if(const auto n = obj.value<size_t>("side", invalidIndex); n < getConnectorCount(segment.type))
            {
              side = static_cast<Side>(n);
            }
          }
          else if(it->is_string()) // deprecated, remove once all side stuff is index based.
          {
            const auto sideStr = obj.value<std::string_view>("side", {});
            if(sideStr == "straight")
            {
              side = Side::End;
            }
            else if(sideStr == "curve")
            {
              side = Side::TurnoutThrown;
            }
          }
        }
      }

      if(obj.contains("start"))
      {
        auto start = obj.value<std::string_view>("start", {});
        const int startPoint = obj.value("start_point", -1);
        if(auto it = trackSegmentId.find(start); it != trackSegmentId.end())
        {
          auto& startSegment = data.trackSegments[it->second];
          if(startSegment.nextSegmentIndex[0] == invalidIndex && (startPoint == -1 || startPoint == 0))
          {
            curPoint = startSegment.origin();
            curRotation = startSegment.rotation + pi;

            lastSide = Side::Origin;
            lastSegmentIndex = it->second;
          }
          else if(startSegment.nextSegmentIndex[1] == invalidIndex && (startPoint == -1 || startPoint == 1))
          {
            curPoint = startSegment.points[1];
            curRotation = startSegment.rotation;
            if(startSegment.type == TrackSegment::Type::Curve || startSegment.type == TrackSegment::Type::TurnoutCurved)
            {
              curRotation += startSegment.curves[0].angle;
            }

            lastSide = Side::End;
            lastSegmentIndex = it->second;
          }
          else if((startSegment.type == TrackSegment::Type::Turnout || startSegment.type == TrackSegment::Type::TurnoutCurved || startSegment.type == TrackSegment::Type::Turnout3Way) &&
            startSegment.nextSegmentIndex[2] == invalidIndex && (startPoint == -1 || startPoint == 2))
          {
            const size_t curveIndex = (startSegment.type == TrackSegment::Type::TurnoutCurved) ? 1 : 0;
            curPoint = startSegment.points[2];
            curRotation = startSegment.rotation + startSegment.curves[curveIndex].angle;

            lastSide = Side::TurnoutThrown;
            lastSegmentIndex = it->second;
          }
          else if(startSegment.type == TrackSegment::Type::Turnout3Way &&
            startSegment.nextSegmentIndex[3] == invalidIndex && (startPoint == -1 || startPoint == 3))
          {
            const size_t curveIndex = 1;
            curPoint = startSegment.points[3];
            curRotation = startSegment.rotation + startSegment.curves[curveIndex].angle;

            lastSide = static_cast<Side>(3);
            lastSegmentIndex = it->second;
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
          curPoint.x = obj.value("x", 0.0f);
          lastSegmentIndex = invalidIndex;
        }
        if(obj.contains("y"))
        {
          curPoint.y = obj.value("y", 0.0f);
          lastSegmentIndex = invalidIndex;
        }
        if(obj.contains("rotation"))
        {
          curRotation = deg2rad(obj.value("rotation", 0.0f));
          lastSegmentIndex = invalidIndex;
        }
      }

      if((segment.type == TrackSegment::Type::Turnout || segment.type == TrackSegment::Type::Turnout3Way) && side == Side::End)
      {
        segment.rotation = curRotation + pi;
        if(segment.rotation >= 2 * pi)
        {
          segment.rotation -= 2 * pi;
        }
        segment.points[0].x = curPoint.x + segment.straight.length * std::cos(curRotation);
        segment.points[0].y = curPoint.y + segment.straight.length * std::sin(curRotation);
        nextPointIndex = 0;
      }
      else if((segment.type == TrackSegment::Type::Turnout && side == Side::TurnoutThrown) ||
          (segment.type == TrackSegment::Type::TurnoutCurved && (side == Side::End || side == Side::TurnoutThrown)) ||
          (segment.type == TrackSegment::Type::Turnout3Way && (side == static_cast<Side>(2) || side == static_cast<Side>(3))))
      {
        const size_t curveIndex = ((segment.type == TrackSegment::Type::TurnoutCurved && side == Side::TurnoutThrown) || (segment.type == TrackSegment::Type::Turnout3Way && side == static_cast<Side>(3))) ? 1 : 0;
        auto& curve = segment.curves[curveIndex];

        const float curAngle = (curve.angle > 0) ? (curRotation + pi) : curRotation;

        // Calc circle center:
        curve.center.x = curPoint.x - curve.radius * std::sin(curAngle);
        curve.center.y = curPoint.y + curve.radius * std::cos(curAngle);

        // Calc origin:
        segment.points[0].x = curve.center.x - curve.radius * std::sin(curRotation - curve.angle + pi);
        segment.points[0].y = curve.center.y + curve.radius * std::cos(curRotation - curve.angle + pi);

        curRotation -= curve.angle;

        segment.rotation = curRotation + pi;
        if(segment.rotation >= 2 * pi)
        {
          segment.rotation -= 2 * pi;
        }

        nextPointIndex = 0;
      }
      else
      {
        segment.points[0] = curPoint;
        segment.rotation = curRotation;

        if(segment.type == TrackSegment::Type::Curve || segment.type == TrackSegment::Type::TurnoutCurved)
        {
          curRotation += segment.curves[0].angle;
        }
      }

      assert(segment.points[0].isFinite()); // origin must be known now

      // Calculate center/length of curves:
      for(size_t i = 0; i < getCurveCount(segment.type); ++i)
      {
        auto& curve = segment.curves[i];
        if(!curve.center.isFinite()) // skip if already known
        {
          const float startAngle = (curve.angle < 0) ? (segment.rotation + pi) : segment.rotation;
          curve.center.x = segment.points[0].x - curve.radius * std::sin(startAngle);
          curve.center.y = segment.points[0].y + curve.radius * std::cos(startAngle);

          curve.length = std::abs(curve.radius * curve.angle);
        }
        assert(curve.center.isFinite());
      }

      // Calculate points if not known:
      switch(segment.type)
      {
        case TrackSegment::Type::Straight:
          if(!segment.points[1].isFinite())
          {
            segment.points[1] = straightEnd(segment);
          }
          break;

        case TrackSegment::Type::Curve:
          if(!segment.points[1].isFinite())
          {
            segment.points[1] = curveEnd(segment, 0);
          }
          break;

        case TrackSegment::Type::Turnout:
          if(!segment.points[1].isFinite())
          {
            segment.points[2] = straightEnd(segment);
          }
          if(!segment.points[1].isFinite())
          {
            segment.points[2] = curveEnd(segment, 0);
          }
          break;

        case TrackSegment::Type::TurnoutCurved:
          if(!segment.points[1].isFinite())
          {
            segment.points[1] = curveEnd(segment, 0);
          }
          if(!segment.points[2].isFinite())
          {
            segment.points[2] = curveEnd(segment, 1);
          }
          break;

        case TrackSegment::Type::Turnout3Way:
          if(!segment.points[1].isFinite())
          {
            segment.points[1] = straightEnd(segment);
          }
          if(!segment.points[2].isFinite())
          {
            segment.points[2] = curveEnd(segment, 0);
          }
          if(!segment.points[3].isFinite())
          {
            segment.points[3] = curveEnd(segment, 1);
          }
          break;

        default:
          assert(false);
          break;
      }
#ifndef NDEBUG
      for(size_t i = 1; i < getConnectorCount(segment.type); ++i)
      {
        assert(segment.points[i].isFinite()); // all points must be known now
      }
#endif

      // Set current point:
      assert(nextPointIndex < getConnectorCount(segment.type));
      curPoint = segment.points[nextPointIndex];

      // Sensors:
      if(const uint16_t sensorAddress = obj.value("sensor_address", invalidAddress); sensorAddress != invalidAddress)
      {
        const uint16_t sensorChannel = obj.value("sensor_channel", defaultChannel);

        for(size_t i = 0; i < data.sensors.size(); ++i)
        {
          if(data.sensors[i].channel == sensorChannel && data.sensors[i].address == sensorAddress)
          {
            segment.sensor.index = i;
            break;
          }
        }

        if(segment.sensor.index == invalidIndex) // new sensor
        {
          segment.sensor.index = data.sensors.size();
          data.sensors.emplace_back(Sensor{sensorChannel, sensorAddress});
          stateData.sensors.emplace_back(SensorState{0, false});
        }
      }

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
            assert(segment.type == TrackSegment::Type::Turnout || segment.type == TrackSegment::Type::TurnoutCurved);
            segment.nextSegmentIndex[2] = lastSegmentIndex;
            break;

          default:
            assert(false);
            break;
        }

        assert(static_cast<size_t>(lastSide) < getConnectorCount(data.trackSegments[lastSegmentIndex].type));
        data.trackSegments[lastSegmentIndex].nextSegmentIndex[static_cast<size_t>(lastSide)] = data.trackSegments.size();
      }

      data.trackSegments.emplace_back(std::move(segment));

      switch(static_cast<size_t>(side))
      {
        case 0:
          lastSide = Side::End;
          if(segment.type == TrackSegment::Type::Turnout || segment.type == TrackSegment::Type::TurnoutCurved)
          {
            if(segment.nextSegmentIndex[1] != invalidIndex)
            {
              lastSide = Side::TurnoutThrown;
            }
          }
          if(segment.type == TrackSegment::Type::Turnout3Way)
          {
            if(segment.nextSegmentIndex[1] != invalidIndex)
            {
              lastSide = Side::TurnoutThrown;
            }
            if(segment.nextSegmentIndex[2] != invalidIndex)
            {
              lastSide = static_cast<Side>(3);
            }
          }
          break;

        case 1:
        case 2:
        case 3:
          lastSide = Side::Origin;
          break;

        default:
          assert(false);
          break;
      }

      lastSegmentIndex = data.trackSegments.size() - 1;
    }

    // Connect open ends:
    {
      auto findSegment = [&data](size_t start, Point point, size_t& index)
      {
        const size_t count = data.trackSegments.size();
        for(size_t i = start + 1; i < count; ++i)
        {
          auto& segment = data.trackSegments[i];
          const auto connectorCount = getConnectorCount(segment.type);
          for(size_t j = 0; j < connectorCount; ++j)
          {
            if(segment.nextSegmentIndex[j] == invalidIndex && pointsClose(point, segment.points[j]))
            {
              segment.nextSegmentIndex[j] = start;
              index = i;
              return;
            }
          }
        }
      };

      const size_t count = data.trackSegments.size() - 1;
      for(size_t i = 0; i < count; ++i)
      {
        auto& segment = data.trackSegments[i];
        const auto connectorCount = getConnectorCount(segment.type);
        for(size_t j = 0; j < connectorCount; ++j)
        {
          if(segment.nextSegmentIndex[j] == invalidIndex)
          {
            findSegment(i, segment.points[j], segment.nextSegmentIndex[j]);
          }
        }
      }
    }
  }

  if(auto trains = world.find("trains"); trains != world.end() && trains->is_array())
  {
    for(const auto& object : *trains)
    {
      if(!object.is_object() || !object.contains("vehicles") || !object["vehicles"].is_array())
      {
        continue;
      }

      Train train;
      size_t segmentIndex = invalidIndex;

      if(const auto trackId = object.value<std::string_view>("track_id", {}); !trackId.empty())
      {
        if(auto it = trackSegmentId.find(trackId); it != trackSegmentId.end())
        {
          segmentIndex = it->second;
        }
      }

      if(segmentIndex == invalidIndex)
      {
        segmentIndex = 0; // in case there is no free segment
        for(size_t i = 0; i < data.trackSegments.size(); ++i)
        {
          const auto& segment = data.trackSegments[i];
          if(segment.hasSensor() && stateData.sensors[segment.sensor.index].occupied == 0)
          {
            segmentIndex = i;
            break;
          }
        }
      }

      train.protocol = stringToEnum<DecoderProtocol>(object.value<std::string_view>("protocol", {})).value_or(DecoderProtocol::None);
      train.address = object.value("address", train.address);

      for(const auto& vehicle : object["vehicles"])
      {
        if(!vehicle.is_object())
        {
          continue;
        }
        const float length = vehicle.value("length", 20.0f);
        const auto color = stringToEnum<Color>(vehicle.value<std::string_view>("color", {})).value_or(Color::Red);
        const float distance = train.vehicleIndexes.empty() ? 0.0f : stateData.vehicles[train.vehicleIndexes.back()].rear.distance - data.trainCouplingLength;
        train.length += length + (train.vehicleIndexes.empty() ? 0.0f : data.trainCouplingLength);
        train.vehicleIndexes.emplace_back(data.vehicles.size());
        data.vehicles.emplace_back(Vehicle{color, length});
        auto& vehicleState = stateData.vehicles.emplace_back(VehicleState{});
        vehicleState.front.segmentIndex = segmentIndex;
        vehicleState.front.distance = distance;
        vehicleState.rear.segmentIndex = segmentIndex;
        vehicleState.rear.distance = distance - length;
      }

      if(!train.vehicleIndexes.empty())
      {
        // center train in segment and mark it occupied:
        auto& segment = data.trackSegments[segmentIndex];
        if(segment.sensor.index != invalidIndex)
        {
          auto& sensor = stateData.sensors[segment.sensor.index];
          sensor.occupied = train.vehicleIndexes.size() * 2;
          sensor.value = (sensor.occupied != 0);
        }
        const float segmentLength = getSegmentLength(segment, stateData);
        const float move = segmentLength - (segmentLength - train.length) / 2;
        for(const auto& vehicleIndex : train.vehicleIndexes)
        {
          auto& vehicle = stateData.vehicles[vehicleIndex];
          vehicle.front.distance += move;
          vehicle.rear.distance += move;
        }
        data.trains.emplace_back(std::move(train));
        stateData.trains.emplace_back(TrainState{});
      }
    }
  }

  data.trainWidth = world.value("train_width", data.trainWidth);

  assert(data.sensors.size() == stateData.sensors.size());
  assert(data.trains.size() == stateData.trains.size());
  assert(data.vehicles.size() == stateData.vehicles.size());

  return data;
}
