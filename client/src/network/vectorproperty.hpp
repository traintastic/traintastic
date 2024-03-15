/**
 * client/src/network/vectorproperty.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_VECTORPROPERTY_HPP
#define TRAINTASTIC_CLIENT_NETWORK_VECTORPROPERTY_HPP

#include "abstractvectorproperty.hpp"
#include <QVariant>

class Object;

class VectorProperty : public AbstractVectorProperty
{
  Q_OBJECT

  friend class Connection;

  protected:
    QVariantList m_values;
    QString m_enumOrSetName;

  public:
    VectorProperty(Object& object, const QString& name, ValueType type, PropertyFlags flags, QVariantList values) :
      AbstractVectorProperty(object, name, type, flags)
    {
      m_values = std::move(values); // Somehow m_values{std::move(values)} doesn't work, don't know why yet.
    }

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

    int size() const final
    {
      return m_values.size();
    }

    bool getBool(int index) const final
    {
      Q_ASSERT(index >= 0 && index < size());
      return m_values[index].toBool();
    }

    int getInt(int index) const final
    {
      Q_ASSERT(index >= 0 && index < size());
      return m_values[index].toInt();
    }

    int64_t getInt64(int index) const final
    {
      Q_ASSERT(index >= 0 && index < size());
      return m_values[index].toLongLong();
    }

    double getDouble(int index) const final
    {
      Q_ASSERT(index >= 0 && index < size());
      return m_values[index].toDouble();
    }

    QString getString(int index) const final
    {
      Q_ASSERT(index >= 0 && index < size());
      return m_values[index].toString();
    }

    QVariant getVariant(int index) const final
    {
      Q_ASSERT(index >= 0 && index < size());
      return m_values[index];
    }

    void setBool(int index, bool value) final;
    void setInt(int index, int value) final;
    void setInt64(int index, qint64 value) final;
    void setDouble(int index, double value) final;
    void setString(int index, const QString& value) final;
};

#endif
