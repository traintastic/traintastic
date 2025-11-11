/**
 * server/src/hardware/interface/Marklin6050Interface.cpp
 *
 * Dummy Märklin 6050 interface for Traintastic
 * © 2025
 */

#include "Marklin6050Interface.hpp"
#include "../../core/attributes.hpp"
#include "../../world/world.hpp"
#include "../../utils/displayname.hpp"

#include <vector>
#include <string>
#include <algorithm>

CREATE_IMPL(Marklin6050Interface)

// Platform-independent serial enumeration
namespace Serial {
    inline std::vector<std::string> getPortList() {
        std::vector<std::string> ports;

#if defined(_WIN32)
        for (int i = 1; i <= 256; ++i)
            ports.push_back("COM" + std::to_string(i));
#elif defined(__APPLE__)
        ports.push_back("/dev/tty.usbserial");
        ports.push_back("/dev/tty.usbmodem");
#else
        ports.push_back("/dev/ttyS0");
        ports.push_back("/dev/ttyS1");
        ports.push_back("/dev/ttyUSB0");
        ports.push_back("/dev/ttyUSB1");
#endif
        return ports;
    }

    inline bool isValidPort(const std::string& port) {
        auto ports = getPortList();
        return std::find(ports.begin(), ports.end(), port) != ports.end();
    }

    inline bool testOpen(const std::string& port) {
        (void)port;
        return true;
    }
}

Marklin6050Interface::Marklin6050Interface(World& world, std::string_view idValue)
    : Interface(world, idValue),
      serialPort(this, "serialPort", "", PropertyFlags{},   // <- fixed
                 [this](const std::string& value) {
                     serialPortChanged(value);
                 })
{
    name = "Märklin 6050";

    // Populate serial port choices
    auto availablePorts = Serial::getPortList();
    if (!availablePorts.empty())
        serialPort = availablePorts.front();
}

void Marklin6050Interface::addToWorld() {
    Interface::addToWorld();
}

void Marklin6050Interface::loaded() {
    Interface::loaded();
    updateEnabled();
}

void Marklin6050Interface::destroying() {
    Interface::destroying();
}

void Marklin6050Interface::worldEvent(WorldState state, WorldEvent event) {
    Interface::worldEvent(state, event);
    switch (event) {
        case WorldEvent::PowerOn:
        case WorldEvent::PowerOff:
        default:
            break;
    }
}

bool Marklin6050Interface::setOnline(bool& value, bool /*simulation*/) {
    if (value) {
        std::string port = serialPort.value();
        if (!Serial::isValidPort(port) || !Serial::testOpen(port)) {
            value = false;
            setState(InterfaceState::Offline);
            return false;
        }
        setState(InterfaceState::Online);
    } else {
        setState(InterfaceState::Offline);
    }
    updateEnabled();
    return true;
}

void Marklin6050Interface::updateEnabled() {
    // Add any UI or logic updates
}

void Marklin6050Interface::serialPortChanged(const std::string& newPort) {
    (void)newPort;
}
