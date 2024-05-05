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
#include "../../../../world/getworld.hpp"
#include "../../../../hardware/output/outputcontroller.hpp"

static const std::array<SignalAspectITA, 24> aspectValuesITA =
    {SignalAspectITA::Unknown,
     SignalAspectITA::ViaImpedita,
     SignalAspectITA::ViaLibera,
     SignalAspectITA::ViaLibera_AvvisoViaImpedita,
     SignalAspectITA::ViaLibera_AvvisoRiduzione30,
     SignalAspectITA::ViaLibera_AvvisoRiduzione60,
     SignalAspectITA::ViaLibera_AvvisoRiduzione100,
     SignalAspectITA::Riduzione30_AvvisoViaLibera,
     SignalAspectITA::Riduzione60_AvvisoViaLibera,
     SignalAspectITA::Riduzione100_AvvisoViaLibera,
     SignalAspectITA::Riduzione30_AvvisoViaImpedita,
     SignalAspectITA::Riduzione60_AvvisoViaImpedita,
     SignalAspectITA::Riduzione100_AvvisoViaImpedita,
     SignalAspectITA::Riduzione30_Avviso30,
     SignalAspectITA::Riduzione60_Avviso30,
     SignalAspectITA::Riduzione100_Avviso30,
     SignalAspectITA::Riduzione30_Avviso60,
     SignalAspectITA::Riduzione60_Avviso60,
     SignalAspectITA::Riduzione100_Avviso60,
     SignalAspectITA::Riduzione30_Avviso100,
     SignalAspectITA::Riduzione60_Avviso100,
     SignalAspectITA::Riduzione100_Avviso100,
     SignalAspectITA::BinarioIngombroTronco,
     SignalAspectITA::BinarioIngombroTroncoDeviato};

static const std::array<SignalAspectITALampState, 4> lampStateValues = {SignalAspectITALampState::Off,
                                                                        SignalAspectITALampState::On,
                                                                        SignalAspectITALampState::Blinking,
                                                                        SignalAspectITALampState::BlinkingInverse};

static const std::array<SignalAspectITALampColor, 4> lampColorValues = {SignalAspectITALampColor::Red,
                                                                        SignalAspectITALampColor::Green,
                                                                        SignalAspectITALampColor::Yellow};

static const std::array<SignalAspectITAAuxiliarySpeedReduction, 4> auxiliarySpeedReductionValues =
    {SignalAspectITAAuxiliarySpeedReduction::None,
     SignalAspectITAAuxiliarySpeedReduction::Rappel,
     SignalAspectITAAuxiliarySpeedReduction::Triangle30,
     SignalAspectITAAuxiliarySpeedReduction::Triangle60};

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

    inline const SignalRailTileITA& signalITA() const
    {
        return static_cast<const SignalRailTileITA&>(signal());
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

                if(maxSpeed == 0 || (turnoutMaxSpeed > 0 && turnoutMaxSpeed < maxSpeed))
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
        const bool anticipationOnly = signalITA().onlyAnticipateNextSignal.value();

        const auto* blockItem = nextBlock();
        if(!blockItem)
        {
            return SignalAspectITA::ViaImpedita;
        }
        const auto blockState = blockItem->blockState();

        auto [blockItem2, signalItem2] = nextBlockOrSignal(blockItem->next().get());


        if(anticipationOnly)
        {
            // Anticipation-only signal only depent on their next 1st category signal
            // of which they anticipate the aspect
            // Try to get next signal BEFORE next block
            auto signalBeforeBlock = std::get<1>(nextBlockOrSignal(root()));

            auto signalBeforeBlockITA = signalBeforeBlock ? std::dynamic_pointer_cast<SignalRailTileITA>(signalBeforeBlock->signal()) : nullptr;
            if(signalBeforeBlockITA)
                signalItem2 = signalBeforeBlock;
        }

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

        if(!anticipationOnly && blockState == BlockState::Occupied && (!requireReservation() || hasSignalReservedPathToBlock(signal(), *blockItem->block())))
        {
            // Next block is occupied, check if it's only partially occupied on exit side
            const auto &inputMap = blockItem->block()->inputMap;
            if(inputMap->items.size() < 2)
                return SignalAspectITA::ViaImpedita; // Block must have at least 2 sub-sections

            const auto sensor = blockItem->enterSide() == BlockSide::A ? inputMap->items.front() : inputMap->items.back();
            if(!sensor || sensor->value() != SensorState::Free)
                return SignalAspectITA::ViaImpedita; // Sub-sections on enter side must be free

            // Signals protecting station entrance always put red on top
            if(requireReservation())
                return SignalAspectITA::BinarioIngombroTroncoDeviato;

            // In railway lines this aspect it's used instead of plain yellow when distance from next block is extramely short.
            return SignalAspectITA::BinarioIngombroTronco;
        }

        // Anticipation-only signals do not require reservation
        if(anticipationOnly || (!requireReservation() && blockState == BlockState::Free) ||
            (blockState == BlockState::Reserved && hasSignalReservedPathToBlock(signal(), *blockItem->block())))
        {
            // Anticipation-only signals do not depend on block state but only on 1st atecory signal aspects
            if(blockItem2 && !anticipationOnly)
            {
                const auto blockState2 = blockItem2->blockState();

                if(blockState2 == BlockState::Free ||
                    (blockState2 == BlockState::Reserved && isPathReserved(*blockItem->block(), ~blockItem->enterSide(), *blockItem2->block())))
                {
                    return SignalAspectITA(SignalAspectITA_ingredients::ViaLibera | riduzione);
                }
            }
            else if(signalItem2)
            {
                auto signal2Tile = signalItem2->signal();

                auto signal2ITA = std::dynamic_pointer_cast<SignalRailTileITA>(signalItem2->signal());
                if(signal2ITA)
                {
                    SignalAspectITA nextAspect = signal2ITA->aspectITA.value();
                    switch (nextAspect)
                    {
                    case SignalAspectITA::ViaImpedita:
                    case SignalAspectITA::BinarioIngombroTronco:
                    case SignalAspectITA::BinarioIngombroTroncoDeviato:
                    case SignalAspectITA::Unknown:
                    {
                        // Cannot allow 100 if next signal is Stop
                        // See https://www.segnalifs.it/sfi/it/sa/2016_sa_immagini_1ctg.htm#B9c
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
                    SignalAspectITA_ingredients avvisoReduction = SignalAspectITA_ingredients(nextSpeedReduction >> 2);

                    return SignalAspectITA(SignalAspectITA_ingredients::ViaLibera | avvisoReduction | riduzione);
                }

                if(signal2Tile && signal2Tile->aspect != SignalAspect::Unknown && signal2Tile->aspect != SignalAspect::Stop)
                {
                    switch(signal2Tile->aspect.value())
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
        return static_cast<SignalAspect>(aspectITA);
    }

public:
    SignalPath(SignalRailTileITA& signal)
        : AbstractSignalPath(signal, 2)
    {
    }



    static SignalAspectITA adjustAspect(SignalAspectITA value, bool anticipateOnly, SignalAspectITAAuxiliarySpeedReduction auxReduction)
    {
        if(anticipateOnly)
        {
            SignalAspectITA_ingredients ingredients = SignalAspectITA_ingredients(value);

            // We cannot show "Stop" or partially occupied track aspect
            const bool isPartiallyOccupied = SignalAspectITA_ingredients(ingredients & SignalAspectITA_ingredients::BinarioIngombroTronco) == SignalAspectITA_ingredients::BinarioIngombroTronco;
            if(value == SignalAspectITA::ViaImpedita || isPartiallyOccupied)
                return SignalAspectITA::ViaLibera_AvvisoViaImpedita;

            // Remove speed reduction, keep "exected" anticipated speed reduction
            ingredients = SignalAspectITA_ingredients(ingredients & ~SignalAspectITA_ingredients::RiduzioneMASK);

            // Remove non-Straight path attribute
            ingredients = SignalAspectITA_ingredients(ingredients & ~SignalAspectITA_ingredients::Deviata);

            return SignalAspectITA(ingredients);
        }

        //TODO: robust logic to reject invalid aspects or impossible aspects due to lamps number/rappel/triangle
        if(auxReduction == SignalAspectITAAuxiliarySpeedReduction::Triangle30 || auxReduction == SignalAspectITAAuxiliarySpeedReduction::Triangle60)
        {
            // Signal always shows a speed reduction to 30 km/h (or 60 km/h if triangle has "60" inside it)
            SignalAspectITA_ingredients ingredients = SignalAspectITA_ingredients(value);
            SignalAspectITA_ingredients riduzione = SignalAspectITA_ingredients(ingredients & SignalAspectITA_ingredients::RiduzioneMASK);

            SignalAspectITA_ingredients newReduction = SignalAspectITA_ingredients::Riduzione30;

            if(auxReduction == SignalAspectITAAuxiliarySpeedReduction::Triangle60)
            {
                newReduction = SignalAspectITA_ingredients::Riduzione60;

                // Refuse to show non-Stop aspect if required speed limit is lower than 60 km/h
                // If this happens it means signal must be changed to Rappel or Triangle 30 km/h
                if(riduzione == SignalAspectITA_ingredients::Riduzione30)
                    value = SignalAspectITA::ViaImpedita;
            }

            switch (value)
            {
            case SignalAspectITA::Unknown:
            case SignalAspectITA::ViaImpedita:
            case SignalAspectITA::BinarioIngombroTronco:
            case SignalAspectITA::BinarioIngombroTroncoDeviato:
                // These aspect do not need additional speed limit
                break;

            default:
            {
                // Remove current speed reduction
                ingredients = SignalAspectITA_ingredients(ingredients & ~SignalAspectITA_ingredients::RiduzioneMASK);

                // Add new speed reduction imposed by triangle
                ingredients = SignalAspectITA_ingredients(ingredients | newReduction);
                value = SignalAspectITA(ingredients);
            }
            }
        }

        return value;
    }
};
}

SignalRailTileITA::SignalRailTileITA(World& world, std::string_view _id) :
    SignalRailTile(world, _id, TileId::RailSignalAspectITA),
    aspectITA{this, "aspect_ita", SignalAspectITA::Unknown, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly},
    onlyAnticipateNextSignal{this, "only_anticipate", false, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly,
      [this](bool /*value*/) -> void
      {
        updateAuxReduction();
      }},
    auxSpeedReduction{this, "aux_reduction", SignalAspectITAAuxiliarySpeedReduction::None, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly}
{
    const bool editable = contains(m_world.state.value(), WorldState::Edit);

    // Cast values to SignalAspect
    tcb::span<const SignalAspect, 24> aspectValues{reinterpret_cast<const SignalAspect *>(aspectValuesITA.begin()),
                                                   reinterpret_cast<const SignalAspect *>(aspectValuesITA.end())};

    // Chop first element ("Unknown" aspect)
    tcb::span<const SignalAspect, 23> setAspectValues{aspectValues.begin() + 1, aspectValues.end()};

    //TODO: this crashes upon saving because SignalAspectITA is converted to SignalAspect and it cannot find enum values
    //outputMap.setValueInternal(std::make_shared<SignalOutputMap>(*this, outputMap.name(), setAspectValues, getDefaultActionValue));

    outputMap.setValueInternal(std::make_shared<SignalOutputMap>(*this, outputMap.name(), std::initializer_list<SignalAspect>{SignalAspect::Stop, SignalAspect::ProceedReducedSpeed, SignalAspect::Proceed}, getDefaultActionValue));

    Attributes::addValues<SignalAspect>(aspect, aspectValues);
    m_interfaceItems.add(aspect);

    Attributes::addObjectEditor(aspectITA, false);
    Attributes::addValues(aspectITA, aspectValuesITA);
    m_interfaceItems.add(aspectITA);

    Attributes::addValues<bool, SignalAspect>(setAspect, setAspectValues);
    m_interfaceItems.add(setAspect);

    Attributes::addEnabled(onlyAnticipateNextSignal, editable);
    m_interfaceItems.add(onlyAnticipateNextSignal);

    Attributes::addValues(auxSpeedReduction, auxiliarySpeedReductionValues);
    Attributes::addEnabled(auxSpeedReduction, editable);
    m_interfaceItems.add(auxSpeedReduction);

    connectOutputMap();
}

bool SignalRailTileITA::doSetAspect(SignalAspect value, bool skipAction)
{
    SignalAspectITA valueITA = SignalAspectITA(value);
    valueITA = SignalPath::adjustAspect(valueITA, onlyAnticipateNextSignal, auxSpeedReduction);

    const auto* values = setAspect.tryGetValuesAttribute(AttributeName::Values);
    assert(values);
    if(!values->contains(static_cast<int64_t>(valueITA)))
        return false;
    if(aspectITA != valueITA)
    {
        //TODO: maybe do a custom OutputMapBase subclass
        (void)skipAction;
        //if(!skipAction)
        //    (*outputMap)[value]->execute();

        // Convert to basic aspect to allow interacting with 3 aspect signals
        switch (aspectITA)
        {
        case SignalAspectITA::ViaLibera:
            value = SignalAspect::Proceed;
            break;
        case SignalAspectITA::ViaImpedita:
            value = SignalAspect::Stop;
            break;
        default:
            value = SignalAspect::ProceedReducedSpeed;
            break;
        }

        // Store the "full" aspect in a custom property
        aspectITA.setValueInternal(valueITA);
        aspect.setValueInternal(value);
        aspectChanged(*this, value);
        fireEvent(onAspectChanged, shared_ptr<SignalRailTile>(), value);
    }
    return true;
}

void SignalRailTileITA::updateAuxReduction()
{
    const bool editable = contains(m_world.state.value(), WorldState::Edit);
    const bool editableAndStopped = editable && !contains(m_world.state.value(), WorldState::Run);

    // Only 1st category signal can have auxiliary speed reduction
    Attributes::setEnabled(auxSpeedReduction, !onlyAnticipateNextSignal && editableAndStopped);

    if(onlyAnticipateNextSignal)
        auxSpeedReduction = SignalAspectITAAuxiliarySpeedReduction::None;
}

void SignalRailTileITA::loaded()
{
    SignalRailTile::loaded();
    updateAuxReduction();
}

void SignalRailTileITA::boardModified()
{
    m_signalPath = std::make_unique<SignalPath>(*this);
    SignalRailTile::boardModified();
}

void SignalRailTileITA::worldEvent(WorldState state, WorldEvent event)
{
    SignalRailTile::worldEvent(state, event);

    const bool editable = contains(state, WorldState::Edit);
    const bool editableAndStopped = editable && !contains(state, WorldState::Run);

    Attributes::setEnabled(onlyAnticipateNextSignal, editableAndStopped);

    updateAuxReduction();
}
