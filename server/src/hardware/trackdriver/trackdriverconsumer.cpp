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

#include "trackdriverconsumer.hpp"
#include "trackdriver.hpp"
#include "trackdrivercontroller.hpp"
#include "../../core/attributes.hpp"
#include "../../core/eventloop.hpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../utils/category.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../utils/valuestep.hpp"
#include "../../world/world.hpp"

namespace {

constexpr auto addressDefault = std::numeric_limits<uint32_t>::max();
constexpr auto addressRangeFull = std::make_pair(std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());

}

TrackDriverConsumer::TrackDriverConsumer(Object& object, const World& world)
  : interface{&object, "interface", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript,
      [this](const std::shared_ptr<TrackDriverController>& /*newValue*/)
      {
        interfaceChanged();
      },
      [this](const std::shared_ptr<TrackDriverController>& newValue)
      {
        if(interface.value())
        {
          releaseTrackDriver();
        }

        if(newValue)
        {
          if(const auto addressMinMax = newValue->trackDriverAddressMinMax(); !inRange(address.value(), addressMinMax))
          {
            const auto addr = newValue->getUnusedTrackDriverAddress();
            address.setValueInternal(addr ? *addr : addressMinMax.first);
          }

          setTrackDriver(newValue->getTrackDriver(address, m_object));
        }

        return true;
      }}
  , address{&object, "address", addressDefault, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript, nullptr,
      [this](const uint32_t& newValue)
      {
        if(auto obj = interface->getTrackDriver(newValue, m_object))
        {
          setTrackDriver(obj);
          return true;
        }
        return false;
      }}
  , invertPolarity{&object, "invert_polarity", false, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript}
  , m_object{object}
{
  const auto worldState = world.state.value();
  const bool editable = contains(worldState, WorldState::Edit);
  const bool editableAndStopped = editable && !contains(worldState, WorldState::Run);

  Attributes::addCategory(interface, Category::trackDriver);
  Attributes::addDisplayName(interface, DisplayName::Hardware::interface);
  Attributes::addEnabled(interface, editableAndStopped);
  Attributes::addObjectList(interface, world.trackDriverControllers);

  Attributes::addCategory(address, Category::trackDriver);
  Attributes::addDisplayName(address, DisplayName::Hardware::address);
  Attributes::addEnabled(address, editableAndStopped);
  Attributes::addVisible(address, false);
  Attributes::addMinMax(address, addressRangeFull);

  Attributes::addCategory(invertPolarity, Category::trackDriver);
  Attributes::addEnabled(invertPolarity, editableAndStopped);
  Attributes::addVisible(invertPolarity, false);
}

TrackDriverConsumer::~TrackDriverConsumer()
{
  releaseTrackDriver();
}

void TrackDriverConsumer::loaded()
{
  if(interface)
  {
    if(auto object = interface->getTrackDriver(address, m_object))
    {
      setTrackDriver(object);
      interfaceChanged();
    }
    else
    {
      interface.setValueInternal(nullptr);
      //! \todo log warning
    }
  }
}

void TrackDriverConsumer::worldEvent(WorldState worldState, WorldEvent /*worldEvent*/)
{
  const bool editable = contains(worldState, WorldState::Edit);
  const bool editableAndStopped = editable && !contains(worldState, WorldState::Run);

  Attributes::setEnabled({interface, address, invertPolarity}, editableAndStopped);
}

void TrackDriverConsumer::addInterfaceItems(InterfaceItems& items)
{
  items.add(interface);
  items.add(address);
  items.add(invertPolarity);
}

void TrackDriverConsumer::setTrackDriver(std::shared_ptr<TrackDriver> value)
{
  releaseTrackDriver();
  assert(!m_trackDriver);
  m_trackDriver = value;
  if(m_trackDriver)
  {
    m_trackDriverDestroying = m_trackDriver->onDestroying.connect(
      [this](Object& object)
      {
        (void)object; // silence unused warning
        assert(m_trackDriver.get() == &object);
        interface.setValue(nullptr);
      });
  }
}

void TrackDriverConsumer::releaseTrackDriver()
{
  if(m_trackDriver)
  {
    m_trackDriverDestroying.disconnect();
    if(m_trackDriver->interface)
    {
      m_trackDriver->interface->releaseTrackDriver(*m_trackDriver, m_object);
    }
    m_trackDriver.reset();
  }
}

void TrackDriverConsumer::interfaceChanged()
{
  Attributes::setMinMax(address, interface ? interface->trackDriverAddressMinMax() : addressRangeFull);
  Attributes::setVisible({address, invertPolarity}, interface);
}
