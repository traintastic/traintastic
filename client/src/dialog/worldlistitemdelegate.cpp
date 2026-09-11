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

#include "worldlistitemdelegate.hpp"
#include <QApplication>
#include <QListView>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>
#include <traintastic/locale/locale.hpp>
#include "../theme/theme.hpp"

namespace {
  const QSize actionIconSize{24, 24};
  constexpr int actionIconMargin = 5;
}

WorldListItemDelegate::WorldListItemDelegate(QListView* parent)
  : QItemDelegate(parent)
{
}

void WorldListItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  static const QSize iconSize{40, 40};

  auto* model = qobject_cast<QListView*>(parent())->model();
  const auto name = model->data(model->index(index.row(), 0)).toString();
  const auto uuid = model->data(model->index(index.row(), 1)).toString();

  const auto r = option.rect.adjusted(5, 5, -5, -5);
  const int iconOffset = (r.height() - iconSize.height()) / 2;

  const auto palette = QApplication::palette();

  if((option.state & QStyle::State_Selected) != 0)
  {
    painter->fillRect(option.rect, palette.brush(QPalette::Highlight));
  }

  QTextOption textOption;
  textOption.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

  const auto classIcon = Theme::getIcon("world");
  painter->drawPixmap(r.topLeft() + QPoint(iconOffset, iconOffset), classIcon.pixmap(iconSize));

  painter->setPen(palette.color(QPalette::Disabled, QPalette::Text));
  painter->drawText(r.adjusted(r.height() + 10, r.height() / 2, 0, 0), uuid, textOption);

  painter->setPen(palette.color(QPalette::Active, QPalette::Text));
  painter->drawText(r.adjusted(r.height() + 10, 0, 0, -r.height() / 2), name, textOption);

  painter->setPen(QColor(0x80, 0x80, 0x80, 0x30));
  painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());

  for(auto action : actions)
  {
    painter->drawPixmap(actionIconRect(action, option), actionIcon(action).pixmap(actionIconSize));
  }
}

QSize WorldListItemDelegate::sizeHint(const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
  return QSize(-1, 50);
}

bool WorldListItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
  if(event->type() == QEvent::MouseMove)
  {
    auto* mouseEvent = static_cast<QMouseEvent*>(event);
    auto* widget = const_cast<QWidget*>(option.widget);

    bool inRect = false;
    for(auto action : actions)
    {
      if((inRect = actionIconRect(action, option).contains(mouseEvent->pos())))
      {
        break;
      }
    }

    if(inRect)
    {
      widget->setCursor(Qt::PointingHandCursor);
    }
    else
    {
      widget->unsetCursor();
    }
  }
  else if(event->type() == QEvent::MouseButtonRelease)
  {
    auto* mouseEvent = static_cast<QMouseEvent*>(event);

    if(actionIconRect(Action::Delete, option).contains(mouseEvent->pos()))
    {
      emit deleteClicked(index);
      return true;
    }
    if(actionIconRect(Action::Duplicate, option).contains(mouseEvent->pos()))
    {
      emit duplicateClicked(index);
      return true;
    }
  }

  return QItemDelegate::editorEvent(event, model, option, index);
}

bool WorldListItemDelegate::helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index)
{
  for(auto action : actions)
  {
    if(actionIconRect(action, option).contains(event->pos()))
    {
      QToolTip::showText(event->globalPos(), actionToolTip(action));
      return true;
    }
  }
  return QItemDelegate::helpEvent(event, view, option, index);
}

QRect WorldListItemDelegate::actionIconRect(Action action, const QStyleOptionViewItem& option) const
{
  return QRect(
    {
      option.rect.x() + option.rect.width() - (static_cast<int>(action) * (actionIconSize.width() + actionIconMargin)),
      option.rect.y() + (option.rect.height() - actionIconSize.height()) / 2
    }, actionIconSize);
}

QIcon WorldListItemDelegate::actionIcon(Action action) const
{
  switch(action)
  {
    case Action::Delete:
      return Theme::getIcon("delete");

    case Action::Duplicate:
      return Theme::getIcon("world_duplicate");
  }
  return {};
}

QString WorldListItemDelegate::actionToolTip(Action action) const
{
  switch(action)
  {
    case Action::Delete:
      return Locale::tr("qtapp.world_list_dialog:delete_world");

    case Action::Duplicate:
      return Locale::tr("qtapp.world_list_dialog:duplicate_world");
  }
  return {};
}
