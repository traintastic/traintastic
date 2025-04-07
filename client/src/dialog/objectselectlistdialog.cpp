/**
 * client/src/dialog/objectselectlistdialog.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2023-2024 Reinder Feenstra
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

#include "objectselectlistdialog.hpp"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtWaitingSpinner/waitingspinnerwidget.h>
#include "../network/callmethod.hpp"
#include "../network/connection.hpp"
#include "../network/object.hpp"
#include "../network/method.hpp"
#include "../network/objectproperty.hpp"
#include "../widget/tablewidget.hpp"
#include "../widget/alertwidget.hpp"
#include <traintastic/locale/locale.hpp>
#include <unordered_set>

ObjectSelectListDialog::ObjectSelectListDialog(Method& method, bool multiSelect, QWidget* parent) :
  ObjectSelectListDialog(static_cast<InterfaceItem&>(method), multiSelect, parent)
{
}

ObjectSelectListDialog::ObjectSelectListDialog(ObjectProperty& property, QWidget* parent) :
  ObjectSelectListDialog(static_cast<InterfaceItem&>(property), false, parent)
{
}

ObjectSelectListDialog::ObjectSelectListDialog(InterfaceItem& item, bool multiSelect, QWidget* parent) :
  QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
  m_item{item},
  m_multiSelect{multiSelect},
  m_buttons{new QDialogButtonBox(this)},
  m_tableWidget{new TableWidget()}
{
  setWindowTitle(Locale::tr("qtapp.object_select_list_dialog:select_object"));

  m_buttons->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  m_buttons->button(QDialogButtonBox::Ok)->setText(Locale::tr("qtapp.object_select_list_dialog:ok"));
  m_buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
  connect(m_buttons->button(QDialogButtonBox::Ok), &QPushButton::clicked, this,
    [this]()
    {
      if(m_multiSelect)
      {
        acceptRows(m_tableWidget->selectionModel()->selectedIndexes());
      }
      else
      {
        acceptRow(m_tableWidget->selectionModel()->selectedIndexes().first().row());
      }
    });
  m_buttons->button(QDialogButtonBox::Cancel)->setText(Locale::tr("qtapp.object_select_list_dialog:cancel"));
  connect(m_buttons->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &ObjectSelectListDialog::reject);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_tableWidget);
  layout->addWidget(m_buttons);
  setLayout(layout);

  auto* spinner = new WaitingSpinnerWidget(this, true, false);
  spinner->start();

  m_requestId = m_item.object().connection()->getObject(m_item.getAttribute(AttributeName::ObjectList, QVariant()).toString(),
    [this, spinner](const ObjectPtr& object, std::optional<const Error> error)
    {
      if(object)
      {
        m_object = object;

        m_requestId = m_item.object().connection()->getTableModel(m_object,
          [this, spinner](const TableModelPtr& tableModel, std::optional<const Error> err)
          {
            if(tableModel)
            {
              m_requestId = Connection::invalidRequestId;

              m_tableWidget->setTableModel(tableModel);
              m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
              connect(m_tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this,
                [this](const QItemSelection&, const QItemSelection&)
                {
                  const auto selectionCount = m_tableWidget->selectionModel()->selectedRows().count();
                  m_buttons->button(QDialogButtonBox::Ok)->setEnabled(m_multiSelect ? selectionCount > 0 : selectionCount == 1);
                });
              connect(m_tableWidget, &TableWidget::doubleClicked, this,
                [this](const QModelIndex& index)
                {
                  acceptRow(index.row());
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

void ObjectSelectListDialog::acceptRow(int row)
{
  const QString id{m_tableWidget->getRowObjectId(row)};

  if(auto* p = dynamic_cast<ObjectProperty*>(&m_item))
    p->setByObjectId(id);
  else if(auto* m = dynamic_cast<Method*>(&m_item))
    callMethod(*m, nullptr, id);

  accept();
}

void ObjectSelectListDialog::acceptRows(const QModelIndexList& indexes)
{
  if(auto* m = dynamic_cast<Method*>(&m_item))
  {
    // Call method only once for each row, not for every index in row
    std::unordered_set<int> rowsDone;

    for(const auto& index : indexes)
    {
      if(rowsDone.find(index.row()) != rowsDone.end())
        continue;

      callMethod(*m, nullptr, m_tableWidget->getRowObjectId(index.row()));
      rowsDone.insert(index.row());
    }
  }

  accept();
}
