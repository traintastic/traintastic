/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_DIALOG_SCREENSHOTDIALOG_HPP
#define TRAINTASTIC_CLIENT_DIALOG_SCREENSHOTDIALOG_HPP

#include <QDialog>
#include <QDir>
#include <QStack>

class QLabel;
class QProgressBar;
class QPushButton;
class QMdiSubWindow;
class MainWindow;

class ScreenShotDialog final : public QDialog
{
public:
  explicit ScreenShotDialog(MainWindow& mainWindow);

protected:
  void timerEvent(QTimerEvent *event) override;

private:
  enum class Step : uint
  {
    Start, // keep first!

    ConnectToServer,
    CloseWorld,
    NewWorld,

    NewWorldWizard,
    NewWorldWizardSetWorldName,
    NewWorldWizardSetWorldNameShoot,
    NewWorldWizardSetWorldScale,
    NewWorldWizardFinish,

    InterfaceListEmpty,

    LuaScriptListEmpty,
    LuaScriptEditor,

    Done // keep last!
  };

  MainWindow& m_mainWindow;
  QProgressBar* m_progressBar;
  QLabel* m_label;
  QPushButton* m_start;
  const QString m_languageCode;
  QDir m_outputDir;
  Step m_step = Step::Start;
  QStack<QMdiSubWindow*> m_dialogsToClose;
  int m_stepTimer = 0;

  void step();
  void next();

  void savePixmap(QPixmap pixmap, const QString& filename);
  void saveWidgetImage(QWidget* widget, const QString& filename);
  void saveMainWindowImage(const QString& filename);

  static QString getStepLabel(Step step);
};

#endif
