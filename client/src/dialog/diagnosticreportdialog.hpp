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

#ifndef TRAINTASTIC_CLIENT_DIALOG_DIAGNOSTICREPORTDIALOG_HPP
#define TRAINTASTIC_CLIENT_DIALOG_DIAGNOSTICREPORTDIALOG_HPP

#include <QDialog>

class DiagnosticReportDialog : public QDialog
{
public:
  explicit DiagnosticReportDialog(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  ~DiagnosticReportDialog() override;

private:
  enum class State
  {
    WaitingForStart, // first

    GetClientInfo,
    GetClientScreenshot,
    GetServerDiagnosticReport,

    Done // last
  };

  State m_state = State::WaitingForStart;
  int m_requestId;

  QString m_clientInfo;
  QPixmap m_clientScreenshot;
  QString m_serverInfo;
  QString m_serverLog;
  QByteArray m_worldExport;

  void nextState();

  void save(const QString& filename);
};

#endif
