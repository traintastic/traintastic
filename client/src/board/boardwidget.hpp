/**
 * client/src/board/boardwidget.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020,2022-2023 Reinder Feenstra
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
#include "../network/board.hpp"

class QToolBar;
class QToolButton;
class QActionGroup;
class QStatusBar;
class QLabel;
struct TileInfo;
class NXButtonRailTile;

class BoardWidget : public QWidget
{
  Q_OBJECT

  private:
    static constexpr unsigned int nxButtonHoldTime = 3000; // ms
    static constexpr unsigned int nxButtonReleaseDelay = 200; // ms

  protected:
    std::shared_ptr<Board> m_object;
    std::shared_ptr<Object> m_nxManager;
    int m_nxManagerRequestId;
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
    bool m_tileMoveStarted;
    int16_t m_tileMoveX;
    int16_t m_tileMoveY;
    bool m_tileResizeStarted;
    int16_t m_tileResizeX;
    int16_t m_tileResizeY;
    uint8_t m_tileRotates = 0; //!< allowed rotate for add/move tile
    TileRotate m_tileRotateLast = TileRotate::Deg0; //!< Last used tile rotate for add/move
    std::weak_ptr<NXButtonRailTile> m_nxButtonPressed;

    void actionSelected(const Board::TileInfo* tile);
    void keyPressEvent(QKeyEvent* event) override;
    void rotateTile(bool ccw = false);
    void releaseNXButton(const std::shared_ptr<NXButtonRailTile>& nxButton);

  protected slots:
    void worldEditChanged(bool value);
    void gridChanged(BoardAreaWidget::Grid value);
    void zoomLevelChanged(int value);
    void tileClicked(int16_t x, int16_t y);
    void rightClicked();

  public:
    BoardWidget(std::shared_ptr<Board> object, QWidget* parent = nullptr);
    ~BoardWidget() override;

    Board& board() { return *m_object; }
};

#endif
