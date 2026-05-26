/**
 * client/src/dialog/worldlistdialog.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2023,2025 Reinder Feenstra
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

#include "worldlistdialog.hpp"
#include <QVBoxLayout>
#include <QTableView>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLineEdit>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QItemDelegate>
#include <QPainter>
#include <QApplication>
#include <QtWaitingSpinner/waitingspinnerwidget.h>
#include "../network/connection.hpp"
#include "../network/object.hpp"
#include "../network/tablemodel.hpp"
#include "../network/error.hpp"
#include "../theme/theme.hpp"
#include "../widget/alertwidget.hpp"
#include <traintastic/locale/locale.hpp>

constexpr int columnUUID = 1;

class WorldListItemDelegate : public QItemDelegate
{
public:
  inline WorldListItemDelegate(QListView* parent)
    : QItemDelegate(parent)
  {
  }

  inline void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const final
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
  }

  inline QSize sizeHint(const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const final
  {
    return QSize(-1, 50);
  }
};

WorldListDialog::WorldListDialog(std::shared_ptr<Connection> connection, QWidget* parent) :
  QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
  m_connection{std::move(connection)},
  m_buttons{new QDialogButtonBox(this)},
  m_search{new QLineEdit(this)},
  m_list{new QListView(this)}
{
  setWindowTitle(Locale::tr("qtapp.world_list_dialog:world_list"));
  setWindowIcon(Theme::getIcon("world_load"));
  resize(500, 400);

  m_search->setPlaceholderText(Locale::tr("qtapp.world_list_dialog:search"));

  m_list->setItemDelegate(new WorldListItemDelegate(m_list));

  m_buttons->setStandardButtons(QDialogButtonBox::Open | QDialogButtonBox::Cancel);
  m_buttons->button(QDialogButtonBox::Open)->setText(Locale::tr("qtapp.world_list_dialog:load"));
  m_buttons->button(QDialogButtonBox::Open)->setEnabled(false);
  connect(m_buttons->button(QDialogButtonBox::Open), &QPushButton::clicked, this,
    [this]()
    {
      m_uuid = m_list->model()->data(m_list->model()->index(m_list->selectionModel()->selectedIndexes().first().row(), columnUUID)).toString();
      accept();
    });
  m_buttons->button(QDialogButtonBox::Cancel)->setText(Locale::tr("qtapp.world_list_dialog:cancel"));
  connect(m_buttons->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &WorldListDialog::reject);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->addWidget(m_search);
  layout->addWidget(m_list);
  layout->addWidget(m_buttons);
  setLayout(layout);

  auto* spinner = new WaitingSpinnerWidget(this, true, false);
  spinner->start();

  m_requestId = m_connection->getObject("traintastic.world_list",
    [this, spinner](const ObjectPtr& object, std::optional<const Error> error)
    {
      if(object)
      {
        m_object = object;

        m_requestId = m_connection->getTableModel(m_object,
          [this, spinner](const TableModelPtr& tableModel, std::optional<const Error> err)
          {
            if(tableModel)
            {
              m_requestId = Connection::invalidRequestId;

              m_tableModel = tableModel;
              m_tableModel->setRegionAll(true);
              auto* filter = new QSortFilterProxyModel(this);
              filter->setSourceModel(m_tableModel.get());
              filter->setFilterCaseSensitivity(Qt::CaseInsensitive);
              connect(m_search, &QLineEdit::textChanged, filter, &QSortFilterProxyModel::setFilterFixedString);
              filter->setFilterFixedString(m_search->text());
              m_list->setModel(filter);
              m_list->setSelectionBehavior(QAbstractItemView::SelectRows);
              connect(m_list->selectionModel(), &QItemSelectionModel::selectionChanged, this,
                [this](const QItemSelection&, const QItemSelection&)
                {
                  m_buttons->button(QDialogButtonBox::Open)->setEnabled(m_list->selectionModel()->selectedRows().count() == 1);
                });
              connect(m_list, &QListView::doubleClicked, this,
                [this](const QModelIndex& index)
                {
                  m_uuid = m_list->model()->data(m_list->model()->index(index.row(), columnUUID)).toString();
                  accept();
                });
              delete spinner;
            }
            else if(err)
            {
              static_cast<QVBoxLayout*>(this->layout())->insertWidget(0, AlertWidget::error(err->toString()));
            }
            else
            {
              assert(false);
            }
          });
      }
      else if(error)
      {
        static_cast<QVBoxLayout*>(this->layout())->insertWidget(0, AlertWidget::error(error->toString()));
      }
      else
      {
        assert(false);
      }
    });
}

WorldListDialog::~WorldListDialog()
{
  m_connection->cancelRequest(m_requestId);
}
