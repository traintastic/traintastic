/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2019-2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_DIALOG_WORLDLISTITEMDELEGATE_HPP
#define TRAINTASTIC_CLIENT_DIALOG_WORLDLISTITEMDELEGATE_HPP

#include <array>
#include <QItemDelegate>

class QListView;

class WorldListItemDelegate final : public QItemDelegate
{
  Q_OBJECT

public:

  WorldListItemDelegate(QListView* parent);

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const final;
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const final;
  bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) final;
  bool helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index) final;

signals:
  void deleteClicked(const QModelIndex& index);
  void duplicateClicked(const QModelIndex& index);

private:
  enum class Action
  {
    Delete = 1,
    Duplicate = 2,
  };

  static constexpr std::array<WorldListItemDelegate::Action, 2> actions{{
    WorldListItemDelegate::Action::Delete,
    WorldListItemDelegate::Action::Duplicate,
  }};

  QRect actionIconRect(Action action, const QStyleOptionViewItem& option) const;
  QIcon actionIcon(Action action) const;
  QString actionToolTip(Action action) const;
};

#endif
