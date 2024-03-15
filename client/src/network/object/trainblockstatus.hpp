/**
 * client/src/network/object/trainblockstatus.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_OBJECT_TRAINBLOCKSTATUS_HPP
#define TRAINTASTIC_CLIENT_NETWORK_OBJECT_TRAINBLOCKSTATUS_HPP

#include "../object.hpp"
#include "../objectptr.hpp"
#include <traintastic/enum/blocktraindirection.hpp>

class Property;
class ObjectProperty;

class TrainBlockStatus final : public Object
{
  Q_OBJECT

  CLASS_ID("train_block_status")

  private:
    int m_requestId;
    Property* m_directionProperty = nullptr;
    Property* m_identificationProperty = nullptr;
    ObjectProperty* m_trainProperty = nullptr;
    ObjectPtr m_train;

    void updateTrain();

  protected:
    void created() final;

  public:
    TrainBlockStatus(const std::shared_ptr<Connection>& connection, Handle handle, const QString& classId_);
    ~TrainBlockStatus() final;

    BlockTrainDirection direction() const;
    QString identification() const;

    const ObjectPtr& train() const
    {
      return m_train;
    }

  signals:
    void changed();
};

#endif
