/**
 * client/src/network/property.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_PROPERTY_HPP
#define TRAINTASTIC_CLIENT_NETWORK_PROPERTY_HPP

#include "abstractproperty.hpp"
#include <QVariant>

class Object;

class Property : public AbstractProperty
{
  Q_OBJECT

  friend class Connection;

  protected:
    QVariant m_value;
    QString m_enumOrSetName;

  public:
    explicit Property(Object& object, const QString& name, ValueType type, PropertyFlags flags, const QVariant& value);

    const QString& enumName() const final
    {
      Q_ASSERT(type() == ValueType::Enum);
      Q_ASSERT(!m_enumOrSetName.isEmpty());
      return m_enumOrSetName;
    }

    const QString& setName() const final
    {
      Q_ASSERT(type() == ValueType::Set);
      Q_ASSERT(!m_enumOrSetName.isEmpty());
      return m_enumOrSetName;
    }

    bool toBool() const final { return m_value.toBool(); }
    int toInt() const final { return m_value.toInt(); }
    int64_t toInt64() const final { return m_value.toLongLong(); }
    double toDouble() const final { return m_value.toDouble(); }
    QString toString() const final { return m_value.toString(); }
    QVariant toVariant() const final { return m_value; }

    [[nodiscard]] int setValueBool(bool value, std::function<void(std::optional<Error>)> callback) final;
    [[nodiscard]] int setValueInt64(int64_t value, std::function<void(std::optional<Error>)> callback) final;
    [[nodiscard]] int setValueDouble(double value, std::function<void(std::optional<Error>)> callback) final;
    [[nodiscard]] int setValueString(const QString& value, std::function<void(std::optional<Error>)> callback) final;

  public slots:
    void setValueBool(bool value) final;
    void setValueInt(int value) final;
    void setValueInt64(int64_t value) final;
    void setValueDouble(double value) final;
    void setValueString(const QString& value) final;
};

#endif
