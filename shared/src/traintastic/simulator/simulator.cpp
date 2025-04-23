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

Simulator::Point straightEnd(const Simulator::TrackSegment& segment)
{
  assert(segment.type == Simulator::TrackSegment::Type::Straight || segment.type == Simulator::TrackSegment::Type::Turnout);
  return {segment.points[0].x + segment.straight.length * std::cos(segment.rotation),
    segment.points[0].y + segment.straight.length * std::sin(segment.rotation)};
}

Simulator::Point curveEnd(const Simulator::TrackSegment& segment)
{
  assert(segment.type == Simulator::TrackSegment::Type::Curve || segment.type == Simulator::TrackSegment::Type::Turnout);

  const float angle = (segment.curve.angle < 0) ? (segment.rotation + pi) : segment.rotation;
  float cx;
  float cy;

  if(segment.type == Simulator::TrackSegment::Type::Curve)
  {
    cx = segment.points[0].x;
    cy = segment.points[0].y;
  }
  else
  {
    cx = segment.points[0].x - segment.curve.radius * std::sin(angle);
    cy = segment.points[0].y + segment.curve.radius * std::cos(angle);
  }
  return {cx + segment.curve.radius * std::sin(angle + segment.curve.angle),
    cy - segment.curve.radius * std::cos(angle + segment.curve.angle)};
}

#ifdef _MSC_VER
const // std::abs is not constexpr :(
#else
constexpr
#endif
bool pointsClose(Simulator::Point a, Simulator::Point b)
{
  return std::abs(a.x - b.x) <= 1.0f && std::abs(a.y - b.y) <= 1.0f;
}

constexpr size_t getConnectorCount(Simulator::TrackSegment::Type type)
{
  switch(type)
  {
    using enum Simulator::TrackSegment::Type;

    case Straight:
    case Curve:
      return 2;

    case Turnout:
      return 3;
  }
  return 0;
}

float getSegmentLength(const Simulator::TrackSegment& segment, const Simulator::StateData& stateData)
{
  switch(segment.type)
  {
    using enum Simulator::TrackSegment::Type;

    case Straight:
      return segment.straight.length;

    case Curve:
      return segment.curve.length;

    case Turnout:
      switch(stateData.turnouts[segment.turnout.index].state)
      {
        using enum Simulator::TurnoutState::State;

        case Closed:
          return segment.straight.length;

        case Thrown:
          return segment.curve.length;
      }
      break;
  }
  return 0.0f;
}

size_t getNextSegmentIndex(const Simulator::TrackSegment& segment, bool directionPositive, const Simulator::StateData& stateData)
{
  if(!directionPositive)
  {
    return segment.nextSegmentIndex[0];
  }
  if(segment.type == Simulator::TrackSegment::Type::Turnout && stateData.turnouts[segment.turnout.index].state == Simulator::TurnoutState::State::Thrown)
  {
    return segment.nextSegmentIndex[2];
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
  if(segmentIndex < staticData.trackSegments.size() && staticData.trackSegments[segmentIndex].turnout.index != invalidIndex)
  {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    auto& turnout = m_stateData.turnouts[staticData.trackSegments[segmentIndex].turnout.index];
    if(turnout.state != state)
    {
      turnout.state = state;
      send(SimulatorProtocol::AccessorySetState(0, staticData.trackSegments[segmentIndex].turnout.address, turnout.state == TurnoutState::State::Thrown ? 1 : 2));
    }
  }
}

void Simulator::toggleTurnoutState(size_t segmentIndex)
{
  if(segmentIndex < staticData.trackSegments.size() && staticData.trackSegments[segmentIndex].turnout.index != invalidIndex)
  {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    auto& currentState = m_stateData.turnouts[staticData.trackSegments[segmentIndex].turnout.index].state;
    currentState = (currentState != TurnoutState::State::Closed) ? TurnoutState::State::Closed : TurnoutState::State::Thrown;
    send(SimulatorProtocol::AccessorySetState(0, staticData.trackSegments[segmentIndex].turnout.address, currentState == TurnoutState::State::Thrown ? 1 : 2));
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
        if(segment.type == Simulator::TrackSegment::Type::Turnout && m.address == segment.turnout.address)
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

    if(segment.type == TrackSegment::Type::Straight || isTurnoutClosed(segment))
    {
      face.position.x = segment.points[0].x + distance * std::cos(segment.rotation);
      face.position.y = segment.points[0].y + distance * std::sin(segment.rotation);
    }
    else if(segment.type == TrackSegment::Type::Curve || isTurnoutThrown(segment))
    {
      float angle = segment.rotation + (distance / segment.curve.length) * segment.curve.angle;
      if(segment.curve.angle < 0)
      {
        angle += pi;
      }

      if(segment.type == TrackSegment::Type::Curve)
      {
        face.position.x = segment.points[0].x + segment.curve.radius * std::sin(angle);
        face.position.y = segment.points[0].y - segment.curve.radius * std::cos(angle);
      }
      else
      {
        const float rotation = segment.curve.angle < 0 ? segment.rotation + pi : segment.rotation;
        const float cx = segment.points[0].x - segment.curve.radius * std::sin(rotation);
        const float cy = segment.points[0].y + segment.curve.radius * std::cos(rotation);

        face.position.x = cx + segment.curve.radius * std::sin(angle);
        face.position.y = cy - segment.curve.radius * std::cos(angle);
      }
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

bool Simulator::isTurnoutClosed(const TrackSegment& segment)
{
  return segment.type == TrackSegment::Type::Turnout && m_stateData.turnouts[segment.turnout.index].state == Simulator::TurnoutState::State::Closed;
}

bool Simulator::isTurnoutThrown(const TrackSegment& segment)
{
  return segment.type == TrackSegment::Type::Turnout && m_stateData.turnouts[segment.turnout.index].state == Simulator::TurnoutState::State::Thrown;
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
    float curX = 0;
    float curY = 0;
    float curRotation = 0;

    for(const auto& obj : *trackPlan)
    {
      if(!obj.is_object())
      {
        continue;
      }

      Side side = Side::Origin;
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
        segment.turnout.address = obj.value("address", segment.turnout.address);
        segment.turnout.index = stateData.turnouts.size();
        stateData.turnouts.emplace_back(TurnoutState{});

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
      else
      {
        throw std::runtime_error("unkonen track element type");
      }

      if(segment.type == TrackSegment::Type::Straight || segment.type == TrackSegment::Type::Turnout)
      {
        segment.straight.length = obj.value("length", segment.straight.length);
      }
      if(segment.type == TrackSegment::Type::Curve || segment.type == TrackSegment::Type::Turnout)
      {
        segment.curve.radius = obj.value("radius", 100.0f);
        segment.curve.angle = deg2rad(obj.value("angle", 45.0f));
        segment.curve.length = std::abs(segment.curve.radius * segment.curve.angle);
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
            const auto pt = startSegment.origin();
            curX = pt.x;
            curY = pt.y;
            curRotation = startSegment.rotation;

            lastSide = Side::Origin;
            lastSegmentIndex = it->second;
          }
          else if(startSegment.nextSegmentIndex[1] == invalidIndex && (startPoint == -1 || startPoint == 1))
          {
            const auto pt = startSegment.points[1];
            curX = pt.x;
            curY = pt.y;
            curRotation = startSegment.rotation;
            if(startSegment.type == TrackSegment::Type::Curve)
            {
              curRotation += startSegment.curve.angle;
            }

            lastSide = Side::End;
            lastSegmentIndex = it->second;
          }
          else if(startSegment.type == TrackSegment::Type::Turnout && startSegment.nextSegmentIndex[2] == invalidIndex && (startPoint == -1 || startPoint == 2))
          {
            const auto pt = startSegment.points[2];
            curX = pt.x;
            curY = pt.y;
            curRotation = startSegment.rotation + startSegment.curve.angle;

            lastSide = Side::TurnoutThrown;
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
          curX = obj.value("x", 0.0f);
          lastSegmentIndex = invalidIndex;
        }
        if(obj.contains("y"))
        {
          curY = obj.value("y", 0.0f);
          lastSegmentIndex = invalidIndex;
        }
        if(obj.contains("rotation"))
        {
          curRotation = obj.value("rotation", 0.0f);
          lastSegmentIndex = invalidIndex;
        }
      }

      if(segment.type == TrackSegment::Type::Turnout && side == Side::End)
      {
        segment.rotation = curRotation + pi;
        if(segment.rotation >= 2 * pi)
        {
          segment.rotation -= 2 * pi;
        }
        segment.points[0].x = curX + segment.straight.length * std::cos(curRotation);
        segment.points[0].y = curY + segment.straight.length * std::sin(curRotation);
        curX = segment.points[0].x;
        curY = segment.points[0].y;
      }
      else if(segment.type == TrackSegment::Type::Turnout && side == Side::TurnoutThrown)
      {
        const float curAngle = (segment.curve.angle < 0) ? curRotation : (curRotation + pi);

        // Calc circle center:
        segment.points[0].x = curX - segment.curve.radius * std::sin(curAngle);
        segment.points[0].y = curY + segment.curve.radius * std::cos(curAngle);

        // Calc end point:
        curX = segment.points[0].x + segment.curve.radius * std::sin(-curAngle + segment.curve.angle);
        curY = segment.points[0].y - segment.curve.radius * std::cos(-curAngle + segment.curve.angle);

        curRotation -= segment.curve.angle;

        segment.rotation = curRotation + pi;
        if(segment.rotation >= 2 * pi)
        {
          segment.rotation -= 2 * pi;
        }

        segment.points[0].x = curX;
        segment.points[0].y = curY;
      }
      else
      {
        segment.points[0].x = curX;
        segment.points[0].y = curY;
        segment.rotation = curRotation;

        if(segment.type == TrackSegment::Type::Straight || segment.type == TrackSegment::Type::Turnout)
        {
          curX += segment.straight.length * std::cos(curRotation);
          curY += segment.straight.length * std::sin(curRotation);
        }
        else if(segment.type == TrackSegment::Type::Curve)
        {
          const float curAngle = (segment.curve.angle < 0) ? (curRotation + pi) : curRotation;

          // Calc circle center:
          segment.points[0].x = curX - segment.curve.radius * std::sin(curAngle);
          segment.points[0].y = curY + segment.curve.radius * std::cos(curAngle);

          // Calc end point:
          curX = segment.points[0].x + segment.curve.radius * std::sin(curAngle + segment.curve.angle);
          curY = segment.points[0].y - segment.curve.radius * std::cos(curAngle + segment.curve.angle);

          curRotation += segment.curve.angle;
        }
      }

      if(segment.type == TrackSegment::Type::Straight || segment.type == TrackSegment::Type::Turnout)
      {
        segment.points[1] = straightEnd(segment);
      }
      if(segment.type == TrackSegment::Type::Curve || segment.type == TrackSegment::Type::Turnout)
      {
        segment.points[(segment.type == TrackSegment::Type::Curve) ? 1 : 2] = curveEnd(segment);
      }

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
            assert(segment.type == TrackSegment::Type::Turnout);
            segment.nextSegmentIndex[2] = lastSegmentIndex;
            break;

          default:
            assert(false);
            break;
        }

        switch(lastSide)
        {
          case Side::Origin:
            data.trackSegments[lastSegmentIndex].nextSegmentIndex[0] = data.trackSegments.size();
            break;

          case Side::End:
            data.trackSegments[lastSegmentIndex].nextSegmentIndex[1] = data.trackSegments.size();
            break;

          case Side::TurnoutThrown:
            assert(data.trackSegments[lastSegmentIndex].type == TrackSegment::Type::Turnout);
            data.trackSegments[lastSegmentIndex].nextSegmentIndex[2] = data.trackSegments.size();
            break;

          default:
            assert(false);
            break;
        }
      }
      data.trackSegments.emplace_back(std::move(segment));

      switch(side)
      {
        case Side::Origin:
          lastSide = Side::End;
          if(segment.type == TrackSegment::Type::Turnout)
          {
              if(segment.nextSegmentIndex[1] != invalidIndex)
                  lastSide = Side::TurnoutThrown;
          }
          break;

        case Side::End:
        case Side::TurnoutThrown:
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
            break;
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
