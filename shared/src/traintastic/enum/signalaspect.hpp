/**
 * shared/src/traintastic/enum/signalaspect.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_SIGNALASPECT_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_SIGNALASPECT_HPP

#include <cstdint>
#include "enum.hpp"

enum class SignalAspect : uint8_t
{
  Unknown = 0,
  Stop = 1,
  Proceed = 2,
  ProceedReducedSpeed = 3,
};

TRAINTASTIC_ENUM(SignalAspect, "signal_aspect", 4,
{
  {SignalAspect::Unknown, "unknown"},
  {SignalAspect::Stop, "stop"},
  {SignalAspect::Proceed, "proceed"},
  {SignalAspect::ProceedReducedSpeed, "proceed_reduced_speed"},
});

enum SignalAspectITA_ingredients : uint8_t
{
    ViaImpedita = 1 << 0,
    ViaLibera = 1 << 1,
    AvvisoViaImpedita = ViaLibera | ViaImpedita,
    AvvisoMASK = AvvisoViaImpedita,

    Deviata = 1 << 2,

    AvvisoRiduzione30 = 1 << 3,
    AvvisoRiduzione60 = 1 << 4,
    AvvisoRiduzione100 = AvvisoRiduzione30 | AvvisoRiduzione60,
    AvvisoRiduzioneMASK = AvvisoRiduzione100,

    Riduzione30 = 1 << 5,
    Riduzione60 = 1 << 6,
    Riduzione100 = Riduzione30 | Riduzione60,
    RiduzioneMASK = Riduzione100,

    BinarioIngombroTronco = 1 << 7 | AvvisoViaImpedita
};

using ITA = SignalAspectITA_ingredients;

enum class SignalAspectITA : uint8_t
{
    Unknown = 0,
    ViaImpedita = ITA::ViaImpedita,
    ViaLibera = ITA::ViaLibera,
    ViaLibera_AvvisoViaImpedita = ITA::AvvisoViaImpedita,

    ViaLibera_AvvisoRiduzione30  = ITA::ViaLibera | ITA::AvvisoRiduzione30,
    ViaLibera_AvvisoRiduzione60  = ITA::ViaLibera | ITA::AvvisoRiduzione60,
    ViaLibera_AvvisoRiduzione100 = ITA::ViaLibera | ITA::AvvisoRiduzione100,

    Riduzione30_AvvisoViaLibera  = ITA::ViaLibera | ITA::Riduzione30,
    Riduzione60_AvvisoViaLibera  = ITA::ViaLibera | ITA::Riduzione60,
    Riduzione100_AvvisoViaLibera = ITA::ViaLibera | ITA::Riduzione100,

    Riduzione30_AvvisoViaImpedita  = ITA::AvvisoViaImpedita | ITA::Riduzione30,
    Riduzione60_AvvisoViaImpedita  = ITA::AvvisoViaImpedita | ITA::Riduzione60,
    Riduzione100_AvvisoViaImpedita = ITA::AvvisoViaImpedita | ITA::Riduzione100,

    Riduzione30_Avviso30  = ITA::ViaLibera | ITA::Riduzione30  | ITA::AvvisoRiduzione30,
    Riduzione60_Avviso30  = ITA::ViaLibera | ITA::Riduzione60  | ITA::AvvisoRiduzione30,
    Riduzione100_Avviso30 = ITA::ViaLibera | ITA::Riduzione100 | ITA::AvvisoRiduzione30,

    Riduzione30_Avviso60  = ITA::ViaLibera | ITA::Riduzione30  | ITA::AvvisoRiduzione60,
    Riduzione60_Avviso60  = ITA::ViaLibera | ITA::Riduzione60  | ITA::AvvisoRiduzione60,
    Riduzione100_Avviso60 = ITA::ViaLibera | ITA::Riduzione100 | ITA::AvvisoRiduzione60,

    Riduzione30_Avviso100  = ITA::ViaLibera | ITA::Riduzione30  | ITA::AvvisoRiduzione100,
    Riduzione60_Avviso100  = ITA::ViaLibera | ITA::Riduzione60  | ITA::AvvisoRiduzione100,
    Riduzione100_Avviso100 = ITA::ViaLibera | ITA::Riduzione100 | ITA::AvvisoRiduzione100,

    BinarioIngombroTronco        = ITA::BinarioIngombroTronco,
    BinarioIngombroTroncoDeviato = ITA::BinarioIngombroTronco | ITA::Deviata
};

TRAINTASTIC_ENUM(SignalAspectITA, "signal_aspect_ita", 24,
{
    {SignalAspectITA::Unknown, "unknown"},
    {SignalAspectITA::ViaImpedita, "stop"},
    {SignalAspectITA::ViaLibera, "proceed"},
    {SignalAspectITA::ViaLibera_AvvisoViaImpedita, "avv_stop"},

    {SignalAspectITA::ViaLibera_AvvisoRiduzione30, "avv_30"},
    {SignalAspectITA::ViaLibera_AvvisoRiduzione60, "avv_60"},
    {SignalAspectITA::ViaLibera_AvvisoRiduzione100, "avv_100"},

    {SignalAspectITA::Riduzione30_AvvisoViaLibera, "rid_30"},
    {SignalAspectITA::Riduzione60_AvvisoViaLibera, "rid_60"},
    {SignalAspectITA::Riduzione100_AvvisoViaLibera, "rid_100"},

    {SignalAspectITA::Riduzione30_AvvisoViaImpedita, "rid_30_avv_stop"},
    {SignalAspectITA::Riduzione60_AvvisoViaImpedita, "rid_60_avv_stop"},
    {SignalAspectITA::Riduzione100_AvvisoViaImpedita, "rid_100_avv_stop"},

    {SignalAspectITA::Riduzione30_Avviso30, "rid_30_avv_30"},
    {SignalAspectITA::Riduzione60_Avviso30, "rid_60_avv_30"},
    {SignalAspectITA::Riduzione100_Avviso30, "rid_100_avv_30"},

    {SignalAspectITA::Riduzione30_Avviso60, "rid_30_avv_60"},
    {SignalAspectITA::Riduzione60_Avviso60, "rid_60_avv_60"},
    {SignalAspectITA::Riduzione100_Avviso60, "rid_100_avv_60"},

    {SignalAspectITA::Riduzione30_Avviso100, "rid_30_avv_100"},
    {SignalAspectITA::Riduzione60_Avviso100, "rid_60_avv_100"},
    {SignalAspectITA::Riduzione100_Avviso100, "rid_100_avv_100"},

    {SignalAspectITA::BinarioIngombroTronco, "bin_ingombro"},
    {SignalAspectITA::BinarioIngombroTroncoDeviato, "bin_ingombro_dev"},
});

enum class SignalAspectITALampState
{
    Off = 0,
    On = 1,
    Blinking = 2,
    BlinkingInverse = 3
};

TRAINTASTIC_ENUM(SignalAspectITALampState, "signal_ita_lamp_state", 4,
{
    {SignalAspectITALampState::Off, "off"},
    {SignalAspectITALampState::On, "on"},
    {SignalAspectITALampState::Blinking, "blink"},
    {SignalAspectITALampState::BlinkingInverse, "blink_inverse"},
});

enum class SignalAspectITALampColor
{
    Red = 0,
    Green = 1,
    Yellow = 2
};

TRAINTASTIC_ENUM(SignalAspectITALampColor, "signal_ita_lamp_color", 3,
{
    {SignalAspectITALampColor::Red, "off"},
    {SignalAspectITALampColor::Green, "on"},
    {SignalAspectITALampColor::Yellow, "blink"}
});

typedef std::pair<SignalAspectITALampState, SignalAspectITALampColor> SignalAspectITALampPair;

#endif
