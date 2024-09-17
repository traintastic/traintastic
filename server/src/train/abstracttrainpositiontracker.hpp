/**
 * server/src/train/abstracttrainpositiontracker.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_TRAIN_ABSTRACTTRAINPOSITIONTRACKER_HPP
#define TRAINTASTIC_SERVER_TRAIN_ABSTRACTTRAINPOSITIONTRACKER_HPP

#include <memory>
#include <boost/asio/steady_timer.hpp>

class Train;

class AbstractTrainPositionTracker
{
public:
    AbstractTrainPositionTracker(const std::shared_ptr<Train>& train);
    virtual ~AbstractTrainPositionTracker();

    virtual void trainSpeedChanged(double physicalSpeedMS) = 0;

private:
  std::weak_ptr<Train> m_train;
};

class DeadlineTrainPositionTracker : public AbstractTrainPositionTracker
{
public:
    DeadlineTrainPositionTracker(const std::shared_ptr<Train>& train,
                                 double targetTravelledMeters_,
                                 const std::function<bool(double&)>& callback);

    void trainSpeedChanged(double physicalSpeedMS) override;

private:
    double targetTravelledMeters = 0;
    double currentTravelledMeters = 0;
    double currentTrainSpeed = 0;
    boost::asio::steady_timer expectedArrivalTimer;
    std::chrono::steady_clock::time_point lastSpeedChange;

    std::function<bool(double&)> m_onTargetCallback;
};

#endif // TRAINTASTIC_SERVER_TRAIN_ABSTRACTTRAINPOSITIONTRACKER_HPP
