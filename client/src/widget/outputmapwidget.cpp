/**
 * client/src/widget/outputmapwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2025 Reinder Feenstra
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

#include "outputmapwidget.hpp"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QToolBar>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QEvent>
#include <traintastic/locale/locale.hpp>
#include "createwidget.hpp"
#include "interfaceitemnamelabel.hpp"
#include "propertycheckbox.hpp"
#include "propertycombobox.hpp"
#include "propertypairoutputaction.hpp"
#include "propertyspinbox.hpp"
#include "objectpropertycombobox.hpp"
#include "propertyaddresses.hpp"
#include "outputmapoutputactionwidget.hpp"
#include "methodicon.hpp"
#include "../board/tilepainter.hpp"
#include "../board/getboardcolorscheme.hpp"
#include "../dialog/objectselectlistdialog.hpp"
#include "../network/callmethod.hpp"
#include "../network/method.hpp"
#include "../network/property.hpp"
#include "../network/objectproperty.hpp"
#include "../network/vectorproperty.hpp"
#include "../network/objectvectorproperty.hpp"
#include "../utils/enum.hpp"
#include "../theme/theme.hpp"
#include "../misc/methodaction.hpp"

constexpr int columnKey = 0;

static bool hasUseColumn(const QString& classId)
{
  return classId == "output_map.signal";
}

static void setComboBoxMinimumWidth(QComboBox* comboBox)
{
  comboBox->setMinimumWidth(25 * comboBox->fontMetrics().averageCharWidth());
}

OutputMapWidget::OutputMapWidget(ObjectPtr object, QWidget* parent)
  : QWidget(parent)
  , m_object{std::move(object)}
  , m_hasUseColumn{hasUseColumn(m_object->classId())}
  , m_columnCountNonOutput{m_hasUseColumn ? 2 : 1}
  , m_addresses{m_object->getVectorProperty("addresses")}
  , m_ecosObject{dynamic_cast<Property*>(m_object->getProperty("ecos_object"))}
  , m_items{m_object->getObjectVectorProperty("items")}
  , m_table{new QTableWidget(this)}
  , m_getParentRequestId{Connection::invalidRequestId}
  , m_getItemsRequestId{Connection::invalidRequestId}
{
  QVBoxLayout* l = new QVBoxLayout();

  QFormLayout* form = new QFormLayout();
  if(auto* interface = dynamic_cast<ObjectProperty*>(m_object->getProperty("interface")))
  {
    auto* comboBox = new ObjectPropertyComboBox(*interface, this);
    setComboBoxMinimumWidth(comboBox);
    form->addRow(new InterfaceItemNameLabel(*interface, this), comboBox);
  }
  if(auto* channel = dynamic_cast<Property*>(m_object->getProperty("channel")))
  {
    auto* comboBox = new PropertyComboBox(*channel, this);
    setComboBoxMinimumWidth(comboBox);
    form->addRow(new InterfaceItemNameLabel(*channel, this), comboBox);
  }
  if(m_addresses)
  {
    form->addRow(new InterfaceItemNameLabel(*m_addresses, this), new PropertyAddresses(*m_addresses, m_object->getMethod("add_address"), m_object->getMethod("remove_address"), this));
    connect(m_addresses, &AbstractVectorProperty::valueChanged, this, &OutputMapWidget::updateTableOutputColumns);
  }
  if(m_ecosObject)
  {
    auto* comboBox = new PropertyComboBox(*m_ecosObject, this);
    setComboBoxMinimumWidth(comboBox);
    form->addRow(new InterfaceItemNameLabel(*m_ecosObject, this), comboBox);
    connect(m_ecosObject, &AbstractVectorProperty::valueChanged, this, &OutputMapWidget::updateTableOutputColumns);
  }
  l->addLayout(form);

  const int listViewIconSize = m_table->style()->pixelMetric(QStyle::PM_ListViewIconSize);
  m_table->setIconSize({listViewIconSize, listViewIconSize});
  m_table->setColumnCount(m_columnCountNonOutput);
  m_table->setRowCount(0);
  QStringList labels;
  labels.append(Locale::tr(m_object->classId() + ":key"));
  if(m_hasUseColumn)
  {
    labels.append(Locale::tr("output_map:use"));
  }
  m_table->setHorizontalHeaderLabels(labels);
  m_table->verticalHeader()->setVisible(false);
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

  l->addWidget(m_table);

  if(auto* swapOutputs = m_object->getMethod("swap_outputs"))
  {
    m_swapOutputs = new MethodIcon(*swapOutputs, Theme::getIcon("swap"), m_table);
    if(!swapOutputs->getAttributeBool(AttributeName::Visible, true))
    {
      m_swapOutputs->hide();
    }
    m_table->installEventFilter(this);
    m_swapOutputs->installEventFilter(this);
  }

  setLayout(l);

  if(auto* parentObject = m_object->getObjectProperty("parent"))
  {
    m_getParentRequestId = parentObject->getObject(
      [this](const ObjectPtr& obj, std::optional<const Error> ec)
      {
        if(obj && !ec)
        {
          m_getParentRequestId = Connection::invalidRequestId;
          m_parentObject = obj;
          updateKeyIcons();
        }
      });
  }

  if(m_items) /*[[likely]]*/
  {
    m_getItemsRequestId = m_items->getObjects(
      [this](const std::vector<ObjectPtr>& objects, std::optional<const Error> ec)
      {
        if(!ec)
        {
          updateItems(objects);
        }
      });

    connect(&BoardSettings::instance(), &BoardSettings::changed, this, &OutputMapWidget::updateKeyIcons);
  }
}

OutputMapWidget::~OutputMapWidget()
{
  if(m_getParentRequestId != Connection::invalidRequestId)
  {
    m_object->connection()->cancelRequest(m_getParentRequestId);
  }
  if(m_getItemsRequestId != Connection::invalidRequestId)
  {
    m_object->connection()->cancelRequest(m_getItemsRequestId);
  }
}

void OutputMapWidget::updateItems(const std::vector<ObjectPtr>& items)
{
  m_table->setRowCount(static_cast<int>(items.size()));
  m_itemObjects = items;
  m_actions.resize(items.size());
  for(size_t i = 0; i < items.size(); i++)
  {
    if(auto* p = items[i]->getProperty("key"))
    {
      QString text;
      if(p->type() ==  ValueType::Enum)
      {
        text = translateEnum(*p);
      }
      else if(p->type() == ValueType::Boolean)
      {
        text = p->toBool() ? "true" : "false";

        if(p->hasAttribute(AttributeName::AliasKeys))
        {
          const QVariantList aliasKeys = p->getAttribute(AttributeName::AliasKeys, QVariant()).toList();
          const QVariantList aliasValues = p->getAttribute(AttributeName::AliasValues, QVariant()).toList();

          if(aliasKeys.size() == aliasValues.size()) /*[[likely]]*/
          {
            if(int index = aliasKeys.indexOf(p->toBool()); index != -1)
            {
              text = Locale::instance->parse(aliasValues[index].toString());
            }
          }
        }
      }
      else /*[[unlikely]]*/
      {
        assert(false);
        text = "?";
      }
      m_table->setItem(static_cast<int>(i), columnKey, new QTableWidgetItem(text));
    }

    if(m_hasUseColumn)
    {
      const int columnUse = columnKey + 1;

      if(auto* p = dynamic_cast<Property*>(items[i]->getProperty("use")))
      {
        QWidget* w = new QWidget(m_table);
        QHBoxLayout* l = new QHBoxLayout();
        l->setAlignment(Qt::AlignCenter);
        l->addWidget(new PropertyCheckBox(*p, w));
        w->setLayout(l);
        m_table->setCellWidget(static_cast<int>(i), columnUse, w);
      }
    }

    if(auto* p = items[i]->getProperty("visible"))
    {
      m_table->setRowHidden(static_cast<int>(i), !p->toBool());

      connect(p, &Property::valueChangedBool, this,
        [this, row=static_cast<int>(i)](bool value)
        {
          m_table->setRowHidden(row, !value);
        });
    }

    if(auto* outputActions = dynamic_cast<ObjectVectorProperty*>(items[i]->getVectorProperty("output_actions")))
    {
      updateTableOutputActions(*outputActions, static_cast<int>(i));

      connect(outputActions, &ObjectVectorProperty::valueChanged, this,
        [this, row=static_cast<int>(i)]()
        {
          updateTableOutputActions(*dynamic_cast<ObjectVectorProperty*>(sender()), row);
        });
    }
  }

  updateKeyIcons();
  updateTableOutputColumns();
}

void OutputMapWidget::updateKeyIcons()
{
  if(!m_parentObject)
  {
    return;
  }

  if(auto tileIdProperty = m_parentObject->getProperty("tile_id"))
  {
    const bool darkBackground = m_table->palette().window().color().lightnessF() < 0.5;
    const auto tileId = tileIdProperty->toEnum<TileId>();

    const int iconSize = m_table->iconSize().height();
    QImage image(iconSize, iconSize, QImage::Format_ARGB32);
    QPainter painter{&image};
    painter.setRenderHint(QPainter::Antialiasing, true);
    TilePainter tilePainter{painter, iconSize, *getBoardColorScheme(darkBackground ? BoardSettings::ColorScheme::Dark : BoardSettings::ColorScheme::Light)};

    for(size_t i = 0; i < m_itemObjects.size(); i++)
    {
      if(auto* key = m_itemObjects[i]->getProperty("key"))
      {
        image.fill(Qt::transparent);

        if(isRailTurnout(tileId))
        {
          tilePainter.drawTurnout(tileId, image.rect(), TileRotate::Deg0, TurnoutPosition::Unknown, static_cast<TurnoutPosition>(key->toInt()));
        }
        else if(isRailSignal(tileId))
        {
          tilePainter.drawSignal(tileId, image.rect(), TileRotate::Deg0, false, static_cast<SignalAspect>(key->toInt()));
        }
        else if(tileId == TileId::RailDirectionControl)
        {
          tilePainter.drawDirectionControl(tileId, image.rect(), TileRotate::Deg0, false, static_cast<DirectionControlState>(key->toInt()));
        }
        else if(tileId == TileId::RailDecoupler)
        {
          tilePainter.drawRailDecoupler(image.rect(), TileRotate::Deg90, false, static_cast<DecouplerState>(key->toInt()));
        }
        else if(tileId == TileId::Switch)
        {
          tilePainter.drawSwitch(image.rect(), key->toBool() ? Color::Yellow : Color::Gray);
        }
        else
        {
          break; // tileId not supported (yet)
        }

        m_table->item(static_cast<int>(i), columnKey)->setIcon(QPixmap::fromImage(image));
      }
    }
  }
}

void OutputMapWidget::updateTableOutputColumns()
{
  if(m_addresses && m_addresses->getAttributeBool(AttributeName::Visible, true))
  {
    const auto size = m_addresses->size();

    m_table->setColumnCount(m_columnCountNonOutput + size);
    for(int i = 0; i < size; i++)
    {
      const int column = m_columnCountNonOutput + i;
      const int address = m_addresses->getInt(i);
      auto* item = new QTableWidgetItem(QString("#%1").arg(address));
      item->setToolTip(Locale::tr("output_map:address_x").arg(address));
      m_table->setHorizontalHeaderItem(column, item);
    }
  }
  else if(m_ecosObject && m_ecosObject->getAttributeBool(AttributeName::Visible, true))
  {
    m_table->setColumnCount(m_columnCountNonOutput + 1);
    m_table->setHorizontalHeaderItem(m_columnCountNonOutput, new QTableWidgetItem(Locale::tr("output.ecos_object:state")));
  }
  else
  {
    m_table->setColumnCount(m_columnCountNonOutput);
  }
}

bool OutputMapWidget::eventFilter(QObject* object, QEvent* event)
{
  if(m_swapOutputs && ((object == m_table && event->type() == QEvent::Resize) || (object == m_swapOutputs && event->type() == QEvent::Show)))
  {
    auto pnt = m_swapOutputs->rect().bottomRight();
    pnt = m_table->rect().bottomRight() - pnt - pnt / 4;
    m_swapOutputs->move(pnt.x(), pnt.y());
  }
  return QWidget::eventFilter(object, event);
}

void OutputMapWidget::updateTableOutputActions(ObjectVectorProperty& property, int row)
{
  if(!property.empty())
  {
    m_dummy = property.getObjects(
      [this, row](const std::vector<ObjectPtr>& objects, std::optional<const Error> /*ec*/)
      {
        const int columnCount = static_cast<int>(m_columnCountNonOutput + objects.size());
        if(columnCount > m_table->columnCount())
        {
          updateTableOutputColumns();
        }

        auto& rowActions = m_actions[row];
        int column = m_columnCountNonOutput;
        for(auto& object : objects)
        {
          if(column >= static_cast<int>(rowActions.size()) || object.get() != rowActions[column].get())
          {
            if(auto* action = dynamic_cast<Property*>(object->getProperty("action")))
            {
              m_table->setCellWidget(row, column, createWidget(*action, this));
            }
            else if(auto* aspect = dynamic_cast<Property*>(object->getProperty("aspect")))
            {
              m_table->setCellWidget(row, column, new PropertySpinBox(*aspect, this));
            }
            else if(auto* state = dynamic_cast<Property*>(object->getProperty("state")))
            {
              m_table->setCellWidget(row, column, new PropertySpinBox(*state, this));
            }
          }
          column++;
        }
        rowActions = objects;
      });
  }
}
