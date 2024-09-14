/**
 * server/src/train/trainspeedtable.hpp
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

#ifndef TRAINTASTIC_SERVER_TRAIN_TRAINSPEEDTABLE_HPP
#define TRAINTASTIC_SERVER_TRAIN_TRAINSPEEDTABLE_HPP

#include <memory>
#include <vector>

class VehicleSpeedCurve;

class TrainSpeedTable
{
public:
  static constexpr uint8_t NULL_TABLE_ENTRY = 0;

  struct Entry
  {
    Entry() = default;

    // Disable copy from lvalue.
    Entry(const Entry&) = delete;
    Entry& operator=(const Entry&) = delete;

    // Move contructor
    // NOTE: needed to store std::unique_ptr in QVector
    Entry(Entry &&other)
    {
      stepForLoco_ = std::move(other.stepForLoco_);
      avgSpeed = std::move(other.avgSpeed);
    }

    Entry&
    operator=(Entry&& other)
    {
      stepForLoco_ = std::move(other.stepForLoco_);
      avgSpeed = std::move(other.avgSpeed);
      return *this;
    }

    std::unique_ptr<uint8_t[]> stepForLoco_;
    double avgSpeed = 0;

    inline uint8_t getStepForLoco(uint32_t locoIdx) const
    {
      if(stepForLoco_)
        return stepForLoco_[locoIdx];
      return 0;
    }
  };

  struct ClosestMatchRet
  {
    ClosestMatchRet(const Entry& entry, size_t tableIdx_)
      : tableEntry(entry), tableIdx(uint8_t(tableIdx_))
    {
    }

    ClosestMatchRet(const Entry& entry, int tableIdx_)
      : tableEntry(entry), tableIdx(uint8_t(tableIdx_))
    {
    }

    // Entry is non-copyable
    const Entry& tableEntry;
    uint8_t tableIdx;
  };

  TrainSpeedTable();

  ClosestMatchRet getClosestMatch(uint32_t locoIdx, uint8_t step) const;

  ClosestMatchRet getClosestMatch(double speed) const;

  inline uint8_t count() const
  {
    // Add 1 for null entry (not stored)
    return uint8_t(mEntries.size() + 1);
  }

  const Entry& getEntryAt(uint8_t tableIdx) const;

  static TrainSpeedTable buildTable(const std::vector<std::shared_ptr<VehicleSpeedCurve> > &locoMappings);

private:
  std::vector<Entry> mEntries;
  uint32_t locoCount = 0;

  static Entry nullEntry;
};

#endif // TRAINTASTIC_SERVER_TRAIN_TRAINSPEEDTABLE_HPP
