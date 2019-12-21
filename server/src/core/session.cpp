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
#include <boost/uuid/random_generator.hpp>
#include "traintastic.hpp"
#include "client.hpp"
#include "abstractproperty.hpp"
#include "abstractattribute.hpp"
#include <enum/interfaceitemtype.hpp>
#include "tablemodel.hpp"
#include "world.hpp"
#include "idobject.hpp"




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
    case Message::Command::GetObject:
    case Message::Command::GetTableModel:
    {
      std::string id;
      message.read(id);

      ObjectPtr obj = Traintastic::instance->world->getObject(id);
      if(!obj)
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
        switch(message.command())
        {
          case Message::Command::GetObject:
          {
            Traintastic::instance->console->debug(m_client->m_id, "GetObject: " + id);
            auto response = Message::newResponse(message.command(), message.requestId());
            writeObject(*response, obj);
            m_client->sendMessage(std::move(response));
            break;
          }
          case Message::Command::GetTableModel:
          {
            if(Table* table = dynamic_cast<Table*>(obj.get()))
            {
              TableModelPtr model = table->getModel();
              assert(model);
              Traintastic::instance->console->debug(m_client->m_id, "GetTableModel: " + id);
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
  /*
                void TableModel::sendRegion(uint32_t columnMin, uint32_t columnMax, uint32_t rowMin, uint32_t rowMax)
{
  auto event = Message::newEvent(Message::Command::TableModelUpdateRegion);
  event->write(m_handle);

  for(uint32_t row = rowMin; row <= rowMax; row++)
    for(uint32_t column = columnMin; column <= columnMax; column++)
    {


    }
}
*/

            }
            else
              m_client->sendMessage(Message::newErrorResponse(message.command(), message.requestId(), Message::ErrorCode::ObjectNotTable));
            break;
          }
          default:
            assert(false);
            return false;
        }
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
   // TODO:         assert(!obj || dynamic_cast<IdObject*>(obj.get()));
            if(IdObject* idObj = dynamic_cast<IdObject*>(obj.get()))
              message.write<std::string>(idObj->id);
            else
              message.write<std::string>("");
            break;
          }
          default:
            assert(false);
            break;
        }
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
      event->write(property.toInt64());
      break;

    case ValueType::Float:
      event->write(property.toDouble());
      break;

    case ValueType::String:
      event->write(property.toString());
      break;
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
