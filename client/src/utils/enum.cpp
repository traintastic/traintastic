/**
 * client/src/utils/enum.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#include "enum.hpp"
#include "../network/abstractproperty.hpp"
#include <traintastic/locale/locale.hpp>
#include <traintastic/enum/decoderfunctionfunction.hpp>
#include <traintastic/enum/decoderfunctiontype.hpp>
#include <traintastic/enum/decoderprotocol.hpp>
#include <traintastic/enum/direction.hpp>
#include <traintastic/enum/lengthunit.hpp>
#include <traintastic/enum/loconetinterfacetype.hpp>
#include <traintastic/enum/loconetcommandstation.hpp>
#include <traintastic/enum/loconetserialinterface.hpp>
#include <traintastic/enum/outputaction.hpp>
#include <traintastic/enum/sensortype.hpp>
#include <traintastic/enum/serialflowcontrol.hpp>
#include <traintastic/enum/signalaspect.hpp>
#include <traintastic/enum/speedunit.hpp>
#include <traintastic/enum/turnoutposition.hpp>
#include <traintastic/enum/weightunit.hpp>
#include <traintastic/enum/worldscale.hpp>
#include <traintastic/enum/xpressnetcommandstation.hpp>
#include <traintastic/enum/xpressnetserialinterface.hpp>

#define GET_ENUM_VALUES(_type) \
  if(enumName == EnumName<_type>::value) \
  { \
    for(auto it : EnumValues<_type>::value) \
      values.append(static_cast<qint64>(it.first)); \
  } \
  else

QVector<qint64> enumValues(const QString& enumName)
{
  QVector<qint64> values;
  GET_ENUM_VALUES(LengthUnit)
  GET_ENUM_VALUES(SpeedUnit)
  GET_ENUM_VALUES(WeightUnit)
  {} // fix final else
  return values;
}

#define TRANSLATE_ENUM(_type) \
  if(enumName == EnumName<_type>::value) \
  { return Locale::tr(enumName + ":" + EnumValues<_type>::value.at(static_cast<_type>(value))); } \
  else

QString translateEnum(const QString& enumName, qint64 value)
{
  TRANSLATE_ENUM(DecoderFunctionFunction)
  TRANSLATE_ENUM(DecoderFunctionType)
  TRANSLATE_ENUM(DecoderProtocol)
  TRANSLATE_ENUM(Direction)
  TRANSLATE_ENUM(LengthUnit)
  TRANSLATE_ENUM(LocoNetCommandStation)
  TRANSLATE_ENUM(LocoNetInterfaceType)
  TRANSLATE_ENUM(LocoNetSerialInterface)
  TRANSLATE_ENUM(OutputAction)
  TRANSLATE_ENUM(SensorType)
  TRANSLATE_ENUM(SerialFlowControl)
  TRANSLATE_ENUM(SignalAspect)
  TRANSLATE_ENUM(SpeedUnit)
  TRANSLATE_ENUM(TurnoutPosition)
  TRANSLATE_ENUM(WeightUnit)
  TRANSLATE_ENUM(WorldScale)
  TRANSLATE_ENUM(XpressNetCommandStation)
  TRANSLATE_ENUM(XpressNetSerialInterface)
  return enumName + "@" + QString::number(value);
}

QString translateEnum(const AbstractProperty& property)
{
  Q_ASSERT(property.type() == ValueType::Enum);
  return translateEnum(property.enumName(), property.toInt64());
}
