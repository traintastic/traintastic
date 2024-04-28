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
        static_cast<SignalRailTileITA &>(signal_).aspectITA.setValue(aspectITA);

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
    aspectITA{this, "aspect_ita", SignalAspectITA::Unknown, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly}
{
    outputMap.setValueInternal(std::make_shared<SignalOutputMap>(*this, outputMap.name(), std::initializer_list<SignalAspect>{SignalAspect::Stop, SignalAspect::ProceedReducedSpeed, SignalAspect::Proceed}, getDefaultActionValue));

    Attributes::addValues(aspect, aspectValues);
    m_interfaceItems.add(aspect);

    Attributes::addValues(setAspect, setAspectValues);
    m_interfaceItems.add(setAspect);

    connectOutputMap();
}

void SignalRailTileITA::boardModified()
{
    m_signalPath = std::make_unique<SignalPath>(*this);
    SignalRailTile::boardModified();
}
