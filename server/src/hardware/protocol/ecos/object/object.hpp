/**
 * server/src/hardware/protocol/ecos/object/object.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_OBJECT_OBJECT_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_OBJECT_OBJECT_HPP

#include <cstdint>
#include <memory>
#include <vector>
#include <string_view>

namespace ECoS {

class Kernel;
struct Reply;
struct Event;

class Object
{
  private:
    void update(const std::vector<std::string_view>& lines);

  protected:
    Kernel& m_kernel;
    const uint16_t m_id;
    bool m_isViewActive = false;

    void send(std::string_view message);

    bool objectExists(uint16_t objectId) const;
    void addObject(std::unique_ptr<Object> object);
    void nameChanged();
    void removeObject(uint16_t objectId);

    virtual void update(std::string_view /*option*/, std::string_view /*value*/){}// = 0;

  public:
    Object(const Object&) = delete;
    Object& operator =(const Object&) = delete;

    Object(Kernel& kernel, uint16_t id);
    virtual ~Object() = default;

    virtual bool receiveReply(const Reply& reply);
    virtual bool receiveEvent(const Event& event);

    inline uint16_t id() const { return m_id; }
    inline bool isViewActive() const { return m_isViewActive; }

    void requestView();
};

}

#endif
