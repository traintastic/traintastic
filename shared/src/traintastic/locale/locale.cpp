/**
 * shared/src/locale/locale.cpp
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

#include "locale.hpp"
#include <fstream>
#ifdef QT_CORE_LIB
  #include <QDebug>
  #include <QRegularExpression>
#endif

std::unique_ptr<const Locale> Locale::instance;

Locale::Locale(std::filesystem::path _filename, std::unique_ptr<const Locale> fallback)
  : m_fallback{std::move(fallback)}
  , filename{std::move(_filename)}
{
  // read file into string:
  std::ifstream file{filename, std::ios::binary | std::ios::ate};
  if(file.is_open())
  {
    auto size = file.tellg();
    m_data.resize(size, '\0');
    file.seekg(0);
    file.read(m_data.data(), size);

    const char* p = m_data.data();
    const char* pEnd = m_data.data() + m_data.size();
    while(p < pEnd)
    {
      size_t len = *reinterpret_cast<const uint32_t*>(p);
      p += sizeof(uint32_t);
      std::string_view id{p, len};
      p += len;
      if(len % 4 != 0)
        p += 4 - (len % 4);

      len = *reinterpret_cast<const uint32_t*>(p);
      p += sizeof(uint32_t);
      m_strings.insert({id, {p, len}});
      p += len;
      if(len % 4 != 0)
        p += 4 - (len % 4);
    }
  }
}

void Locale::enableMissingLogging()
{
  if(!m_missing)
    m_missing = std::make_unique<std::set<std::string>>();
}

#ifdef QT_CORE_LIB
QString Locale::translate(const QString& id) const
{
  QByteArray b = id.toLatin1();
  auto it = m_strings.find({b.data(), static_cast<std::string_view::size_type>(b.length())});
  if(it != m_strings.cend())
    return QString::fromUtf8(it->second.data(), static_cast<int>(it->second.size()));
  else if(m_missing)
    m_missing->emplace(id.toStdString());

  if(m_fallback)
    return m_fallback->translate(id);

  qWarning() << "Locale: Missing translation for" << id;
  return id;
}

QString Locale::parse(const QString& text) const
{
  static QRegularExpression re("\\$([a-z0-9:\\._]+)\\$");
  QRegularExpressionMatch m = re.match(text);
  if(m.hasMatch())
    return QString(text).replace(m.capturedStart(), m.capturedLength(), translate(m.captured(1)));
  else
    return text;
}
#else
std::string_view Locale::translate(std::string_view id) const
{
  auto it = m_strings.find(id);
  if(it != m_strings.cend())
    return it->second;
  if(m_missing)
    m_missing->emplace(id);

  if(m_fallback)
    return m_fallback->translate(id);

  return id;
}
#endif
