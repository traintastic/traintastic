/**
 * client/src/mdiarea.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#include "mdiarea.hpp"
#include <QPushButton>
#include <QAction>
#include <QResizeEvent>
#include <QPainter>
#include <QSvgRenderer>

MdiArea::MdiArea(QWidget* parent) :
  QMdiArea(parent),
  m_backgroundImage{new QSvgRenderer(QStringLiteral(":/backgroundimage.svg"), this)}
{
}

void MdiArea::addBackgroundAction(QAction* action)
{
  QPushButton* button = new QPushButton(action->icon(), action->text(), this);
  connect(button, &QPushButton::clicked, action, &QAction::trigger);
  button->setMinimumSize(100, 100);
  button->show();
  m_backgroundActionButtons.emplace_back(action, button);

  updateButtonPositions();
}

void MdiArea::removeBackgroundAction(QAction* action)
{
  auto it = std::find_if(m_backgroundActionButtons.begin(), m_backgroundActionButtons.end(), [action](const auto& i) { return i.first == action; });
  if(it != m_backgroundActionButtons.end())
  {
    delete it->second;
    m_backgroundActionButtons.erase(it);
    updateButtonPositions();
  }
}

void MdiArea::paintEvent(QPaintEvent* event)
{
  QMdiArea::paintEvent(event);

  // paint Traintastic logo on background:
  const QSize sz = viewport()->size();
  const qreal n = qMax(64., qMin(sz.width(), sz.height()) / 1.5);

  QPainter painter(viewport());
  m_backgroundImage->render(&painter, QRectF((sz.width() - n) / 2, (sz.height() - n) / 2, n, n));
}

void MdiArea::resizeEvent(QResizeEvent* event)
{
  QMdiArea::resizeEvent(event);
  if(event->size().width() != event->oldSize().width())
    updateButtonPositions();
}

void MdiArea::updateButtonPositions()
{
  const int buttonMargin = 20;

  int totalWidth = -buttonMargin;
  for(const auto& it : m_backgroundActionButtons)
    totalWidth += it.second->width() + buttonMargin;

  int x = (width() - totalWidth) / 2;
  for(const auto& it : m_backgroundActionButtons)
  {
    it.second->move(x, buttonMargin);
    x += it.second->width() + buttonMargin;
  }
}
