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

#include "decoderfunctionswidget.hpp"
#include <span>
#include <QVBoxLayout>
#include <QToolBar>
#include <QTableView>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QComboBox>
#include "../../network/callmethod.hpp"
#include "../../network/connection.hpp"
#include "../../network/error.hpp"
#include "../../network/method.hpp"
#include "../../network/object.hpp"
#include "../../network/objectproperty.hpp"
#include "../../misc/methodaction.hpp"
#include "../../theme/theme.hpp"
#include "decoderfunctionsmodel.hpp"
#include <traintastic/enum/decoderfunctionfunction.hpp>
#include <traintastic/enum/decoderfunctiontype.hpp>
#include <traintastic/locale/locale.hpp>

namespace {

template<typename T>
class ComboBoxDelegate : public QStyledItemDelegate
{
private:
  const std::span<const T> m_values;

public:
  ComboBoxDelegate(std::span<const T> values, QObject *parent = nullptr)
    : QStyledItemDelegate(parent)
    , m_values{values}
  {
  }

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const override
  {
    QComboBox* editor = new QComboBox(parent);
    for(auto value : m_values)
    {
      editor->addItem(Locale::tr(QString("%1:%2").arg(EnumName<T>::value).arg(EnumValues<T>::value.at(value))), static_cast<qint64>(value));
    }
    return editor;
  }

  void setEditorData(QWidget* editor, const QModelIndex& index) const override
  {
    QComboBox* comboBox = static_cast<QComboBox*>(editor);
    const int n = comboBox->findData(index.data(Qt::EditRole));
    if(n >= 0)
    {
      comboBox->setCurrentIndex(n);
    }
  }

  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
  {
    QComboBox* comboBox = static_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentData(), Qt::EditRole);
  }
};

}

DecoderFunctionsWidget::DecoderFunctionsWidget(ObjectProperty& decoderProperty, QWidget* parent)
  : QWidget(parent)
  , m_decoderProperty{decoderProperty}
  , m_toolbar{new QToolBar(this)}
  , m_table{new QTableView(this)}
  , m_requestId{Connection::invalidRequestId}
{
  setWindowTitle(Locale::tr("decoder:functions"));

  setLayout(new QVBoxLayout());
  layout()->addWidget(m_toolbar);
  layout()->addWidget(m_table);

  connect(&m_decoderProperty, &ObjectProperty::valueChanged, this, &DecoderFunctionsWidget::decoderChanged);
  decoderChanged();
}

DecoderFunctionsWidget::~DecoderFunctionsWidget()
{
  cancelRequest();
}

const ObjectPtr& DecoderFunctionsWidget::getSelectedFunction() const
{
  static const ObjectPtr null;
  if(m_table->selectionModel()->hasSelection())
  {
    const int row = m_table->selectionModel()->selection().first().indexes().front().row();
    return static_cast<DecoderFunctionsModel*>(m_table->model())->getObject(row);
  }
  return null;
}

void DecoderFunctionsWidget::cancelRequest()
{
  if(m_requestId != Connection::invalidRequestId)
  {
    m_decoderProperty.object().connection()->cancelRequest(m_requestId);
    m_requestId = Connection::invalidRequestId;
  }
}

void DecoderFunctionsWidget::decoderChanged()
{
  m_table->setModel(nullptr);

  cancelRequest();

  if(m_decoderProperty.hasObject())
  {
    const auto id = m_decoderProperty.objectId() + ".functions";
    m_requestId = m_decoderProperty.object().connection()->getObject(id,
      [this](const ObjectPtr& object, std::optional<const Error> /*error*/)
      {
        m_requestId = Connection::invalidRequestId;
        if(object)
        {
          if(auto* create = object->getMethod(QStringLiteral("create")))
          {
            m_toolbar->addAction(new MethodAction(Theme::getIcon("add"), *create));
          }
          if(auto* delete_ = object->getMethod(QStringLiteral("delete")))
          {
            m_delete = new MethodAction(Theme::getIcon("delete"), *delete_,
              [this, delete_]()
              {
                if(const auto& function = getSelectedFunction())
                {
                  callMethod(*delete_, nullptr, function);
                }
              });
            m_toolbar->addAction(m_delete);
          }
          if(!m_toolbar->actions().isEmpty())
          {
            m_toolbar->addSeparator();
          }
          if(auto* moveUp = object->getMethod(QStringLiteral("move_up")))
          {
            m_moveUp = new MethodAction(Theme::getIcon("up"), *moveUp,
              [this, moveUp]()
              {
                if(const auto& function = getSelectedFunction())
                {
                  callMethod(*moveUp, nullptr, function);
                }
              });
            m_toolbar->addAction(m_moveUp);
          }
          if(auto* moveDown = object->getMethod(QStringLiteral("move_down")))
          {
            m_moveDown = new MethodAction(Theme::getIcon("down"), *moveDown,
              [this, moveDown]()
              {
                if(const auto& function = getSelectedFunction())
                {
                  callMethod(*moveDown, nullptr, function);
                }
              });
            m_toolbar->addAction(m_moveDown);
          }

          const int acw = m_table->fontMetrics().averageCharWidth();
          m_table->setModel(new DecoderFunctionsModel(object, this));

          m_table->setColumnWidth(DecoderFunctionsModel::columnNumber, 4 * acw);
          m_table->setColumnWidth(DecoderFunctionsModel::columnFunction, 15 * acw);
          m_table->horizontalHeader()->setSectionResizeMode(DecoderFunctionsModel::columnName, QHeaderView::Stretch);
          m_table->setColumnWidth(DecoderFunctionsModel::columnType, 15 * acw);

          m_table->setItemDelegateForColumn(DecoderFunctionsModel::columnFunction, new ComboBoxDelegate<DecoderFunctionFunction>(decoderFunctionFunctionValues, m_table));
          m_table->setItemDelegateForColumn(DecoderFunctionsModel::columnType, new ComboBoxDelegate<DecoderFunctionType>(decoderFunctionTypeValues, m_table));

          m_table->setSelectionMode(QTableView::SingleSelection);
          connect(m_table->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this](const QItemSelection& selected, const QItemSelection& /*deselected*/)
            {
              const bool emptySelection = selected.empty();
              const bool firstRowSelected = !emptySelection && selected.front().indexes().front().row() == 0;
              const bool lastRowSelected = !emptySelection && selected.front().indexes().front().row() == m_table->model()->rowCount() - 1;

              if(m_delete)
              {
                m_delete->setForceDisabled(emptySelection);
              }
              if(m_moveUp)
              {
                m_moveUp->setForceDisabled(emptySelection || firstRowSelected);
              }
              if(m_moveDown)
              {
                m_moveDown->setForceDisabled(emptySelection || lastRowSelected);
              }
            });
        }
      });
  }
}
