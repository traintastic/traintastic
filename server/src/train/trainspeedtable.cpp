/**
 * server/src/train/trainspeedtable.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Filippo Gentile
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

#include "trainspeedtable.hpp"

#include "../vehicle/rail/vehiclespeedcurve.hpp"
#include "../utils/almostzero.hpp"

#include <cstring> // for std::memcpy()

TrainSpeedTable::Entry TrainSpeedTable::nullEntry = TrainSpeedTable::Entry();

TrainSpeedTable::TrainSpeedTable()
{

}

TrainSpeedTable::ClosestMatchRet TrainSpeedTable::getClosestMatch(uint32_t locoIdx, uint8_t step) const
{
  if(step == 0)
  {
      return {nullEntry, NULL_TABLE_ENTRY};
  }

  if(locoIdx >= locoCount)
    return {nullEntry, NULL_TABLE_ENTRY}; // Error

  for(uint8_t i = 0; i < mEntries.size(); i++)
  {
    const Entry &entry = mEntries.at(i);
    const uint8_t candidateStep = entry.getStepForLoco(locoIdx);
    if(candidateStep == step)
      return {entry, i + 1}; // Exact match, Shift by +1

    if(candidateStep > step)
    {
      // We got closest higher step
      // Check if lower step is even closer
      if(i > 0)
      {
        const Entry &prev = mEntries.at(i - 1);
        if((step - prev.getStepForLoco(locoIdx)) < (candidateStep - step))
        {
          // Previous match is closer
          return {prev, i}; // Shift by +1
        }

        // Higher match is closer
        return {entry, i + 1}; // Shift by +1
      }
    }
  }

  // Return highest match if available
  if(!mEntries.empty())
    return {*mEntries.rbegin(), mEntries.size()};

  return {nullEntry, NULL_TABLE_ENTRY}; // Empty table
}

TrainSpeedTable::ClosestMatchRet TrainSpeedTable::getClosestMatch(double speed) const
{
  for(uint8_t i = 0; i < mEntries.size(); i++)
  {
    const Entry& entry = mEntries.at(i);
    if(almostZero(entry.avgSpeed - speed))
      return {entry, i + 1}; // Shift by +1

    if(entry.avgSpeed > speed)
    {
      // Return previous entry
      if(i > 0)
      {
        // This would be "i - 1 + 1 = i", (Shift by +1)
        return {mEntries.at(i - 1), i};
      }

      // No matches below this, return zero
      return {nullEntry, NULL_TABLE_ENTRY};
    }
  }

  // Return highest match if available
  if(!mEntries.empty())
    return {*mEntries.rbegin(), mEntries.size()};

  return {nullEntry, NULL_TABLE_ENTRY}; // Empty table
}

const TrainSpeedTable::Entry& TrainSpeedTable::getEntryAt(uint8_t idx) const
{
  // NULL_TABLE_ENTRY returns null Entry as expected!
  if(idx == NULL_TABLE_ENTRY)
    return nullEntry;

  // Null entry is not stored, shift by -1
  return mEntries.at(idx - 1);
}

TrainSpeedTable TrainSpeedTable::buildTable(const std::vector<VehicleSpeedCurve> &locoMappings)
{
  // TODO: prefer pushing/pulling

  const uint32_t NUM_LOCOS = locoMappings.size();
  if(NUM_LOCOS == 0)
      return {};

  const uint32_t LAST_LOCO = NUM_LOCOS - 1;

  TrainSpeedTable table;
  table.locoCount = NUM_LOCOS;

  if(NUM_LOCOS == 1)
  {
    // Special case: only 1 locomotive
    // Table is a replica of locomotive speed curve
    const VehicleSpeedCurve& speedCurve = locoMappings.at(0);

    table.mEntries.reserve(126);
    for(int step = 1; step <= 126; step++)
    {
      double speed = speedCurve.getSpeedForStep(step);
      Entry entry;
      entry.stepForLoco_.reset(new uint8_t[1]);
      entry.avgSpeed = speed;
      entry.stepForLoco_[0] = step;
      table.mEntries.push_back(std::move(entry));
    }

    return table;
  }

  double maxTrainSpeed = locoMappings.at(0).getSpeedForStep(126);
  for(uint32_t locoIdx = 1; locoIdx < NUM_LOCOS; locoIdx++)
  {
    double maxSpeed = locoMappings.at(locoIdx).getSpeedForStep(126);
    if(maxSpeed < maxTrainSpeed)
      maxTrainSpeed = maxSpeed;
  }

  struct LocoStepCache
  {
    uint8_t currentStep = 0;
    uint8_t minAcceptedStep = 0;
    uint8_t maxAcceptedStep = 0;
    double minSpeedSoFar = 0;
    double maxSpeedSoFar = 0;
    double currentSpeed = 0;
  };

  constexpr double MAX_SPEED_DIFF = 0.005;
  maxTrainSpeed += MAX_SPEED_DIFF;

  std::vector<LocoStepCache> stepCache;
  stepCache.resize(NUM_LOCOS, LocoStepCache());

  // These 2 vector are in sync.
  // Diff vector will be discarded in the end
  // Only entries are stored in the final table
  std::vector<Entry>& entries = table.mEntries;
  std::vector<double> diffVector;

  entries.reserve(200);
  diffVector.reserve(200);

  uint8_t firstLocoMaxStep = locoMappings.at(0).stepLowerBound(maxTrainSpeed);
  if(firstLocoMaxStep == 0)
    firstLocoMaxStep = 126;

  uint32_t currentLocoIdx = 0;

  bool beginNewRound = true;
  bool canCompareToLastInserted = false;

  while(stepCache[0].currentStep <= firstLocoMaxStep)
  {
    const VehicleSpeedCurve& mapping = locoMappings.at(currentLocoIdx);
    LocoStepCache& item = stepCache[currentLocoIdx];

    if(currentLocoIdx == 0)
    {
      item.currentStep++;
      item.currentSpeed = mapping.getSpeedForStep(item.currentStep);

      const double minAcceptedSpeed = item.currentSpeed - MAX_SPEED_DIFF;
      const double maxAcceptedSpeed = item.currentSpeed + MAX_SPEED_DIFF;
      item.minSpeedSoFar = item.maxSpeedSoFar = item.currentSpeed;

      for(uint32_t otherLocoIdx = 1; otherLocoIdx < locoMappings.size(); otherLocoIdx++)
      {
        const VehicleSpeedCurve& otherMapping = locoMappings.at(otherLocoIdx);
        LocoStepCache& otherItem = stepCache[otherLocoIdx];

        otherItem.minAcceptedStep = otherMapping.stepLowerBound(minAcceptedSpeed);
        otherItem.maxAcceptedStep = otherMapping.stepUpperBound(maxAcceptedSpeed);
        if(otherItem.minAcceptedStep == 0)
          otherItem.minAcceptedStep = 126;
        if(otherItem.maxAcceptedStep == 0)
          otherItem.maxAcceptedStep = 126;
      }

      // Go to next loco
      currentLocoIdx++;
      beginNewRound = true;

      // First locomotive step changed, so we do not compare
      // new entries with old ones. We leave duplication removal for later
      canCompareToLastInserted = false; // First loco step changed
      continue;
    }

    const LocoStepCache& prevItem = stepCache.at(currentLocoIdx - 1);

    if(beginNewRound)
    {
      item.currentStep = item.minAcceptedStep;
      beginNewRound = false;
    }
    else
    {
      item.currentStep++;
    }

    if(item.currentStep > item.maxAcceptedStep)
    {
      // Go up to previous loco
      currentLocoIdx--;
      continue;
    }

    item.minSpeedSoFar = prevItem.minSpeedSoFar;
    item.maxSpeedSoFar = prevItem.maxSpeedSoFar;
    item.currentSpeed = mapping.getSpeedForStep(item.currentStep);
    if(item.currentSpeed < item.minSpeedSoFar)
      item.minSpeedSoFar = item.currentSpeed;
    if(item.currentSpeed > item.maxSpeedSoFar)
      item.maxSpeedSoFar = item.currentSpeed;

    const double maxDiff = (item.maxSpeedSoFar - item.minSpeedSoFar);
    if(maxDiff > MAX_SPEED_DIFF)
    {
      // Go to next step
      continue;
    }

    if(currentLocoIdx < LAST_LOCO)
    {
      // Go to next loco
      currentLocoIdx++;
      beginNewRound = true;
      continue;
    }

    // We are last loco, save speed tuple
    Entry entry;
    entry.stepForLoco_.reset(new uint8_t[NUM_LOCOS]);

    double speedSum = 0;
    for(uint32_t locoIdx = 0; locoIdx < NUM_LOCOS; locoIdx++)
    {
      const LocoStepCache& otherItem = stepCache.at(locoIdx);

      entry.stepForLoco_[locoIdx] = otherItem.currentStep;
      speedSum += otherItem.currentSpeed;
    }

    entry.avgSpeed = speedSum / double(NUM_LOCOS);

    if(canCompareToLastInserted)
    {
      if(*diffVector.rbegin() > maxDiff)
      {
        // We are better than previous match, replace it

        std::memcpy(entries.rbegin()->stepForLoco_.get(),
                    entry.stepForLoco_.get(),
                    NUM_LOCOS * sizeof(uint8_t));
        entries.rbegin()->avgSpeed = entry.avgSpeed;

        *diffVector.rbegin() = maxDiff;
      }

      // If not better than previous, no point in addint it
      continue;
    }

    // First good match of new step combination
    entries.push_back(std::move(entry));
    diffVector.push_back(maxDiff);
    canCompareToLastInserted = true;
  }

  if(entries.empty())
    return table; // Error?

  // Remove duplicated steps, keep match with lower maxDiff
  for(uint32_t locoIdx = 0; locoIdx < NUM_LOCOS; locoIdx++)
  {
    const Entry& firstEntry = entries.at(0);
    double bestEntryDiff = diffVector.at(0);
    uint32_t bestEntryIdx = 0;
    uint32_t firstTableIdx = 0;
    uint32_t currentStep = firstEntry.stepForLoco_[locoIdx];

    for(uint32_t tableIdx = 1; tableIdx < entries.size(); tableIdx++)
    {
      uint8_t step = entries.at(tableIdx).stepForLoco_[locoIdx];
      if(step == currentStep)
      {
        const double maxDiff = diffVector.at(tableIdx);
        if(maxDiff < bestEntryDiff)
        {
          bestEntryIdx = tableIdx;
          bestEntryDiff = maxDiff;
        }
          continue;
      }

      // We reached next step
      // Cut all previous step entries, keep only best
      if(firstTableIdx < bestEntryIdx)
      {
        // Best entry is not erased
        entries.erase(entries.begin() + firstTableIdx,
                      entries.begin() + bestEntryIdx);
        diffVector.erase(diffVector.begin() + firstTableIdx,
                      diffVector.begin() + bestEntryIdx);
      }

      // Shift all indexes
      uint32_t idxShift = bestEntryIdx - firstTableIdx;
      bestEntryIdx = firstTableIdx;
      tableIdx -= idxShift;

      // Remove all after best entry up to last of this step
      uint32_t firstToErase = bestEntryIdx + 1;
      if(firstToErase < tableIdx)
      {
          // Best entry is not erased
          entries.erase(entries.begin() + firstToErase,
                        entries.begin() + tableIdx);
          diffVector.erase(diffVector.begin() + firstToErase,
                        diffVector.begin() + tableIdx);
      }

      // Now best entry is at first index, we are just after it
      tableIdx = bestEntryIdx + 1;

      // Re-init
      firstTableIdx = tableIdx;
      bestEntryIdx = firstTableIdx;
      currentStep = step;
      bestEntryDiff = diffVector.at(tableIdx);
    }

    if(firstTableIdx < (entries.size() - 1))
    {
      // Cut all last step entries, keep only best
      if(firstTableIdx < bestEntryIdx)
      {
        // Best entry is not erased
        entries.erase(entries.begin() + firstTableIdx,
                      entries.begin() + bestEntryIdx);
        diffVector.erase(diffVector.begin() + firstTableIdx,
                      diffVector.begin() + bestEntryIdx);
      }

      // Shift all indexes
      bestEntryIdx = firstTableIdx;

      // Remove all after best entry
      uint32_t firstToErase = bestEntryIdx + 1;
      entries.erase(entries.begin() + firstToErase,
                    entries.end());
      diffVector.erase(diffVector.begin() + firstToErase,
                    diffVector.end());
    }
  }

  // Save up some memory
  table.mEntries.shrink_to_fit();

  // Now table is guaranteed to have at most 126 entries
  // An thus can be indexed with uint8_t
  return table;
}
