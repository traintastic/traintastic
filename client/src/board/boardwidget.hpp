/**
 * client/src/board/boardwidget.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_BOARD_BOARDWIDGET_HPP
#define TRAINTASTIC_CLIENT_BOARD_BOARDWIDGET_HPP

#include <QWidget>
#include <memory>
#include <traintastic/enum/tilerotate.hpp>
#include "boardareawidget.hpp"

class Board;
class QToolBar;
class QToolButton;
class QActionGroup;
class QStatusBar;
class QLabel;
struct TileInfo;

class BoardWidget : public QWidget
{
  Q_OBJECT

  protected:
    std::shared_ptr<Board> m_object;
    BoardAreaWidget* m_boardArea;
    QStatusBar* m_statusBar;
    QLabel* m_statusBarMessage;
    QLabel* m_statusBarCoords;
    QLabel* m_statusBarZoom;
    QAction* m_actionZoomIn;
    QAction* m_actionZoomOut;
    QToolButton* m_toolButtonGrid;
    QAction* m_actionGridNone;
    QAction* m_actionGridDot;
    QAction* m_actionGridLine;
    QToolBar* m_toolbarEdit;
    QVector<QAction*> m_addActions; // all tile add actions
    QActionGroup* m_editActions;
    QAction* m_editActionNone;
    QAction* m_editActionMove;
    QAction* m_editActionResize;
    QAction* m_editActionDelete;
    QAction* m_editActionResizeToContents;
    TileRotate m_editRotate;
    bool m_tileMoveStarted;
    int16_t m_tileMoveX;
    int16_t m_tileMoveY;
    bool m_tileResizeStarted;
    int16_t m_tileResizeX;
    int16_t m_tileResizeY;

    void actionSelected(const TileInfo* tile);
    void keyPressEvent(QKeyEvent* event) override;

  protected slots:
    void worldEditChanged(bool value);
    void gridChanged(BoardAreaWidget::Grid value);
    void zoomLevelChanged(int value);
    void tileClicked(int16_t x, int16_t y);
    void rightClicked();

  public:
    BoardWidget(std::shared_ptr<Board> object, QWidget* parent = nullptr);

    Board& board() { return *m_object; }
};

#endif
