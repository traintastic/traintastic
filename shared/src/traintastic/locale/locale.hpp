/**
 * shared/src/locale/locale.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_LOCALE_LOCALE_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_LOCALE_LOCALE_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <set>
#ifdef QT_CORE_LIB
  #include <QString>
#endif
#include "../utils/stdfilesystem.hpp"

class Locale
{
  protected:
    const std::unique_ptr<const Locale> m_fallback;
    std::string m_data;
    std::unordered_map<std::string_view, std::string_view> m_strings;
    std::unique_ptr<std::set<std::string>> m_missing;

  public:
    static std::unique_ptr<const Locale> instance;

#ifdef QT_CORE_LIB
    inline static QString tr(const QString& id) { return instance ? instance->translate(id) : id; };
#else
    inline static std::string_view tr(std::string_view id) { return instance ? instance->translate(id) : id; };
#endif

    const std::filesystem::path filename;

    Locale(std::filesystem::path _filename, std::unique_ptr<const Locale> fallback = {});

    const std::unique_ptr<const Locale>& fallback() const
    {
      return m_fallback;
    }

    const std::unique_ptr<std::set<std::string>>& missing() const { return m_missing; }
    void enableMissingLogging();

#ifdef QT_CORE_LIB
    QString translate(const QString& id) const;
    QString parse(const QString& text) const;
#else
    std::string_view translate(std::string_view id) const;
#endif
};


#endif
