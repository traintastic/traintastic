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
#include <QMessageBox>
#include <QtWaitingSpinner/waitingspinnerwidget.h>
#include "worldlistitemdelegate.hpp"
#include "../network/callmethod.hpp"
#include "../network/connection.hpp"
#include "../network/object.hpp"
#include "../network/tablemodel.hpp"
#include "../network/error.hpp"
#include "../theme/theme.hpp"
#include "../widget/alertwidget.hpp"
#include <traintastic/locale/locale.hpp>

constexpr int columnName = 0;
constexpr int columnUUID = 1;

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

  auto* itemDelegate = new WorldListItemDelegate(m_list);
  connect(itemDelegate, &WorldListItemDelegate::duplicateClicked,
    [this](const QModelIndex& index)
    {
      if(m_object) [[likely]]
      {
        if(auto* duplicate = m_object->getMethod("duplicate")) [[likely]]
        {
          if(QMessageBox::question(
            this,
            Locale::tr("qtapp.world_list_dialog:duplicate_world"),
            Locale::tr("qtapp.world_list_dialog:duplicate_world_confirmation").arg(getWorldName(index.row())),
            QMessageBox::Yes,
            QMessageBox::No) == QMessageBox::Yes)
          {
            const auto uuid = getWorldUUID(index.row());
            const auto name = getWorldName(index.row()).append(" - ").append(Locale::tr("qtapp.world_list_dialog:duplicated_world_name_postfix"));

            [[maybe_unused]] int r = callMethodR<bool>(*duplicate,
              [](bool success, std::optional<const Error> err)
              {
                (void)success;
                (void)err;
              }, uuid, name);
          }
        }
      }
    });
  connect(itemDelegate, &WorldListItemDelegate::deleteClicked,
    [this](const QModelIndex& index)
    {
      if(m_object) [[likely]]
      {
        if(auto* deleteWorld = m_object->getMethod("delete")) [[likely]]
        {
          if(QMessageBox::critical(
            this,
            Locale::tr("qtapp.world_list_dialog:delete_world"),
            Locale::tr("qtapp.world_list_dialog:delete_world_confirmation").arg(getWorldName(index.row())),
            QMessageBox::Yes,
            QMessageBox::No) == QMessageBox::Yes)
          {
            const auto uuid = getWorldUUID(index.row());

            [[maybe_unused]] int r = callMethodR<bool>(*deleteWorld,
              [](bool success, std::optional<const Error> err)
              {
                (void)success;
                (void)err;
              }, uuid);
          }
        }
      }
    });
  m_list->setItemDelegate(itemDelegate);
  m_list->setMouseTracking(true);
  m_list->viewport()->setMouseTracking(true);

  m_buttons->setStandardButtons(QDialogButtonBox::Open | QDialogButtonBox::Cancel);
  m_buttons->button(QDialogButtonBox::Open)->setText(Locale::tr("qtapp.world_list_dialog:load"));
  m_buttons->button(QDialogButtonBox::Open)->setEnabled(false);
  connect(m_buttons->button(QDialogButtonBox::Open), &QPushButton::clicked, this,
    [this]()
    {
      m_uuid = getWorldUUID(m_list->selectionModel()->selectedIndexes().first().row());
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

QString WorldListDialog::getWorldUUID(int row) const
{
  return m_list->model()->data(m_list->model()->index(row, columnUUID)).toString();
}

QString WorldListDialog::getWorldName(int row) const
{
  return m_list->model()->data(m_list->model()->index(row, columnName)).toString();
}

