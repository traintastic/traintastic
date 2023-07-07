/**
 * server/src/network/session.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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

#include "session.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/uuid/random_generator.hpp>
#include "../traintastic/traintastic.hpp"
#include "client.hpp"
#include <traintastic/enum/interfaceitemtype.hpp>
#include <traintastic/enum/attributetype.hpp>
#ifndef NDEBUG
  #include "../core/eventloop.hpp" // for: isEventLoopThread()
#endif
#include "../core/objectproperty.tpp"
#include "../core/tablemodel.hpp"
#include "../log/log.hpp"
#include "../log/memorylogger.hpp"
#include "../board/board.hpp"
#include "../board/tile/tiles.hpp"
#include "../hardware/input/monitor/inputmonitor.hpp"
#include "../hardware/output/keyboard/outputkeyboard.hpp"

#ifdef GetObject
  #undef GetObject // GetObject is defined by a winapi header
#endif

Session::Session(const std::shared_ptr<Client>& client) :
  m_client{client},
  m_uuid{boost::uuids::random_generator()()}
{
  assert(isEventLoopThread());
}

Session::~Session()
{
  assert(isEventLoopThread());
  m_memoryLoggerChanged.disconnect();
  for(const auto& it : m_objectSignals)
    it.second.disconnect();
}

bool Session::processMessage(const Message& message)
{
  switch(message.command())
  {
    case Message::Command::GetObject:
    {
      std::string id;
      message.read(id);

      std::vector<std::string> ids;
      boost::split(ids, id, [](char c){ return c == '.'; });
      auto it = ids.cbegin();

      ObjectPtr obj;
      if(*it == Traintastic::classId)
        obj = Traintastic::instance;
      else if(Traintastic::instance->world)
        obj = Traintastic::instance->world->getObjectById(*it);

      while(obj && ++it != ids.cend())
      {
        if(AbstractProperty* property = obj->getProperty(*it); property && property->type() == ValueType::Object)
          obj = property->toObject();
        else if(AbstractVectorProperty* vectorProperty = obj->getVectorProperty(*it); vectorProperty && vectorProperty->type() == ValueType::Object)
        {
          obj = nullptr;
          const size_t size = vectorProperty->size();
          for(size_t i = 0; i < size; i++)
          {
            ObjectPtr v = vectorProperty->getObject(i);
            if(id == v->getObjectId())
            {
              obj = v;
              it++;
              break;
            }
          }
        }
        else
          obj = nullptr;
      }

      if(obj)
      {
        auto response = Message::newResponse(message.command(), message.requestId());
        writeObject(*response, obj);
        m_client->sendMessage(std::move(response));
      }
      else
        m_client->sendMessage(Message::newErrorResponse(message.command(), message.requestId(), Message::ErrorCode::UnknownObject));

      return true;
    }
    case Message::Command::ReleaseObject:
    {
      // client counter value must match server counter value,
      // to make sure no handles are "on the wire"
      //
      const Handle handle = message.read<Handle>();
      const uint32_t counter = message.read<uint32_t>();
      if(counter == m_handles.getCounter(handle))
      {
        m_handles.removeHandle(handle);

        auto it = m_objectSignals.find(handle);
        while(it != m_objectSignals.end())
        {
          it->second.disconnect();
          m_objectSignals.erase(it);
          it = m_objectSignals.find(handle);
        }

        auto event = Message::newEvent(message.command(), sizeof(Handle));
        event->write(handle);
        m_client->sendMessage(std::move(event));
      }
      break;
    }
    case Message::Command::ObjectSetProperty:
    {
      if(message.isRequest() || message.isEvent())
      {
        if(ObjectPtr object = m_handles.getItem(message.read<Handle>()))
        {
          if(AbstractProperty* property = object->getProperty(message.read<std::string>()); property && !property->isInternal())
          {
            try
            {
              switch(message.read<ValueType>())
              {
                case ValueType::Boolean:
                  property->fromBool(message.read<bool>());
                  break;

                case ValueType::Integer:
                  property->fromInt64(message.read<int64_t>());
                  break;

                case ValueType::Float:
                  property->fromDouble(message.read<double>());
                  break;

                case ValueType::String:
                  property->fromString(message.read<std::string>());
                  break;

                default:
                  throw std::runtime_error("invalid value type");
              }
            }
            catch(const std::exception& e) // set property failed
            {
              if(message.isRequest()) // send error response
                m_client->sendMessage(Message::newErrorResponse(message.command(), message.requestId(), e.what()));
              else // send changed event with current value:
                objectPropertyChanged(*property);
            }

            if(message.isRequest()) // send success response
              m_client->sendMessage(Message::newResponse(message.command(), message.requestId()));
          }
          else if(message.isRequest()) // send error response
            m_client->sendMessage(Message::newErrorResponse(message.command(), message.requestId(), "unknown property"));
        }
        else if(message.isRequest()) // send error response
          m_client->sendMessage(Message::newErrorResponse(message.command(), message.requestId(), "unknown object"));
      }
      return true;
    }
    case Message::Command::ObjectSetUnitPropertyUnit:
    {
      if(ObjectPtr object = m_handles.getItem(message.read<Handle>()))
      {
        if(AbstractUnitProperty* property = dynamic_cast<AbstractUnitProperty*>(object->getProperty(message.read<std::string>())); property && !property->isInternal())
        {
          try
          {
            property->setUnitValue(message.read<int64_t>());
          }
          catch(const std::exception&)
          {
            // set unit property unit failed, send changed event with current value:
            objectPropertyChanged(*property);
          }
        }
      }
      break;
    }
    case Message::Command::ObjectGetObjectPropertyObject:
      if(message.isRequest())
      {
        if(ObjectPtr object = m_handles.getItem(message.read<Handle>()))
        {
          if(auto* property = object->getObjectProperty(message.read<std::string>()); property && !property->isInternal())
          {
            if(auto obj = property->toObject())
            {
              auto response = Message::newResponse(message.command(), message.requestId());
              writeObject(*response, obj);
              m_client->sendMessage(std::move(response));
            }
            else
              m_client->sendMessage(Message::newErrorResponse(message.command(), message.requestId(), Message::ErrorCode::UnknownObject));
          }
          else // send error response
            m_client->sendMessage(Message::newErrorResponse(message.command(), message.requestId(), "unknown property"));
        }
        else // send error response
          m_client->sendMessage(Message::newErrorResponse(message.command(), message.requestId(), "unknown object"));

        return true;
      }
      break;

    case Message::Command::ObjectGetObjectVectorPropertyObject:
      if(message.isRequest())
      {
        if(ObjectPtr object = m_handles.getItem(message.read<Handle>()))
        {
          if(auto* property = object->getVectorProperty(message.read<std::string>()); property && !property->isInternal())
          {
            const size_t startIndex = message.read<uint32_t>();
            const size_t endIndex = message.read<uint32_t>();

            if(endIndex >= startIndex && endIndex < property->size())
            {
              auto response = Message::newResponse(message.command(), message.requestId());
              for(size_t i = startIndex; i <= endIndex; i++)
                writeObject(*response, property->getObject(i));
              m_client->sendMessage(std::move(response));
            }
            else // send error response
              m_client->sendMessage(Message::newErrorResponse(message.command(), message.requestId(), "invalid indices"));
          }
          else // send error response
            m_client->sendMessage(Message::newErrorResponse(message.command(), message.requestId(), "unknown property"));
        }
        else // send error response
          m_client->sendMessage(Message::newErrorResponse(message.command(), message.requestId(), "unknown object"));

        return true;
      }
      break;

    case Message::Command::ObjectSetObjectPropertyById:
    {
      if(ObjectPtr object = m_handles.getItem(message.read<Handle>()))
      {
        if(AbstractObjectProperty* property = dynamic_cast<AbstractObjectProperty*>(object->getProperty(message.read<std::string>())); property && !property->isInternal())
        {
          try
          {
            const std::string id = message.read<std::string>();
            if(!id.empty())
            {
              if(ObjectPtr obj = Traintastic::instance->world->getObjectByPath(id))
                property->fromObject(obj);
              else
                throw std::runtime_error("");
            }
            else
              property->fromObject(nullptr);
          }
          catch(const std::exception&)
          {
            // set object property failed, send changed event with current value:
            objectPropertyChanged(*property);
          }
        }
      }
      break;
    }
    case Message::Command::ObjectCallMethod:
    {
      if(ObjectPtr object = m_handles.getItem(message.read<Handle>()))
      {
        if(AbstractMethod* method = object->getMethod(message.read<std::string>()); method && !method->isInternal())
        {
          const ValueType resultType = message.read<ValueType>();
          const uint8_t argumentCount = message.read<uint8_t>();

          Arguments args;
          for(uint8_t i = 0; i < argumentCount; i++)
          {
            switch(message.read<ValueType>())
            {
              case ValueType::Boolean:
                args.push_back(message.read<bool>());
                break;

              case ValueType::Enum:
              case ValueType::Integer:
              case ValueType::Set:
                args.push_back(message.read<int64_t>());
                break;

              case ValueType::Float:
                args.push_back(message.read<double>());
                break;

              case ValueType::String:
              {
                std::string arg = message.read<std::string>();
                if(i < method->argumentTypeInfo().size() && method->argumentTypeInfo()[i].type == ValueType::Object)
                {
                  if(arg.empty())
                    args.push_back(ObjectPtr());
                  else if(ObjectPtr obj = Traintastic::instance->world->getObjectByPath(arg))
                    args.push_back(obj);
                  else
                    args.push_back(arg);
                }
                else
                  args.push_back(arg);
                break;
              }
              case ValueType::Object:
                args.push_back(m_handles.getItem(message.read<Handle>()));
                break;

              default:
                assert(false);
                break;
            }
          }

          try
          {
            AbstractMethod::Result result = method->call(args);

            if(message.isRequest())
            {
              auto response = Message::newResponse(message.command(), message.requestId());

              switch(resultType)
              {
                case ValueType::Boolean:
                  response->write(std::get<bool>(result));
                  break;

                case ValueType::Integer:
                case ValueType::Enum:
                case ValueType::Set:
                  response->write(std::get<int64_t>(result));
                  break;

                case ValueType::Float:
                  response->write(std::get<double>(result));
                  break;

                case ValueType::String:
                  response->write(std::get<std::string>(result));
                  break;

                case ValueType::Object:
                  writeObject(*response, std::get<ObjectPtr>(result));
                  break;

                case ValueType::Invalid:
                  assert(false);
                  break;
              }

              m_client->sendMessage(std::move(response));
              return true;
            }
          }
          catch(const std::exception& e)
          {
            if(message.isRequest())
            {
              m_client->sendMessage(Message::newErrorResponse(message.command(), message.requestId(), Message::ErrorCode::Failed));
              return true;
            }
          }
        }
      }
      break;
    }
    case Message::Command::GetTableModel:
    {
      if(ObjectPtr object = m_handles.getItem(message.read<Handle>()))
      {
        if(Table* table = dynamic_cast<Table*>(object.get()))
        {
          TableModelPtr model = table->getModel();
          assert(model);
          auto response = Message::newResponse(message.command(), message.requestId());
          writeTableModel(*response, model);
          m_client->sendMessage(std::move(response));

          model->columnHeadersChanged = [this](const TableModelPtr& tableModel)
            {
              auto event = Message::newEvent(Message::Command::TableModelColumnHeadersChanged);
              event->write(m_handles.getHandle(std::dynamic_pointer_cast<Object>(tableModel)));
              event->write(tableModel->columnCount());
              for(const auto& text : tableModel->columnHeaders())
                event->write(text);
              m_client->sendMessage(std::move(event));
            };

          model->rowCountChanged = [this](const TableModelPtr& tableModel)
            {
              auto event = Message::newEvent(Message::Command::TableModelRowCountChanged);
              event->write(m_handles.getHandle(std::dynamic_pointer_cast<Object>(tableModel)));
              event->write(tableModel->rowCount());
              m_client->sendMessage(std::move(event));
            };

          model->updateRegion = [this](const TableModelPtr& tableModel, const TableModel::Region& region)
            {
              auto event = Message::newEvent(Message::Command::TableModelUpdateRegion);
              event->write(m_handles.getHandle(std::dynamic_pointer_cast<Object>(tableModel)));
              event->write(region.columnMin);
              event->write(region.columnMax);
              event->write(region.rowMin);
              event->write(region.rowMax);

              for(uint32_t row = region.rowMin; row <= region.rowMax; row++)
                for(uint32_t column = region.columnMin; column <= region.columnMax; column++)
                  event->write(tableModel->getText(column, row));

              m_client->sendMessage(std::move(event));
            };

          return true;
        }
      }
      m_client->sendMessage(Message::newErrorResponse(message.command(), message.requestId(), Message::ErrorCode::ObjectNotTable));
      return true;
    }
    case Message::Command::ReleaseTableModel:
      m_handles.removeHandle(message.read<Handle>());
      break;

    case Message::Command::TableModelSetRegion:
    {
      TableModelPtr model = std::dynamic_pointer_cast<TableModel>(m_handles.getItem(message.read<Handle>()));
      if(model)
      {
        TableModel::Region region;
        message.read(region.columnMin);
        message.read(region.columnMax);
        message.read(region.rowMin);
        message.read(region.rowMax);
        model->setRegion(region);
      }
      break;
    }
    case Message::Command::InputMonitorGetInputInfo:
    {
      auto inputMonitor = std::dynamic_pointer_cast<InputMonitor>(m_handles.getItem(message.read<Handle>()));
      if(inputMonitor)
      {
        auto inputInfo = inputMonitor->getInputInfo();
        auto response = Message::newResponse(message.command(), message.requestId());
        response->write<uint32_t>(inputInfo.size());
        for(auto& info : inputInfo)
        {
          response->write(info.address);
          response->write(info.id);
          response->write(info.value);
        }
        m_client->sendMessage(std::move(response));
        return true;
      }
      break;
    }
    case Message::Command::OutputKeyboardGetOutputInfo:
    {
      auto outputKeyboard = std::dynamic_pointer_cast<OutputKeyboard>(m_handles.getItem(message.read<Handle>()));
      if(outputKeyboard)
      {
        auto outputInfo = outputKeyboard->getOutputInfo();
        auto response = Message::newResponse(message.command(), message.requestId());
        response->write<uint32_t>(outputInfo.size());
        for(auto& info : outputInfo)
        {
          response->write(info.address);
          response->write(info.id);
          response->write(info.value);
        }
        m_client->sendMessage(std::move(response));
        return true;
      }
      break;
    }
    case Message::Command::OutputKeyboardSetOutputValue:
    {
      auto outputKeyboard = std::dynamic_pointer_cast<OutputKeyboard>(m_handles.getItem(message.read<Handle>()));
      if(outputKeyboard)
      {
        const uint32_t address = message.read<uint32_t>();
        const bool value = message.read<bool>();
        outputKeyboard->setOutputValue(address, value);
      }
      break;
    }
    case Message::Command::BoardGetTileData:
    {
      auto board = std::dynamic_pointer_cast<Board>(m_handles.getItem(message.read<Handle>()));
      if(board)
      {
        auto response = Message::newResponse(message.command(), message.requestId());
        for(const auto& it : board->tileMap())
        {
          const Tile& tile = *(it.second);
          if(it.first != tile.location()) // only tiles at origin
            continue;
          response->write(tile.location());
          response->write(tile.data());
          assert(tile.data().isActive() == isActive(tile.data().id()));
          if(tile.data().isActive())
            writeObject(*response, it.second);
        }
        m_client->sendMessage(std::move(response));
        return true;
      }
      break;
    }
    case Message::Command::BoardGetTileInfo:
    {
      auto response = Message::newResponse(message.command(), message.requestId());
      const auto& info = Tiles::getInfo();
      response->write<uint32_t>(info.size());
      for(const auto& item : info)
      {
        response->writeBlock();
        response->write(item.classId);
        response->write(item.tileId);
        response->write(item.rotates);
        response->write(item.menu);
        response->writeBlockEnd();
      }
      m_client->sendMessage(std::move(response));
      return true;
    }
    case Message::Command::OutputMapGetItems:
    {
      if(auto outputMap = std::dynamic_pointer_cast<OutputMap>(m_handles.getItem(message.read<Handle>())))
      {
        auto response = Message::newResponse(message.command(), message.requestId());
        for(auto& item : outputMap->items())
          writeObject(*response, item);
        m_client->sendMessage(std::move(response));
        return true;
      }
      break;
    }
    case Message::Command::OutputMapGetOutputs:
    {
      if(auto outputMap = std::dynamic_pointer_cast<OutputMap>(m_handles.getItem(message.read<Handle>())))
      {
        auto response = Message::newResponse(message.command(), message.requestId());
        for(const auto& item : outputMap->outputs())
          writeObject(*response, item);
        m_client->sendMessage(std::move(response));
        return true;
      }
      break;
    }
    case Message::Command::ServerLog:
      if(message.read<bool>())
      {
        if(auto* logger = Log::getMemoryLogger())
        {
          m_memoryLoggerChanged = logger->changed.connect(std::bind(&Session::memoryLoggerChanged, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
          memoryLoggerChanged(*logger, logger->size(), 0);
        }
      }
      else // disable
        m_memoryLoggerChanged.disconnect();
      break;

    case Message::Command::ImportWorld:
      if(message.isRequest())
      {
        std::vector<std::byte> worldData;
        message.read(worldData);
        if(Traintastic::instance->importWorld(worldData))
          m_client->sendMessage(Message::newResponse(message.command(), message.requestId()));
        else
          m_client->sendMessage(Message::newErrorResponse(message.command(), message.requestId(), Message::ErrorCode::Failed));
      }
      break;

    case Message::Command::ExportWorld:
      if(message.isRequest())
      {
        if(Traintastic::instance->world)
        {
          std::vector<std::byte> worldData;
          if(Traintastic::instance->world->export_(worldData))
          {
            auto response = Message::newResponse(message.command(), message.requestId());
            response->write(worldData);
            m_client->sendMessage(std::move(response));
            return true;
          }
        }
        m_client->sendMessage(Message::newErrorResponse(message.command(), message.requestId(), Message::ErrorCode::Failed));
        return true;
      }
      break;

    default:
      break;
  }
  return false;
}

void Session::writeObject(Message& message, const ObjectPtr& object)
{
  message.writeBlock(); // object

  auto handle = m_handles.getHandle(object);
  if(handle == Handles::invalidHandle)
  {
    using namespace std::placeholders;

    handle = m_handles.addItem(object);

    m_objectSignals.emplace(handle, object->onDestroying.connect(std::bind(&Session::objectDestroying, this, std::placeholders::_1)));
    m_objectSignals.emplace(handle, object->propertyChanged.connect(std::bind(&Session::objectPropertyChanged, this, std::placeholders::_1)));
    m_objectSignals.emplace(handle, object->attributeChanged.connect(std::bind(&Session::objectAttributeChanged, this, std::placeholders::_1)));

    if(auto* inputMonitor = dynamic_cast<InputMonitor*>(object.get()))
    {
      m_objectSignals.emplace(handle, inputMonitor->inputIdChanged.connect(std::bind(&Session::inputMonitorInputIdChanged, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
      m_objectSignals.emplace(handle, inputMonitor->inputValueChanged.connect(std::bind(&Session::inputMonitorInputValueChanged, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
    }
    else if(auto* outputKeyboard = dynamic_cast<OutputKeyboard*>(object.get()))
    {
      m_objectSignals.emplace(handle, outputKeyboard->outputIdChanged.connect(std::bind(&Session::outputKeyboardOutputIdChanged, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
      m_objectSignals.emplace(handle, outputKeyboard->outputValueChanged.connect(std::bind(&Session::outputKeyboardOutputValueChanged, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
    }
    else if(auto* board = dynamic_cast<Board*>(object.get()))
    {
      m_objectSignals.emplace(handle, board->tileDataChanged.connect(std::bind(&Session::boardTileDataChanged, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
    }
    else if(auto* outputMap = dynamic_cast<OutputMap*>(object.get()))
    {
      m_objectSignals.emplace(handle, outputMap->outputsChanged.connect(std::bind(&Session::outputMapOutputsChanged, this, std::placeholders::_1)));
    }

    bool hasPublicEvents = false;

    message.write(handle);
    message.write(object->getClassId());

    message.writeBlock(); // items
    const InterfaceItems& interfaceItems = object->interfaceItems();
    for(const auto& name : interfaceItems.names())
    {
      InterfaceItem& item = interfaceItems[name];

      if(item.isInternal())
        continue;

      message.writeBlock(); // item
      message.write(name);

      if(BaseProperty* baseProperty = dynamic_cast<BaseProperty*>(&item))
      {
        AbstractProperty* property = nullptr;
        AbstractUnitProperty* unitProperty = nullptr;
        AbstractVectorProperty* vectorProperty = nullptr;

        if((property = dynamic_cast<AbstractProperty*>(baseProperty)))
        {
          if((unitProperty = dynamic_cast<AbstractUnitProperty*>(property)))
            message.write(InterfaceItemType::UnitProperty);
          else
            message.write(InterfaceItemType::Property);
        }
        else if((vectorProperty = dynamic_cast<AbstractVectorProperty*>(baseProperty)))
          message.write(InterfaceItemType::VectorProperty);
        else
        {
          assert(false);
          continue;
        }

        message.write(baseProperty->flags());
        message.write(baseProperty->type());

        if(baseProperty->type() == ValueType::Enum)
          message.write(baseProperty->enumName());
        else if(baseProperty->type() == ValueType::Set)
          message.write(baseProperty->setName());

        if(property)
        {
          writePropertyValue(message, *property);

          if(unitProperty)
          {
            message.write(unitProperty->unitName());
            message.write(unitProperty->unitValue());
          }
        }
        else if(vectorProperty)
          writeVectorPropertyValue(message, *vectorProperty);
        else
          assert(false);
      }
      else if(const AbstractMethod* method = dynamic_cast<const AbstractMethod*>(&item))
      {
        message.write(InterfaceItemType::Method);
        message.write(method->resultTypeInfo().type);
        message.write(static_cast<uint8_t>(method->argumentTypeInfo().size()));
        for(const auto& info : method->argumentTypeInfo())
          message.write(info.type);
      }
      else if(const auto* event = dynamic_cast<const AbstractEvent*>(&item))
      {
        hasPublicEvents = true;

        message.write(InterfaceItemType::Event);
        message.write(static_cast<uint8_t>(event->argumentTypeInfo().size()));
        for(const auto& typeInfo : event->argumentTypeInfo())
          writeTypeInfo(message, typeInfo);
      }
      else
        assert(false);

      message.writeBlock(); // attributes

      for(const auto& it : item.attributes())
      {
        const AbstractAttribute& attribute = *it.second;
        message.writeBlock(); // attribute
        writeAttribute(message, attribute);
        message.writeBlockEnd(); // end attribute
      }

      message.writeBlockEnd(); // end attributes
      message.writeBlockEnd(); // end item
    }
    message.writeBlockEnd(); // end items

    if(hasPublicEvents)
      m_objectSignals.emplace(handle, object->onEventFired.connect(std::bind(&Session::objectEventFired, this, std::placeholders::_1, std::placeholders::_2)));
  }
  else
    message.write(handle);

  message.writeBlockEnd(); // end object
}

void Session::writeTableModel(Message& message, const TableModelPtr& model)
{
  message.writeBlock(); // model
  assert(m_handles.getHandle(std::dynamic_pointer_cast<Object>(model)) == Handles::invalidHandle);
  auto handle = m_handles.addItem(std::dynamic_pointer_cast<Object>(model));
  message.write(handle);
  message.write(model->getClassId());
  message.write(model->columnCount());
  for(const auto& text : model->columnHeaders())
    message.write(text);
  message.write(model->rowCount());
  message.writeBlockEnd(); // end model
}

void Session::memoryLoggerChanged(const MemoryLogger& logger, const uint32_t added, const uint32_t removed)
{
  auto event = Message::newEvent(Message::Command::ServerLog);
  event->write(removed);
  event->write(added);

  const uint32_t size = logger.size();
  for(uint32_t i = size - added; i < size; i++)
  {
    const auto& log = logger[i];
    event->write((std::chrono::duration_cast<std::chrono::microseconds>(log.time.time_since_epoch())).count());
    event->write(log.objectId);
    event->write(log.message);
    const size_t argc = log.args ? std::min<size_t>(log.args->size(), std::numeric_limits<uint8_t>::max()) : 0;
    event->write(static_cast<uint8_t>(argc));
    for(size_t j = 0; j < argc; j++)
      event->write(log.args->at(j));
  }

  m_client->sendMessage(std::move(event));
}

void Session::objectDestroying(Object& object)
{
  const auto handle = m_handles.getHandle(object.shared_from_this());
  m_handles.removeHandle(handle);
  m_objectSignals.erase(handle);

  auto event = Message::newEvent(Message::Command::ObjectDestroyed, sizeof(Handle));
  event->write(handle);
  m_client->sendMessage(std::move(event));
}

void Session::objectPropertyChanged(BaseProperty& baseProperty)
{
  if(baseProperty.isInternal())
    return;

  auto event = Message::newEvent(Message::Command::ObjectPropertyChanged);
  event->write(m_handles.getHandle(baseProperty.object().shared_from_this()));
  event->write(baseProperty.name());
  event->write(baseProperty.type());
  if(AbstractProperty* property = dynamic_cast<AbstractProperty*>(&baseProperty))
  {
    writePropertyValue(*event, *property);

    if(AbstractUnitProperty* unitProperty = dynamic_cast<AbstractUnitProperty*>(property))
      event->write(unitProperty->unitValue());
  }
  else if(AbstractVectorProperty* vectorProperty = dynamic_cast<AbstractVectorProperty*>(&baseProperty))
    writeVectorPropertyValue(*event, *vectorProperty);
  else
    assert(false);

  m_client->sendMessage(std::move(event));
}

void Session::writePropertyValue(Message& message , const AbstractProperty& property)
{
  switch(property.type())
  {
    case ValueType::Boolean:
      message.write(property.toBool());
      break;

    case ValueType::Enum:
    case ValueType::Integer:
    case ValueType::Set:
      message.write(property.toInt64());
      break;

    case ValueType::Float:
      message.write(property.toDouble());
      break;

    case ValueType::String:
      message.write(property.toString());
      break;

    case ValueType::Object:
      if(ObjectPtr obj = property.toObject())
        message.write(obj->getObjectId());
      else
        message.write<std::string_view>("");
      break;

    case ValueType::Invalid:
      assert(false);
      break;
  }
}

void Session::writeVectorPropertyValue(Message& message , const AbstractVectorProperty& vectorProperty)
{
  const size_t size = vectorProperty.size();
  message.write(static_cast<uint32_t>(size));

  switch(vectorProperty.type())
  {
    case ValueType::Boolean:
      for(size_t i = 0; i < size; i++)
        message.write(vectorProperty.getBool(i));
      break;

    case ValueType::Enum:
    case ValueType::Integer:
    case ValueType::Set:
      for(size_t i = 0; i < size; i++)
        message.write(vectorProperty.getInt64(i));
      break;

    case ValueType::Float:
      for(size_t i = 0; i < size; i++)
        message.write(vectorProperty.getDouble(i));
      break;

    case ValueType::String:
      for(size_t i = 0; i < size; i++)
        message.write(vectorProperty.getString(i));
      break;

    case ValueType::Object:
      for(size_t i = 0; i < size; i++)
        if(ObjectPtr obj = vectorProperty.getObject(i))
          message.write(obj->getObjectId());
        else
          message.write<std::string>("");
      break;

    case ValueType::Invalid:
      assert(false);
      break;
  }
}

void Session::objectAttributeChanged(AbstractAttribute& attribute)
{
  auto event = Message::newEvent(Message::Command::ObjectAttributeChanged);
  event->write(m_handles.getHandle(attribute.item().object().shared_from_this()));
  event->write(attribute.item().name());
  writeAttribute(*event, attribute);
  m_client->sendMessage(std::move(event));
}

void Session::objectEventFired(const AbstractEvent& event, const Arguments& arguments)
{
  auto message = Message::newEvent(Message::Command::ObjectEventFired);
  message->write(m_handles.getHandle(event.object().shared_from_this()));
  message->write(event.name());
  message->write(static_cast<uint32_t>(arguments.size()));
  size_t i = 0;
  for(const auto& typeInfo : event.argumentTypeInfo())
  {
    switch(typeInfo.type)
    {
      case ValueType::Boolean:
        message->write(std::get<bool>(arguments[i]));
        break;

      case ValueType::Enum:
      case ValueType::Integer:
      case ValueType::Set:
        message->write(std::get<int64_t>(arguments[i]));
        break;

      case ValueType::Float:
        message->write(std::get<double>(arguments[i]));
        break;

      case ValueType::String:
        message->write(std::get<std::string>(arguments[i]));
        break;

      case ValueType::Object:
        if(ObjectPtr obj = std::get<ObjectPtr>(arguments[i]))
          message->write(obj->getObjectId());
        else
          message->write(std::string_view{});
        break;

      case ValueType::Invalid:
        assert(false);
        return;
    }
    i++;
  }
  m_client->sendMessage(std::move(message));
}

void Session::writeAttribute(Message& message , const AbstractAttribute& attribute)
{
  message.write(attribute.name());
  message.write(attribute.type());
  if(const AbstractValueAttribute* valueAttribute = dynamic_cast<const AbstractValueAttribute*>(&attribute))
  {
    message.write(AttributeType::Value);
    switch(attribute.type())
    {
      case ValueType::Boolean:
        message.write(valueAttribute->toBool());
        break;

      case ValueType::Enum:
      case ValueType::Integer:
        message.write(valueAttribute->toInt64());
        break;

      case ValueType::Float:
        message.write(valueAttribute->toDouble());
        break;

      case ValueType::String:
        message.write(valueAttribute->toString());
        break;

      default:
        assert(false);
        break;
    }
  }
  else if(const AbstractValuesAttribute* valuesAttributes = dynamic_cast<const AbstractValuesAttribute*>(&attribute))
  {
    const uint32_t length = valuesAttributes->length();
    message.write(AttributeType::Values);
    message.write(length);
    switch(attribute.type())
    {
      case ValueType::Boolean:
        for(uint32_t i = 0; i < length; i++)
          message.write(valuesAttributes->getBool(i));
        break;

      case ValueType::Enum:
      case ValueType::Integer:
        for(uint32_t i = 0; i < length; i++)
          message.write(valuesAttributes->getInt64(i));
        break;

      case ValueType::Float:
        for(uint32_t i = 0; i < length; i++)
          message.write(valuesAttributes->getDouble(i));
        break;

      case ValueType::String:
        for(uint32_t i = 0; i < length; i++)
          message.write(valuesAttributes->getString(i));
        break;

      default:
        assert(false);
        break;
    }
  }
  else
    assert(false);
}

void Session::writeTypeInfo(Message& message, const TypeInfo& typeInfo)
{
  assert((typeInfo.type == ValueType::Enum) != typeInfo.enumName.empty());
  assert((typeInfo.type == ValueType::Set) != typeInfo.setName.empty());

  message.write(typeInfo.type);
  if(typeInfo.type == ValueType::Enum)
    message.write(typeInfo.enumName);
  else if(typeInfo.type == ValueType::Set)
    message.write(typeInfo.setName);
}

void Session::inputMonitorInputIdChanged(InputMonitor& inputMonitor, const uint32_t address, std::string_view id)
{
  auto event = Message::newEvent(Message::Command::InputMonitorInputIdChanged);
  event->write(m_handles.getHandle(inputMonitor.shared_from_this()));
  event->write(address);
  event->write(id);
  m_client->sendMessage(std::move(event));
}

void Session::inputMonitorInputValueChanged(InputMonitor& inputMonitor, const uint32_t address, const TriState value)
{
  auto event = Message::newEvent(Message::Command::InputMonitorInputValueChanged);
  event->write(m_handles.getHandle(inputMonitor.shared_from_this()));
  event->write(address);
  event->write(value);
  m_client->sendMessage(std::move(event));
}

void Session::outputKeyboardOutputIdChanged(OutputKeyboard& outputKeyboard, const uint32_t address, std::string_view id)
{
  auto event = Message::newEvent(Message::Command::OutputKeyboardOutputIdChanged);
  event->write(m_handles.getHandle(outputKeyboard.shared_from_this()));
  event->write(address);
  event->write(id);
  m_client->sendMessage(std::move(event));
}

void Session::outputKeyboardOutputValueChanged(OutputKeyboard& outputKeyboard, const uint32_t address, const TriState value)
{
  auto event = Message::newEvent(Message::Command::OutputKeyboardOutputValueChanged);
  event->write(m_handles.getHandle(outputKeyboard.shared_from_this()));
  event->write(address);
  event->write(value);
  m_client->sendMessage(std::move(event));
}

void Session::boardTileDataChanged(Board& board, const TileLocation& location, const TileData& data)
{
  auto event = Message::newEvent(Message::Command::BoardTileDataChanged);
  event->write(m_handles.getHandle(board.shared_from_this()));
  event->write(location);
  event->write(data);
  assert(data.isActive() == isActive(data.id()));
  if(data.isActive())
  {
    auto tile = board.getTile(location);
    assert(tile);
    writeObject(*event, tile);
  }
  m_client->sendMessage(std::move(event));
}

void Session::outputMapOutputsChanged(OutputMap& outputMap)
{
  auto event = Message::newEvent(Message::Command::OutputMapOutputsChanged);
  event->write(m_handles.getHandle(outputMap.shared_from_this()));
  for(const auto& item : outputMap.outputs())
    writeObject(*event, item);
  m_client->sendMessage(std::move(event));
}
