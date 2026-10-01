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

#include "diagnosticreportdialog.hpp"

#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSettings>
#include <QFileDialog>
#include <QStandardPaths>
#include <QBuffer>
#include <QMainWindow>
#include <QScreen>
#include <QTimer>
#include <QDateTime>

#include <archive.h> // for: archive_version_details()

#include <version.hpp>
#include <traintastic/locale/locale.hpp>
#include <traintastic/os/systeminfo.hpp>

#include "../mainwindow.hpp"
#include "../network/callmethod.hpp"
#include "../utils/zipwriter.hpp"

DiagnosticReportDialog::DiagnosticReportDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f)
  , m_requestId{Connection::invalidRequestId}
{
  setWindowTitle(Locale::tr("qtapp.diagnostic_report_dialog:window_title"));

  auto* main = new QVBoxLayout();

  auto* explanation = new QLabel(Locale::tr("qtapp.diagnostic_report_dialog:explanation"), this);
  explanation->setWordWrap(true);

  main->addWidget(explanation);
  main->addStretch();

  auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  buttonBox->button(QDialogButtonBox::Ok)->setText(Locale::tr("qtapp.diagnostic_report_dialog:create_report"));
  connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
    [this, buttonBox]()
    {
      buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
      nextState();
    });
  buttonBox->button(QDialogButtonBox::Cancel)->setText(Locale::tr("qtapp.dialog:cancel"));
  connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &DiagnosticReportDialog::reject);

  main->addWidget(buttonBox);

  setLayout(main);
}

DiagnosticReportDialog::~DiagnosticReportDialog()
{
  if(m_requestId != Connection::invalidRequestId)
  {
    if(auto* mainWindow = qobject_cast<MainWindow*>(parent())) [[likely]]
    {
      if(auto connection = mainWindow->connection())
      {
        connection->cancelRequest(m_requestId);
      }
    }
  }
}

void DiagnosticReportDialog::nextState()
{
  assert(m_state != State::Done);

  m_state = static_cast<State>(static_cast<std::underlying_type_t<State>>(m_state) + 1);

  switch(m_state)
  {
    case State::WaitingForStart: [[unlikely]]
      assert(false);
      break;

    case State::GetClientInfo:
    {
      m_clientInfo = "### Traintastic Client ###\nVersion: " TRAINTASTIC_VERSION_FULL "\n\n";
      m_clientInfo.append(QString::fromStdString(getSystemInfo()));
      m_clientInfo.append(QString("\n### Libraries ###\nQt: %1 (compiled: " QT_VERSION_STR ")\nlibarchive: %2\n").arg(qVersion()).arg(archive_version_details()));
      nextState();
      break;
    }
    case State::GetClientScreenshot:
    {
      if(auto* mainWindow = qobject_cast<QMainWindow*>(parent())) [[likely]]
      {
        hide();
        QTimer::singleShot(500, // ms (we need to wait a bit for desktop effects etc.)
          [this, mainWindow]()
          {
            m_clientScreenshot = mainWindow->screen()->grabWindow(0).copy(mainWindow->frameGeometry());
            show();
            nextState();
          });
      }
      else
      {
        nextState();
      }
      break;
    }
    case State::GetServerDiagnosticReport:
      if(auto* mainWindow = qobject_cast<MainWindow*>(parent())) [[likely]]
      {
        if(auto connection = mainWindow->connection())
        {
          m_requestId = connection->getServerDiagnosticReport(
            [this](const QString& info, const QString& log, const QByteArray& world)
            {
              m_requestId = Connection::invalidRequestId;
              m_serverInfo = info;
              m_serverLog = log;
              m_worldExport = world;
              nextState();
            });
          return;
        }
      }
      nextState();
      break;

    case State::Done:
    {
      QSettings settings;
      settings.beginGroup("diagnostic_report");
      const QString pathKey{"path"};

      QString filename =
        settings.value(pathKey, QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).toString()
          .append(QDir::separator())
          .append("traintastic_diagnostic_report_")
          .append(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"))
          .append(".zip");

      filename =
        QFileDialog::getSaveFileName(
          this,
          Locale::tr("qtapp.diagnostic_report_dialog:save_diagnostic_report"),
          filename,
          Locale::tr("qtapp.diagnostic_report_dialog:zip_file").append(" (*.zip)"));

      if(!filename.isEmpty())
      {
        settings.setValue(pathKey, QFileInfo(filename).absolutePath());
        save(filename);
        accept();
      }
      else
      {
        reject();
      }
      break;
    }
  }
}

void DiagnosticReportDialog::save(const QString& filename)
{
  ZipWriter zip(filename);

  if(!m_clientInfo.isEmpty())
  {
    zip.addFile("traintastic-client-info.txt", m_clientInfo.toUtf8());
  }

  if(!m_clientScreenshot.isNull())
  {
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    if(m_clientScreenshot.save(&buffer, "PNG"))
    {
      zip.addFile("traintastic-client.png", png);
    }
  }

  if(!m_serverInfo.isEmpty())
  {
    zip.addFile("traintastic-server-info.txt", m_serverInfo.toUtf8());
  }

  if(!m_serverLog.isEmpty())
  {
    zip.addFile("traintastic-server-log.txt", m_serverLog.toUtf8());
  }

  if(!m_worldExport.isEmpty())
  {
    zip.addFile("world.ctw", m_worldExport);
  }
}
