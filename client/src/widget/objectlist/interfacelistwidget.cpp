/**
 * client/src/widget/objectlist/interfacelistwidget.cpp
 *
 * This file is part of the traintastic source code.
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

#include "interfacelistwidget.hpp"

#include <QApplication>
#include <QItemDelegate>
#include <QListView>
#include <QMenu>
#include <QPainter>
#include <QtMath>

#include <traintastic/enum/interfacestate.hpp>
#include <traintastic/locale/locale.hpp>

#include "../methodicon.hpp"
#include "../../theme/theme.hpp"
#include "../../mainwindow.hpp"

class InterfaceListItemDelegate : public QItemDelegate
{
public:
  inline InterfaceListItemDelegate(QListView* parent)
    : QItemDelegate(parent)
  {
  }

  inline void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const final
  {
    static const QSize classIconSize{40, 40};
    static const QSize stateIconSize{16, 16};

    auto* model = qobject_cast<QListView*>(parent())->model();
    const auto id = model->data(model->index(index.row(), 0)).toString().prepend('#');
    const auto name = model->data(model->index(index.row(), 1)).toString();
    const auto state = model->data(model->index(index.row(), 2)).toString();
    const auto classId = model->data(model->index(index.row(), 3)).toString();

    const auto r = option.rect.adjusted(5, 5, -5, -5);
    const int iconOffset = (r.height() - classIconSize.height()) / 2;

    const auto palette = QApplication::palette();

    QTextOption textOption;
    textOption.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    const auto classIcon = Theme::getIconForClassId(classId);
    if(!classIcon.isNull())
    {
      painter->drawPixmap(r.topLeft() + QPoint(iconOffset, iconOffset), classIcon.pixmap(classIconSize));
    }

    painter->setPen(palette.color(QPalette::Disabled, QPalette::Text));
    painter->drawText(r.adjusted((classIcon.isNull() ? 0 : r.height() + 10), r.height() / 2, 0, 0), id, textOption);

    painter->setPen(palette.color(QPalette::Active, QPalette::Text));
    painter->drawText(r.adjusted((classIcon.isNull() ? 0 : r.height() + 10), 0, 0, -r.height() / 2), name, textOption);

    textOption.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    const auto stateText = Locale::tr(QString(EnumName<InterfaceState>::value).append(":").append(state));
    const auto stateRect = r.adjusted(0, r.height() / 2, 0, 0);
    const auto stateTextWidth = qCeil(painter->boundingRect(stateRect, stateText, textOption).width());
    painter->drawText(stateRect, stateText, textOption);

    auto stateIcon = QIcon(Theme::getIconFile(QString("interface_state.").append(state)));
    painter->drawPixmap(
      stateRect.topRight() - QPoint(stateTextWidth + stateIconSize.width() + 5, -(stateRect.height() - stateIconSize.height()) / 2),
      stateIcon.pixmap(stateIconSize));

    painter->setPen(QColor(0x80, 0x80, 0x80, 0x30));
    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
  }

  inline QSize sizeHint(const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const final
  {
    return QSize(-1, 50);
  }
};

InterfaceListWidget::InterfaceListWidget(const ObjectPtr& object, QWidget* parent)
  : StackedObjectListWidget(object, parent)
{
  m_list->setItemDelegate(new InterfaceListItemDelegate(m_list));
  m_listEmptyLabel->setText(Locale::tr("interface_list:list_is_empty"));

  if(m_create) /*[[likely]]*/
  {
    m_create->setToolTip(Locale::tr("interface_list:create"));
  }

  if(m_createMenu) /*[[likely]]*/
  {
    m_createMenu->addSeparator();
    m_createMenu->addAction(Theme::getIcon("wizard"), Locale::tr("list:setup_using_wizard") + "...",
      []()
      {
        MainWindow::instance->showAddInterfaceWizard();
      });
  }
}
