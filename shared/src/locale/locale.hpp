/**
 * shared/src/locale/locale.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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



#ifndef TRAINTASTIC_SHARED_LOCALE_LOCALE_HPP
#define TRAINTASTIC_SHARED_LOCALE_LOCALE_HPP

#include <string>
#include <unordered_map>
#ifdef QT_CORE_LIB
  #include <QString>
#endif

class Locale
{
  protected:
    const Locale* m_fallback;
    std::string m_data;
    std::unordered_map<std::string_view, std::string_view> m_strings;

  public:
    static const Locale* instance;
#ifdef QT_CORE_LIB
    inline static QString tr(const QString& id) { return instance ? instance->translate(id) : id; };
#else
    inline static std::string_view tr(std::string_view id) { return instance ? instance->translate(id) : id; };
#endif

    Locale(std::string_view filename, Locale* fallback = nullptr);

#ifdef QT_CORE_LIB
    QString translate(const QString& id) const;
#else
    std::string_view translate(std::string_view id) const;
#endif
};


#endif
