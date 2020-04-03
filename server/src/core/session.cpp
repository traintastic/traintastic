/**
 * server/src/core/session.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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
#include "traintastic.hpp"
#include "client.hpp"
#include "abstractproperty.hpp"
#include "abstractattribute.hpp"
#include <enum/interfaceitemtype.hpp>
#include "tablemodel.hpp"
#include "world.hpp"
#include "idobject.hpp"
#include "subobject.hpp"




#include "settings.hpp"

#include "../hardware/commandstation/commandstationlist.hpp"
#include "../hardware/decoder/decoderlist.hpp"

#include <iostream>

Session::Session(const std::shared_ptr<Client>& client) :
  m_client{client},
  m_uuid{boost::uuids::random_generator()()}
{
}

bool Session::processMessage(const Message& message)
{
  switch(message.command())
  {
    case Message::Command::CreateObject:
    {
      std::string classId;
      std::string id;
      message.read(classId);
      message.read(id);

      Traintastic::instance->console->debug(m_client->m_id, "CreateObject: " + classId);

      ObjectPtr obj = Traintastic::instance->world->createObject(classId, id);
      if(obj)
      {
        auto response = Message::newResponse(message.command(), message.requestId());
        writeObject(*response, obj);
        m_client->sendMessage(std::move(response));
      }
      else
        m_client->sendMessage(Message::newErrorResponse(message.command(), message.requestId(), Message::ErrorCode::UnknownClassId));

      return true;
    }
    case Message::Command::GetObject:
    {
      std::string id;
      message.read(id);

      std::vector<std::string> ids;
      boost::split(ids, id, [](char c){ return c == '.'; });
      auto it = ids.cbegin();

      ObjectPtr obj = Traintastic::instance->world->getObject(*it);
      if(obj)
      {
        it++;
        while(obj && it != ids.cend())
        {
          AbstractProperty* property = obj->getProperty(*it);
          if(property && property->type() == ValueType::Object)
            obj = property->toObject();
          else
            obj = nullptr;
          it++;
        }
      }
      else
      {
        if(id == Traintastic::id)
          obj = Traintastic::instance;
        else if(id == "console")
          obj = Traintastic::instance->console.toObject();
        else if(id == "settings")
          obj = Traintastic::instance->settings.toObject();
      }

      if(obj)
      {
        Traintastic::instance->console->debug(m_client->m_id, "GetObject: " + id);
        auto response = Message::newResponse(message.command(), message.requestId());
        writeObject(*response, obj);
        m_client->sendMessage(std::move(response));
      }
      else
      {
        Traintastic::instance->console->debug(m_client->m_id, "UnknownObject: " + id);
        m_client->sendMessage(Message::newErrorResponse(message.command(), message.requestId(), Message::ErrorCode::UnknownObject));
      }
      return true;
    }
    case Message::Command::ReleaseObject:
    {
      Handle handle = message.read<Handle>();
      Traintastic::instance->console->debug(m_client->m_id, "ReleaseObject: " + std::to_string(handle));
      m_handles.removeHandle(handle);
      m_propertyChanged.erase(handle);
      m_attributeChanged.erase(handle);
      break;
    }
    case Message::Command::ObjectSetProperty:
    {
      if(ObjectPtr object = m_handles.getItem(message.read<Handle>()))
      {
        if(AbstractProperty* property = object->getProperty(message.read<std::string>()))
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
            }
          }
          catch(const std::exception&)
          {
            // set property failed, send changed event with current value:
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
        if(AbstractMethod* method = object->getMethod(message.read<std::string>()))
        {
          const ValueType resultType = message.read<ValueType>();
          const uint8_t argumentCount = message.read<uint8_t>();

          std::vector<AbstractMethod::Argument> args;
          for(uint8_t i = 0; i < argumentCount; i++)
          {
            switch(message.read<ValueType>())
            {
              case ValueType::Boolean:
                args.push_back(message.read<bool>());
                break;

              case ValueType::Integer:
                args.push_back(message.read<int64_t>());
                break;

              case ValueType::Float:
                args.push_back(message.read<double>());
                break;

              case ValueType::String:
                args.push_back(message.read<std::string>());
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

                /*default:
                  assert(false);
                  break;*/
              }

              m_client->sendMessage(std::move(response));
              return true;
            }
          }
          catch(const std::exception& e)
          {
            std::cerr << e.what() << '\n';
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
          //Traintastic::instance->console->debug(m_client->m_id, "GetTableModel: " + id);
          auto response = Message::newResponse(message.command(), message.requestId());
          writeTableModel(*response, model);
          m_client->sendMessage(std::move(response));

          model->columnHeadersChanged = [this](const TableModelPtr& model)
            {
              auto event = Message::newEvent(Message::Command::TableModelColumnHeadersChanged);
              event->write(m_handles.getHandle(std::dynamic_pointer_cast<Object>(model)));
              event->write(model->columnCount());
              for(const std::string& text : model->columnHeaders())
                event->write(text);
              m_client->sendMessage(std::move(event));
            };

          model->rowCountChanged = [this](const TableModelPtr& model)
            {
              auto event = Message::newEvent(Message::Command::TableModelRowCountChanged);
              event->write(m_handles.getHandle(std::dynamic_pointer_cast<Object>(model)));
              event->write(model->rowCount());
              m_client->sendMessage(std::move(event));
            };

          model->updateRegion = [this](const TableModelPtr& model, const TableModel::Region& region)
            {
              std::cout << "updateRegion " << region.columnMin << " " << region.columnMax << " " << region.rowMin << " " << region.rowMax << std::endl;

              auto event = Message::newEvent(Message::Command::TableModelUpdateRegion);
              event->write(m_handles.getHandle(std::dynamic_pointer_cast<Object>(model)));
              event->write(region.columnMin);
              event->write(region.columnMax);
              event->write(region.rowMin);
              event->write(region.rowMax);

              for(uint32_t row = region.rowMin; row <= region.rowMax; row++)
                for(uint32_t column = region.columnMin; column <= region.columnMax; column++)
                  event->write(model->getText(column, row));

              m_client->sendMessage(std::move(event));
            };

          return true;
        }
      }
      m_client->sendMessage(Message::newErrorResponse(message.command(), message.requestId(), Message::ErrorCode::ObjectNotTable));
      return true;
    }
    case Message::Command::ReleaseTableModel:
    {
      Handle handle = message.read<Handle>();
      Traintastic::instance->console->debug(m_client->m_id, "ReleaseTableModel: " + std::to_string(handle));
      m_handles.removeHandle(handle);
      break;
    }
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
        //std::cout << "TableModelSetRegion " << region.columnMin << " " << region.columnMax << " " << region.rowMin << " " << region.rowMax << std::endl;
        model->setRegion(region);
      }
      break;
    }
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

    m_propertyChanged.emplace(handle, object->propertyChanged.connect(std::bind(&Session::objectPropertyChanged, this, std::placeholders::_1)));
    m_attributeChanged.emplace(handle, object->attributeChanged.connect(std::bind(&Session::objectAttributeChanged, this, std::placeholders::_1)));

    message.write(handle);
    message.write(object->getClassId());

    message.writeBlock(); // items
    const InterfaceItems& interfaceItems = object->interfaceItems();
    for(const std::string& name : interfaceItems.names())
    {
      InterfaceItem& item = interfaceItems[name];

      // TODO: if(item. internal)

      message.writeBlock(); // item
      message.write(name);

      if(AbstractProperty* property = dynamic_cast<AbstractProperty*>(&item))
      {
        message.write(InterfaceItemType::Property);
        message.write(property->flags());
        message.write(property->type());
        switch(property->type())
        {
          case ValueType::Boolean:
            message.write(property->toBool());
            break;

          case ValueType::Enum:
            message.write(property->toInt64());
            message.write(property->enumName());
            break;

          case ValueType::Integer:
            message.write(property->toInt64());
            break;

          case ValueType::Float:
            message.write(property->toDouble());
            break;

          case ValueType::String:
            message.write(property->toString());
            break;

          case ValueType::Object:
          {
            ObjectPtr obj = property->toObject();
            if(obj)
            {
              if(IdObject* idObj = dynamic_cast<IdObject*>(obj.get()))
                message.write<std::string>(idObj->id);
              else if(SubObject* subObj = dynamic_cast<SubObject*>(obj.get()))
                message.write<std::string>(subObj->id());
              else if(dynamic_cast<World*>(obj.get()))
                message.write<std::string_view>(World::classId);
              else
              {
                //assert(false);
                message.write<std::string>("");
              }
            }
            else
              message.write<std::string>("");
            break;
          }
          case ValueType::Set:
            message.write(property->toInt64());
            message.write(property->setName());
            break;

          default:
            assert(false);
            break;
        }
      }
      else if(const AbstractMethod* method = dynamic_cast<const AbstractMethod*>(&item))
      {
        message.write(InterfaceItemType::Method);
        message.write(method->resultType());
        message.write(static_cast<uint8_t>(method->argumentCount()));
        for(auto type : method->argumentTypes())
          message.write(type);
      }

      message.writeBlock(); // attributes

      for(const auto& it : item.attributes())
      {
        const AbstractAttribute& attribute = *it.second;
        message.writeBlock(); // attribute
        message.write(attribute.name());
        message.write(attribute.type());
        switch(attribute.type())
        {
          case ValueType::Boolean:
            message.write(attribute.toBool());
            break;

          case ValueType::Enum:
          case ValueType::Integer:
            message.write(attribute.toInt64());
            break;

          case ValueType::Float:
            message.write(attribute.toDouble());
            break;

          case ValueType::String:
            message.write(attribute.toString());
            break;

          default:
            assert(false);
            break;
        }
        message.writeBlockEnd(); // end attribute
      }

      message.writeBlockEnd(); // end attributes
      message.writeBlockEnd(); // end item
    }
    message.writeBlockEnd(); // end items
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
  for(const std::string& text : model->columnHeaders())
    message.write(text);
  message.write(model->rowCount());
  message.writeBlockEnd(); // end model
}

void Session::objectPropertyChanged(AbstractProperty& property)
{
  std::cout << "objectPropertyChanged " << property.name() << std::endl;

  auto event = Message::newEvent(Message::Command::ObjectPropertyChanged);
  event->write(m_handles.getHandle(property.object().shared_from_this()));
  event->write(property.name());
  event->write(property.type());
  switch(property.type())
  {
    case ValueType::Boolean:
      event->write(property.toBool());
      break;

    case ValueType::Enum:
    case ValueType::Integer:
    case ValueType::Set:
      event->write(property.toInt64());
      break;

    case ValueType::Float:
      event->write(property.toDouble());
      break;

    case ValueType::String:
      event->write(property.toString());
      break;

    case ValueType::Object:
    {
      ObjectPtr obj = property.toObject();
      if(obj)
      {
        if(IdObject* idObj = dynamic_cast<IdObject*>(obj.get()))
          event->write<std::string>(idObj->id);
        else if(SubObject* subObj = dynamic_cast<SubObject*>(obj.get()))
          event->write<std::string>(subObj->id());
        else if(dynamic_cast<World*>(obj.get()))
          event->write<std::string_view>(World::classId);
        else
        {
          assert(false);
          event->write<std::string_view>("");
        }
      }
      else
        event->write<std::string_view>("");
      break;
    }
  }
  m_client->sendMessage(std::move(event));
}

void Session::objectAttributeChanged(AbstractAttribute& attribute)
{
  std::cout << "objectAttributeChanged " << attribute.item().name() << "." << (int)attribute.name() << std::endl;

  auto event = Message::newEvent(Message::Command::ObjectAttributeChanged);
  event->write(m_handles.getHandle(attribute.item().object().shared_from_this()));
  event->write(attribute.item().name());
  event->write(attribute.name());
  event->write(attribute.type());
  switch(attribute.type())
  {
    case ValueType::Boolean:
      event->write(attribute.toBool());
      break;

    case ValueType::Enum:
    case ValueType::Integer:
      //event->write(attribute.toInt64());
      break;

    case ValueType::Float:
      //event->write(attribute.toDouble());
      break;

    case ValueType::String:
      //event->write(attribute.toString());
      break;
  }
  m_client->sendMessage(std::move(event));
}
