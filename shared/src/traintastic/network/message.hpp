/**
 * shared/src/message.hpp
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_NETWORK_MESSAGE_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_NETWORK_MESSAGE_HPP

#include <vector>
#include <string>
#include <atomic>
#include <memory>
#include <stack>
#include <cstdint>
#include <cstring>
#include <cassert>
#ifdef QT_CORE_LIB
  #include <QByteArray>
  #include <QUuid>
#endif

#include "../enum/logmessage.hpp"

class Message
{
  public:
    enum class Command : uint8_t
    {
      Invalid = 0,
      Ping = 1,
      Login = 2,
      NewSession = 3,
      ServerLog = 5,
      ImportWorld = 9,
      ExportWorld = 10,
      CreateObject = 11,
      GetObject = 14,
      ReleaseObject = 15,
      ObjectSetProperty = 16,
      ObjectSetUnitPropertyUnit = 26,
      ObjectSetObjectPropertyById = 27,
      ObjectPropertyChanged = 17,
      ObjectAttributeChanged = 18,
      ObjectCallMethod = 25,
      ObjectDestroyed = 28,
      ObjectEventFired = 42,

      GetTableModel = 19,
      ReleaseTableModel = 20,
      TableModelColumnHeadersChanged = 21,
      TableModelRowCountChanged = 22,
      TableModelSetRegion = 23,
      TableModelUpdateRegion = 24,

      InputMonitorGetInputInfo = 30,
      InputMonitorInputIdChanged = 31,
      InputMonitorInputValueChanged = 32,

      OutputKeyboardGetOutputInfo = 33,
      OutputKeyboardSetOutputValue = 34,
      OutputKeyboardOutputIdChanged = 35,
      OutputKeyboardOutputValueChanged = 36,

      BoardGetTileData = 37,
      BoardTileDataChanged = 38,
      BoardGetTileInfo = 43,

      OutputMapGetItems = 39,
      OutputMapGetOutputs = 40,
      OutputMapOutputsChanged = 41,

      ObjectGetObjectPropertyObject = 44,
      ObjectGetObjectVectorPropertyObject = 45,

      Discover = 255,
    };

    enum class Type : uint8_t
    {
      Request = 1,
      Response = 2,
      Event = 3,
    };
#ifdef _MSC_VER
  #pragma pack(push, 1)
#endif
    struct Header
    {
      Command command;
      struct Flags
      {
        uint8_t reserved : 5; // must be zero
        uint8_t error : 1;
        uint8_t type : 2;
      } flags;
      uint16_t requestId;
      uint32_t dataSize;
    }
#ifdef __GNUC__
    __attribute__((packed))
#endif
    ;
#ifdef _MSC_VER
  #pragma pack(pop)
#endif
    static_assert(sizeof(Header) == 8);

  private:
    inline static std::atomic<uint16_t> s_requestId{0};

  protected:
    std::vector<uint8_t> m_data;
    mutable uint32_t m_readPosition;
    mutable std::stack<uint32_t> m_block;

    const Header& header() const { return *reinterpret_cast<const Header*>(m_data.data()); }
    Header& header() { return *reinterpret_cast<Header*>(m_data.data()); }

    void updateDataSize()
    {
      header().dataSize = static_cast<uint32_t>(m_data.size() - sizeof(Header));
    }

  public:
    using Length = uint32_t;

    static std::unique_ptr<Message> newRequest(Command command, size_t capacity = 0)
    {
      return std::make_unique<Message>(command, Type::Request, ++s_requestId, capacity);
    }

    static std::unique_ptr<Message> newResponse(Command command, uint16_t requestId, size_t capacity = 0)
    {
      return std::make_unique<Message>(command, Type::Response, requestId, capacity);
    }

    static std::unique_ptr<Message> newErrorResponse(Command command, uint16_t requestId, LogMessage code)
    {
      std::unique_ptr<Message> message = std::make_unique<Message>(command, Type::Response, requestId);
      message->header().flags.error = 1;
      message->write(code);
      message->write<Length>(0); // no args
      return message;
    }

    static std::unique_ptr<Message> newErrorResponse(Command command, uint16_t requestId, LogMessage code, std::string_view arg)
    {
      std::unique_ptr<Message> message = std::make_unique<Message>(command, Type::Response, requestId);
      message->header().flags.error = 1;
      message->write(code);
      message->write<Length>(1);
      message->write(arg);
      return message;
    }

    static std::unique_ptr<Message> newErrorResponse(Command command, uint16_t requestId, LogMessage code, const std::vector<std::string>& args)
    {
      std::unique_ptr<Message> message = std::make_unique<Message>(command, Type::Response, requestId);
      message->header().flags.error = 1;
      message->write(code);
      message->write<Length>(args.size());
      for(const auto& arg : args)
      {
        message->write(arg);
      }
      return message;
    }

    static std::unique_ptr<Message> newEvent(Command command, size_t capacity = 0)
    {
      return std::make_unique<Message>(command, Type::Event, 0, capacity);
    }

    Message(const Header& _header) :
      m_data(sizeof(Header) + _header.dataSize, 0),
      m_readPosition{0}
    {
      header() = _header;
    }

    Message(Command command, Type type, uint16_t requestId, size_t capacity = 0) :
      m_data(sizeof(Header), 0),
      m_readPosition{0}
    {
      header().command = command;
      header().flags.reserved = 0;
      header().flags.error = 0;
      header().flags.type = static_cast<uint8_t>(type);
      header().requestId = requestId;
      header().dataSize = 0;

      if(capacity > 0)
        m_data.reserve(sizeof(Header) + capacity);
    }

    Message(uint32_t size) :
      m_data(size, 0),
      m_readPosition{0}
    {
    }

    ~Message()
    {
    }

    inline Command command() const { return header().command; }
    inline Type type() const { return static_cast<Type>(header().flags.type); }
    inline bool isRequest() const { return type() == Type::Request; }
    inline bool isResponse() const  { return type() == Type::Response; }
    inline bool isEvent() const { return type() == Type::Event; }
    inline bool isError() const { return header().flags.error; }
    inline uint16_t requestId() const { return header().requestId; }

    const void* operator*() const { return m_data.data(); }
    void* operator*() { return m_data.data(); }
    size_t size() const { return m_data.size(); }

    const void* data() const { return m_data.data() + sizeof(Header); }
    void* data() { return m_data.data() + sizeof(Header); }
    uint32_t dataSize() const { return header().dataSize; }

    const void* current() const { return m_data.data() + sizeof(Header) + m_readPosition; }

    template<typename T>
    void read(T& value) const
    {
      const uint8_t* p = m_data.data() + sizeof(Header) + m_readPosition;

#ifdef QT_CORE_LIB
      if constexpr(std::is_same_v<T,QUuid>)
      {
        value = QUuid::fromRfc4122(QByteArray(reinterpret_cast<const char*>(p), sizeof(QUuid)));
        m_readPosition += sizeof(QUuid);
      }
      else if constexpr(std::is_same_v<T,QByteArray>)
      {
        const Length length = read<Length>();
        value.resize(length);
        memcpy(value.data(), p + sizeof(Length), length);
        m_readPosition += length;
      }
      else
#endif
      if constexpr(std::is_same_v<T,bool>)
      {
        value = *p;
        m_readPosition++;
      }
      else if constexpr(std::is_same_v<T,std::string>)
      {
        const Length length = read<Length>();
        value.resize(length);
        memcpy(value.data(), p + sizeof(Length), length);
        m_readPosition += length;
      }
      else if constexpr(std::is_trivially_copyable_v<T>)
      {
        memcpy(&value, p, sizeof(value));
        m_readPosition += sizeof(value);
      }
      else
        static_assert(sizeof(T) != sizeof(T));
    }

    template<typename T>
    void read(std::vector<T>& value) const
    {
      const Length length = read<Length>();

      if constexpr(std::is_trivially_copyable_v<T>)
      {
        value.resize(length);
        memcpy(value.data(), m_data.data() + sizeof(Header) + m_readPosition, length * sizeof(T));
        m_readPosition += length * sizeof(T);
      }
      else
        static_assert(sizeof(T) != sizeof(T));
    }

    template<typename T>
    T read() const
    {
      T value;
      read(value);
      return value;
    }

    bool endOfBlock() const
    {
      assert(!m_block.empty());
      return m_readPosition == m_block.top();
    }

    bool endOfMessage() const
    {
      return m_readPosition == dataSize();
    }

    uint32_t readBlock() const
    {
      const uint32_t blockSize = read<uint32_t>();
      m_block.push(m_readPosition + blockSize);
      return blockSize;
    }

    void readBlockEnd() const
    {
      assert(!m_block.empty());
      assert(m_readPosition <= m_block.top());
      m_readPosition = m_block.top();
      m_block.pop();
    }

    template<typename T>
    void write(const T& value)
    {
      const size_t oldSize = m_data.size();

#ifdef QT_CORE_LIB
      if constexpr(std::is_same_v<T,QByteArray>)
      {
        m_data.resize(oldSize + sizeof(Length) + value.size());
        const Length length = value.size();
        memcpy(m_data.data() + oldSize, &length, sizeof(length));
        memcpy(m_data.data() + oldSize + sizeof(Length), value.data(), value.size());
      }
      else
#endif
      if constexpr(std::is_same_v<T,std::string_view> || std::is_same_v<T,std::string>)
      {
        m_data.resize(oldSize + sizeof(Length) + value.size());
        const Length length = static_cast<Length>(value.size());
        memcpy(m_data.data() + oldSize, &length, sizeof(length));
        memcpy(m_data.data() + oldSize + sizeof(Length), value.data(), value.size());
      }
      else if constexpr(std::is_trivially_copyable_v<T>)
      {
        m_data.resize(oldSize + sizeof(value));
        memcpy(m_data.data() + oldSize, &value, sizeof(value));
      }
      else
        static_assert(sizeof(T) != sizeof(T));

      updateDataSize();
    }

    template<typename T>
    void write(const std::vector<T>& value)
    {
      if constexpr(std::is_same_v<T,std::string_view> || std::is_same_v<T,std::string>)
      {
        write(static_cast<Length>(value.size()));
        for(const auto& v : value)
          write(v);
      }
      else if constexpr(std::is_trivially_copyable_v<T>)
      {
        write(static_cast<Length>(value.size())); // number of elements, not bytes!
        const Length dataSize = value.size() * sizeof(T);
        const size_t oldSize = m_data.size();
        m_data.resize(oldSize + dataSize);
        memcpy(m_data.data() + oldSize, value.data(), dataSize);
      }
      else
        static_assert(sizeof(T) != sizeof(T));

      updateDataSize();
    }

    void writeBlock()
    {
      write<uint32_t>(0);
      m_block.push(header().dataSize);
    }

    void writeBlockEnd()
    {
      assert(!m_block.empty());
      const uint32_t blockSize = header().dataSize - m_block.top();
      memcpy(m_data.data() + sizeof(Header) + m_block.top() - sizeof(uint32_t), &blockSize, sizeof(blockSize));
      m_block.pop();
    }
};

#endif
