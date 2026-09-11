/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "zipwriter.hpp"
#include <archive.h>
#include <archive_entry.h>
#include <QDateTime>

ZipWriter::ZipWriter(const QString& filename)
  : m_archive{archive_write_new(),
    [](archive* a)
    {
      archive_write_close(a);
      archive_write_free(a);
    }}
{
  const auto str = filename.toStdString();
  if(archive_write_set_format_zip(m_archive.get()) != ARCHIVE_OK ||
      archive_write_open_filename(m_archive.get(), str.c_str()) != ARCHIVE_OK)
  {
    m_archive.reset();
  }
}

ZipWriter::~ZipWriter() = default;

bool ZipWriter::addFile(const QString& filename, const QByteArray& contents)
{
  if(!m_archive)
  {
    return false;
  }
  const auto str = filename.toStdString();
  auto* entry = archive_entry_new();
  archive_entry_set_pathname(entry, str.c_str());
  archive_entry_set_size(entry, contents.size());
  archive_entry_set_filetype(entry, AE_IFREG);
  archive_entry_set_perm(entry, 0644);
  archive_entry_set_birthtime(entry, QDateTime::currentSecsSinceEpoch(), 0);
  archive_entry_set_ctime(entry, QDateTime::currentSecsSinceEpoch(), 0);
  if(archive_write_header(m_archive.get(), entry) != ARCHIVE_OK)
  {
    archive_entry_free(entry);
    return false;
  }
  archive_write_data(m_archive.get(), contents.data(), contents.size());
  archive_entry_free(entry);
  return true;
}
