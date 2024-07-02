/**
 * client/src/network/abstractproperty.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2023-2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_ABSTRACTPROPERTY_HPP
#define TRAINTASTIC_CLIENT_NETWORK_ABSTRACTPROPERTY_HPP

#include "baseproperty.hpp"
#include <optional>
#include <traintastic/set/set.hpp>

struct Error;

class AbstractProperty : public BaseProperty
{
  Q_OBJECT

  protected:
    explicit AbstractProperty(Object& object, const QString& name, ValueType type, PropertyFlags flags) :
      BaseProperty(object, name, type, flags)
    {
    }

  public:
    virtual bool toBool() const            { Q_ASSERT(false); return false; }
    virtual int toInt() const              { Q_ASSERT(false); return 0; }
    virtual int64_t toInt64() const        { Q_ASSERT(false); return 0; }
    virtual double toDouble() const        { Q_ASSERT(false); return 0; }
    virtual QString toString() const       { Q_ASSERT(false); return ""; }
    virtual QVariant toVariant() const     { Q_ASSERT(false); return QVariant(); }

    template<typename T>
    T toEnum() const
    {
      static_assert(std::is_enum_v<T>);
      return static_cast<T>(toInt64());
    }

    template<typename T>
    void setValueEnum(T value)
    {
      static_assert(std::is_enum_v<T>);
      return setValueInt64(static_cast<int64_t>(value));
    }

    template<typename T>
    T toSet() const
    {
      static_assert(is_set_v<T>);
      return static_cast<T>(toInt64());
    }

    template<typename T>
    void setValueSet(T value)
    {
      static_assert(is_set_v<T>);
      return setValueInt64(static_cast<int64_t>(value));
    }

    [[nodiscard]] virtual int setValueBool(bool value, std::function<void(std::optional<Error>)> /*callback*/) { Q_ASSERT(value != value); return -1; }
    [[nodiscard]] virtual int setValueInt(int value, std::function<void(std::optional<Error>)> /*callback*/) { Q_ASSERT(value != value); return -1; }
    [[nodiscard]] virtual int setValueInt64(int64_t value, std::function<void(std::optional<Error>)> /*callback*/) { Q_ASSERT(value != value); return -1; }
    [[nodiscard]] virtual int setValueDouble(double value, std::function<void(std::optional<Error>)> /*callback*/) { Q_ASSERT(value != value); return -1; }
    [[nodiscard]] virtual int setValueString(const QString& value, std::function<void(std::optional<Error>)> /*callback*/) { Q_ASSERT(value != value); return -1; }

  signals:
    void valueChangedBool(bool newValue);
    void valueChangedInt(int newValue);
    void valueChangedInt64(int64_t newValue);
    void valueChangedDouble(double newValue);
    void valueChangedString(const QString& newValue);

  public slots:
    virtual void setValueBool(bool value)      { Q_ASSERT(value != value); }
    virtual void setValueInt(int value)          { Q_ASSERT(value != value); }
    virtual void setValueInt64(int64_t value)          { Q_ASSERT(value != value); }
    virtual void setValueDouble(double value)              { Q_ASSERT(value != value); }
    virtual void setValueString(const QString& value)  { Q_ASSERT(value != value); }

    void setValueVariant(const QVariant& value)
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
      switch(value.typeId())
#else
      switch(value.type())
#endif
      {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        case QMetaType::Bool:
#else
        case QVariant::Bool:
#endif
          setValueBool(value.toBool());
          break;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        case QMetaType::Int:
#else
        case QVariant::Int:
#endif
          setValueInt(value.toInt());
          break;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        case QMetaType::UInt:
        case QMetaType::LongLong:
        case QMetaType::ULongLong:
#else
        case QVariant::UInt:
        case QVariant::LongLong:
        case QVariant::ULongLong:
#endif
          setValueInt64(value.toLongLong());
          break;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        case QMetaType::Double:
#else
        case QVariant::Double:
#endif
          setValueDouble(value.toDouble());
          break;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        case QMetaType::QString:
#else
        case QVariant::String:
#endif
          setValueString(value.toString());
          break;

        default: /*[[unlikely]]*/
          assert(false);
          break;
      }
    }
};

#endif
