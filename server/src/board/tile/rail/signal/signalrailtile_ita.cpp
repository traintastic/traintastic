/**
 * server/src/board/tile/rail/signal/signal3aspectrailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2024 Reinder Feenstra
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

#include "signalrailtile_ita.hpp"
#include "../blockrailtile.hpp"
#include "../turnout/turnoutrailtile.hpp"
#include "../../../map/abstractsignalpath.hpp"
#include "../../../map/blockpath.hpp"
#include "../../../../core/attributes.hpp"
#include "../../../../core/method.tpp"
#include "../../../../core/objectproperty.tpp"
#include "../../../../hardware/output/outputcontroller.hpp"

static const std::array<SignalAspect, 4> aspectValues = {SignalAspect::Stop, SignalAspect::ProceedReducedSpeed, SignalAspect::Proceed, SignalAspect::Unknown};
static const std::array<SignalAspect, 3> setAspectValues = {SignalAspect::Stop, SignalAspect::ProceedReducedSpeed, SignalAspect::Proceed};

namespace
{
class SignalPath : public AbstractSignalPath
{
private:
    static bool hasSignalReservedPathToBlock(const SignalRailTile& signalTile, const BlockRailTile& blockTile)
    {
        const auto path = signalTile.reservedPath();
        return path && path->toBlock().get() == &blockTile;
    }

    static bool isPathReserved(const BlockRailTile& from, BlockSide fromSide, const BlockRailTile& to)
    {
        if(const auto path = from.getReservedPath(fromSide))
        {
            return (&to == path->toBlock().get());
        }
        return false;
    }

protected:
    int getTurnoutMaxSpeed(const std::shared_ptr<TurnoutRailTile>& turnout, bool &outDeviata) const
    {
        outDeviata = false;
        switch (turnout->tileId())
        {
        case TileId::RailTurnoutLeft45:
        case TileId::RailTurnoutRight45:
        case TileId::RailTurnoutLeft90:
        case TileId::RailTurnoutRight90:
        case TileId::RailTurnoutLeftCurved:
        case TileId::RailTurnoutRightCurved:
        {
            if(turnout->position.value() != TurnoutPosition::Straight)
            {
                outDeviata = true;
                return 60;
            }
            break;
        }

        case TileId::RailTurnoutWye:
        {
            // Always non-straight
            outDeviata = true;
            return 100;
        }
        case TileId::RailTurnout3Way:
        {
            if(turnout->position.value() != TurnoutPosition::Straight)
            {
                outDeviata = true;
                return 30;
            }
            break;
        }
        case TileId::RailTurnoutSingleSlip:
        case TileId::RailTurnoutDoubleSlip:
        {
            auto pos = turnout->position.value();
            if(pos == TurnoutPosition::Left || pos == TurnoutPosition::Right)
            {
                outDeviata = true;
                return 30;
            }
            break;
        }
        default:
            break;
        }

        return 0;
    }

    int getPathMaxSpeed(const Item* item, bool &outDeviata) const
    {
        outDeviata = false;
        int maxSpeed = 0;

        while(item)
        {
            if(const auto* turnoutItem = dynamic_cast<const TurnoutItem*>(item))
            {
                auto turnout = turnoutItem->turnout();

                bool newDeviata = false;
                int turnoutMaxSpeed = getTurnoutMaxSpeed(turnout, newDeviata);

                if(maxSpeed == 0 || turnoutMaxSpeed < maxSpeed)
                    maxSpeed = turnoutMaxSpeed;

                outDeviata |= newDeviata;
            }
            if(const auto* signalItem = dynamic_cast<const SignalItem*>(item))
            {
                (void)signalItem;
                break;
            }
            item = item->next().get();
        }

        return maxSpeed;
    }

    SignalAspectITA determineAspectITA() const
    {
        const auto* blockItem = nextBlock();
        if(!blockItem)
        {
            return SignalAspectITA::ViaImpedita;
        }
        const auto blockState = blockItem->blockState();

        auto [blockItem2, signalItem2] = nextBlockOrSignal(blockItem->next().get());

        bool isDeviata = false;
        int speedReduction = getPathMaxSpeed(root(), isDeviata);
        SignalAspectITA_ingredients riduzione = SignalAspectITA_ingredients(0);
        if(speedReduction > 0)
        {
            if(speedReduction < 60)
                riduzione = SignalAspectITA_ingredients::Riduzione30;
            else if(speedReduction < 100)
                riduzione = SignalAspectITA_ingredients::Riduzione60;
            else
                riduzione = SignalAspectITA_ingredients::Riduzione100;
        }

        if((!requireReservation() && blockState == BlockState::Free) ||
            (blockState == BlockState::Reserved && hasSignalReservedPathToBlock(signal(), *blockItem->block())))
        {
            if(blockItem2)
            {
                const auto blockState2 = blockItem2->blockState();

                if(blockState2 == BlockState::Free ||
                    (blockState2 == BlockState::Reserved && isPathReserved(*blockItem->block(), ~blockItem->enterSide(), *blockItem2->block())))
                {
                    return SignalAspectITA(SignalAspectITA_ingredients::ViaLibera | riduzione);
                }

                // Nei segnali di protezione delle stazioni si usa sempre il rosso/giallo/giallo
                if(requireReservation())
                    return SignalAspectITA::BinarioIngombroTroncoDeviato;

                // In linea si usa il giallo/giallo per avvisare di distanze ridottissime del blocco successivo
                return SignalAspectITA::BinarioIngombroTronco;
            }
            else if(signalItem2)
            {
                auto signalTile = signalItem2->signal();

                auto signal2ITA = std::dynamic_pointer_cast<SignalRailTileITA>(signalItem2->signal());
                if(signal2ITA)
                {
                    SignalAspectITA nextAspect = signal2ITA->aspectITA.value();
                    switch (nextAspect)
                    {
                    case SignalAspectITA::ViaImpedita:
                    case SignalAspectITA::Unknown:
                    {
                        // Cannot allow 100 if next signal is Stop (See segnalifs.it)
                        if(riduzione == SignalAspectITA_ingredients::Riduzione100)
                            riduzione = SignalAspectITA_ingredients::Riduzione60;

                        return SignalAspectITA(SignalAspectITA_ingredients::AvvisoViaImpedita | riduzione);
                    }
                    default:
                        break;
                    }

                    // Extract next signal max allowed speed
                    SignalAspectITA_ingredients nextSpeedReduction = SignalAspectITA_ingredients(int(nextAspect) & SignalAspectITA_ingredients::RiduzioneMASK);

                    // Convert in avviso of next signal max speed
                    SignalAspectITA_ingredients avvisoReduction = SignalAspectITA_ingredients(nextSpeedReduction << 2);

                    return SignalAspectITA(SignalAspectITA_ingredients::ViaLibera | avvisoReduction | riduzione);
                }

                if(signalTile && signalTile->aspect != SignalAspect::Unknown && signalTile->aspect != SignalAspect::Stop)
                {
                    switch(signalTile->aspect.value())
                    {
                    case SignalAspect::ProceedReducedSpeed:
                    case SignalAspect::Proceed:
                        return SignalAspectITA::ViaLibera;

                    case SignalAspect::Stop:
                    case SignalAspect::Unknown:
                        break;
                    }
                }
            }
            return SignalAspectITA::ViaLibera_AvvisoViaImpedita;
        }
        return SignalAspectITA::ViaImpedita;
    }

    SignalAspect determineAspect() const final
    {
        SignalAspectITA aspectITA = determineAspectITA();

        auto &signal_ = const_cast<SignalRailTile&>(signal());
        static_cast<SignalRailTileITA &>(signal_).setAspectITA(aspectITA);

        switch (aspectITA)
        {
        case SignalAspectITA::ViaLibera:
            return SignalAspect::Proceed;
        case SignalAspectITA::ViaLibera_AvvisoViaImpedita:
            return SignalAspect::ProceedReducedSpeed;
        default:
            break;
        }

        return SignalAspect::Stop;
    }

public:
    SignalPath(SignalRailTileITA& signal)
        : AbstractSignalPath(signal, 2)
    {
    }
};
}

SignalRailTileITA::SignalRailTileITA(World& world, std::string_view _id) :
    SignalRailTile(world, _id, TileId::RailSignal3Aspect),
    aspectITA{this, "aspect_ita", SignalAspectITA::Unknown, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly},
    lampState1{this, "lamp_state_1", SignalAspectITALampState::Off, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly},
    lampColor1{this, "lamp_color_1", SignalAspectITALampColor::Red, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly},
    lampState2{this, "lamp_state_2", SignalAspectITALampState::Off, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly},
    lampColor2{this, "lamp_color_2", SignalAspectITALampColor::Red, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly},
    lampState3{this, "lamp_state_3", SignalAspectITALampState::Off, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly},
    lampColor3{this, "lamp_color_3", SignalAspectITALampColor::Red, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly}
{
    outputMap.setValueInternal(std::make_shared<SignalOutputMap>(*this, outputMap.name(), std::initializer_list<SignalAspect>{SignalAspect::Stop, SignalAspect::ProceedReducedSpeed, SignalAspect::Proceed}, getDefaultActionValue));

    Attributes::addValues(aspect, aspectValues);
    m_interfaceItems.add(aspect);

    Attributes::addValues(setAspect, setAspectValues);
    m_interfaceItems.add(setAspect);

    connectOutputMap();
}

void SignalRailTileITA::setAspectITA(SignalAspectITA value)
{
    calculateLampStates();

    aspectITA.setValueInternal(value);

    //TODO: aspectChanged is already called by setAspect()
}

void SignalRailTileITA::calculateLampStates()
{
    SignalAspectITA value = aspectITA.value();
    SignalAspectITA_ingredients ingredients = SignalAspectITA_ingredients(value);

    switch (value)
    {
    case SignalAspectITA::Unknown:
    case SignalAspectITA::ViaImpedita:
    case SignalAspectITA::ViaLibera:
    case SignalAspectITA::ViaLibera_AvvisoViaImpedita:
    {
        lampState2.setValue(SignalAspectITALampState::Off);
        lampState3.setValue(SignalAspectITALampState::Off);

        if(value == SignalAspectITA::Unknown)
            lampState1.setValue(SignalAspectITALampState::Off);
        else
        {
            lampState1.setValue(SignalAspectITALampState::On);
            switch (value)
            {
            case SignalAspectITA::ViaImpedita:
                lampColor1.setValue(SignalAspectITALampColor::Red);
                break;
            case SignalAspectITA::ViaLibera:
                lampColor1.setValue(SignalAspectITALampColor::Green);
                break;
            case SignalAspectITA::ViaLibera_AvvisoViaImpedita:
                lampColor1.setValue(SignalAspectITALampColor::Yellow);
                break;
            default:
                assert(false);
            }
        }
        return;
    }
    default:
        break;
    }

    bool shiftByOne = false;

    if((ingredients & RiduzioneMASK) != 0 || ingredients & Deviata)
    {
        // All begin with red lamp on top
        lampState1.setValue(SignalAspectITALampState::On);
        lampColor1.setValue(SignalAspectITALampColor::Red);

        shiftByOne = true;
    }

    auto& firstLampState = shiftByOne ? lampState2 : lampState1;
    auto& firstLampColor = shiftByOne ? lampColor2 : lampColor1;
    auto& secondLampState = shiftByOne ? lampState3 : lampState2;
    auto& secondLampColor = shiftByOne ? lampColor3 : lampColor2;

    SignalAspectITA_ingredients avvisoRiduzione = SignalAspectITA_ingredients(ingredients & SignalAspectITA_ingredients::AvvisoRiduzioneMASK);
    SignalAspectITA_ingredients avviso = SignalAspectITA_ingredients(ingredients & SignalAspectITA_ingredients::AvvisoMASK);

    if(ingredients & SignalAspectITA_ingredients::BinarioIngombroTronco)
    {
        firstLampState.setValue(SignalAspectITALampState::On);
        firstLampColor.setValue(SignalAspectITALampColor::Yellow);

        secondLampState.setValue(SignalAspectITALampState::On);
        secondLampColor.setValue(SignalAspectITALampColor::Yellow);
    }
    else if(avviso == SignalAspectITA_ingredients::AvvisoViaImpedita
               || (avviso == SignalAspectITA_ingredients::ViaLibera && !avvisoRiduzione))
    {
        firstLampState.setValue(SignalAspectITALampState::On);

        if(avviso == SignalAspectITA_ingredients::AvvisoViaImpedita)
            firstLampColor.setValue(SignalAspectITALampColor::Yellow);
        else
            firstLampColor.setValue(SignalAspectITALampColor::Green);

        secondLampState.setValue(SignalAspectITALampState::Off);
    }
    else if(avvisoRiduzione)
    {
        firstLampColor.setValue(SignalAspectITALampColor::Yellow);
        secondLampColor.setValue(SignalAspectITALampColor::Green);

        switch (avvisoRiduzione)
        {
        case SignalAspectITA_ingredients::AvvisoRiduzione30:
            firstLampState.setValue(SignalAspectITALampState::On);
            secondLampState.setValue(SignalAspectITALampState::On);
            break;

        case SignalAspectITA_ingredients::AvvisoRiduzione60:
            firstLampState.setValue(SignalAspectITALampState::Blinking);
            secondLampState.setValue(SignalAspectITALampState::Blinking);
            break;

        case SignalAspectITA_ingredients::AvvisoRiduzione100:
            firstLampState.setValue(SignalAspectITALampState::BlinkingInverse);
            secondLampState.setValue(SignalAspectITALampState::Blinking);
            break;

        default:
            break;
        }
    }
}

void SignalRailTileITA::boardModified()
{
    m_signalPath = std::make_unique<SignalPath>(*this);
    SignalRailTile::boardModified();
}
