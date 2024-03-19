/**
 * client/src/widget/objectpropertycombobox.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "objectpropertycombobox.hpp"
#include "../network/connection.hpp"
#include "../network/object.hpp"
#include "../network/objectproperty.hpp"
#include "../network/tablemodel.hpp"
#include "../network/error.hpp"

class ObjectPropertyComboBoxModel final : public QAbstractListModel
{
  private:
    static constexpr int columnId = 0;
    static constexpr int columnName = 1;

    ObjectProperty& m_property;
    int m_requestId = Connection::invalidRequestId;
    ObjectPtr m_object;
    TableModelPtr m_model;

  public:
    ObjectPropertyComboBoxModel(ObjectProperty& property, QObject* parent = nullptr)
      : QAbstractListModel(parent)
      , m_property{property}
    {
      m_requestId = m_property.object().connection()->getObject(m_property.getAttributeString(AttributeName::ObjectList, {}),
        [this](const ObjectPtr& object, std::optional<const Error> /*error*/)
        {
          m_requestId = Connection::invalidRequestId;
          if(object)
          {
            m_object = object;

            m_requestId = m_property.object().connection()->getTableModel(m_object,
              [this](const TableModelPtr& model, std::optional<const Error> /*error*/)
              {
                m_requestId = Connection::invalidRequestId;
                if(model)
                {
                  beginResetModel();
                  m_model = model;
                  m_model->setRegion(0, columnName, 0, std::numeric_limits<int>::max());
                  connect(m_model.get(), &TableModel::modelAboutToBeReset, this, &ObjectPropertyComboBoxModel::beginResetModel);
                  connect(m_model.get(), &TableModel::modelReset, this, &ObjectPropertyComboBoxModel::endResetModel);
                  endResetModel();
                }
              });
          }
        });
    }

    ~ObjectPropertyComboBoxModel() final
    {
      if(m_requestId != Connection::invalidRequestId)
      {
        m_property.object().connection()->cancelRequest(m_requestId);
      }
    }

    int rowCount(const QModelIndex& /*parent*/ = QModelIndex()) const final
    {
      return m_model ? 1 + m_model->rowCount() : 0;
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const final
    {
      if(!m_model)
      {
        return {};
      }

      if(index.row() == 0)
      {
        return role == Qt::DisplayRole ? QVariant("") : QVariant{};
      }
      return m_model->data(createIndex(index.row() - 1, columnName), role);
    }

    int getIndex(const QString& objectId) const
    {
      if(m_model)
      {
        for(int row = 0; row < m_model->rowCount(); ++row)
        {
          if(m_model->data(createIndex(row, columnId), Qt::DisplayRole).toString() == objectId)
          {
            return 1 + row;
          }
        }
      }
      return -1;
    }

    QString getObjectId(int index)
    {
      if(!m_model) /*[[unlikely]]*/
      {
        return {};
      }
      return m_model->getRowObjectId(index);
    }
};


ObjectPropertyComboBox::ObjectPropertyComboBox(ObjectProperty& property, QWidget* parent) :
  QComboBox(parent),
  m_property{property}
{
  setEnabled(m_property.getAttributeBool(AttributeName::Enabled, true));
  setVisible(m_property.getAttributeBool(AttributeName::Visible, true));

  setModel(new ObjectPropertyComboBoxModel(m_property, this));
  connect(model(), &QAbstractItemModel::modelAboutToBeReset, this,
    [this]()
    {
      m_modelUpdating = true;
    });
  connect(model(), &QAbstractItemModel::modelReset, this,
    [this]()
    {
      m_modelUpdating = false;
      updateValue();
    });

  connect(this, static_cast<void(ObjectPropertyComboBox::*)(int)>(&ObjectPropertyComboBox::currentIndexChanged),
    [this](int index)
    {
      if(m_modelUpdating)
      {
        return; // ignore index changes during update, else the property is changed.
      }

      if(index == 0)
      {
        m_property.setByObjectId({}); // set empty
      }
      else
      {
        m_property.setByObjectId(static_cast<ObjectPropertyComboBoxModel*>(model())->getObjectId(index - 1));
      }
    });

  connect(&m_property, &ObjectProperty::valueChanged, this, &ObjectPropertyComboBox::updateValue);
  connect(&m_property, &ObjectProperty::attributeChanged, this,
    [this](AttributeName name, const QVariant& value)
    {
      switch(name)
      {
        case AttributeName::Enabled:
          setEnabled(value.toBool());
          break;

        case AttributeName::Visible:
          setVisible(value.toBool());
          break;

        default:
          break;
      }
    });
}

void ObjectPropertyComboBox::updateValue()
{
  QSignalBlocker b{*this}; // prevent emitting currentIndexChanged

  if(m_property.hasObject())
  {
    setCurrentIndex(static_cast<ObjectPropertyComboBoxModel*>(model())->getIndex(m_property.objectId()));
  }
  else
  {
    setCurrentIndex(0);
  }
}
