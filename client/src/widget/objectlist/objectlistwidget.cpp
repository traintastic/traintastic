/**
 * client/src/widget/objectlist/objectlistwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

#include "objectlistwidget.hpp"
#include <QVBoxLayout>
#include <QToolBar>
#include <QTableView>
#include <QtWaitingSpinner/waitingspinnerwidget.h>
#include <traintastic/locale/locale.hpp>
#include "../tablewidget.hpp"
#include "../../network/connection.hpp"
#include "../../network/object.hpp"
#include "../../network/method.hpp"
#include "../../network/utils.hpp"
#include "../../network/callmethod.hpp"
#include "../alertwidget.hpp"
#include "../../theme/theme.hpp"
#include "../../misc/methodaction.hpp"


#include "../../mainwindow.hpp"



#include <QMenu>
#include <QToolButton>



ObjectListWidget::ObjectListWidget(const ObjectPtr& object, QWidget* parent) :
  QWidget(parent),
  m_requestIdInputMonitor{Connection::invalidRequestId},
  m_requestIdOutputKeyboard{Connection::invalidRequestId},
  m_object{object},
  m_toolbar{new QToolBar()},
  m_buttonAdd{nullptr},
  m_actionAdd{nullptr},
  m_actionEdit{nullptr},
  m_actionDelete{nullptr},
  m_actionInputMonitor{nullptr},
  m_actionInputMonitorChannel{nullptr},
  m_actionOutputKeyboard{nullptr},
  m_actionOutputKeyboardChannel{nullptr},
  m_tableWidget{new TableWidget()}
{
  m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_toolbar);
  layout->addWidget(m_tableWidget);
  setLayout(layout);

  auto* spinner = new WaitingSpinnerWidget(this, true, false);
  spinner->start();

  m_requestId = m_object->connection()->getTableModel(m_object,
    [this, spinner](const TableModelPtr& tableModel, Message::ErrorCode ec)
    {
      if(tableModel)
      {
        m_requestId = Connection::invalidRequestId;

        m_tableWidget->setTableModel(tableModel);
        connect(m_tableWidget, &TableWidget::doubleClicked, this, &ObjectListWidget::tableDoubleClicked);
        connect(m_tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this,
          [this](const QItemSelection&, const QItemSelection&)
          {
            tableSelectionChanged();
          });
        tableSelectionChanged();

        delete spinner;
      }
      else
        static_cast<QVBoxLayout*>(this->layout())->insertWidget(0, AlertWidget::error(errorCodeToText(ec)));
    });

  if(Method* method = m_object->getMethod("add");
     method && method->resultType() == ValueType::Object)
  {
    if(method->argumentTypes().size() == 0) // Add method witout argument
    {
      m_actionAdd = m_toolbar->addAction(Theme::getIcon("add"), method->displayName(),
        [this, method]()
        {
          if(m_requestIdAdd != Connection::invalidRequestId)
            m_object->connection()->cancelRequest(m_requestIdAdd);

          m_requestIdAdd = method->call(
            [this](const ObjectPtr& addedObject, Message::ErrorCode /*ec*/)
            {
              m_requestIdAdd = Connection::invalidRequestId;
              if(addedObject)
              {
                MainWindow::instance->showObject(addedObject);
              }
              // TODO: show error
            });
        });
      m_actionAdd->setEnabled(method->getAttributeBool(AttributeName::Enabled, true));
      connect(method, &Method::attributeChanged, this,
        [this](AttributeName name, QVariant value)
        {
          if(name == AttributeName::Enabled)
            m_actionAdd->setEnabled(value.toBool());
        });
    }
    else if(method->argumentTypes().size() == 1 && method->argumentTypes()[0] == ValueType::String)
    {
      m_buttonAdd = new QToolButton(m_toolbar);
      m_buttonAdd->setIcon(Theme::getIcon("add"));
      m_buttonAdd->setText(method->displayName());
      m_buttonAdd->setPopupMode(QToolButton::InstantPopup);

      QMenu* menu = new QMenu(m_buttonAdd);

      QStringList classList = method->getAttribute(AttributeName::ClassList, QVariant()).toStringList();
      for(const QString& classId : classList)
      {
        QAction* action = menu->addAction(Locale::tr("class_id:" + classId));
        action->setData(classId);
        connect(action, &QAction::triggered, this,
          [this, method, action]()
          {
            if(m_requestIdAdd != Connection::invalidRequestId)
              m_object->connection()->cancelRequest(m_requestIdAdd);

            m_requestIdAdd = method->call(action->data().toString(),
              [this](const ObjectPtr& addedObject, Message::ErrorCode /*ec*/)
              {
                m_requestIdAdd = Connection::invalidRequestId;
                if(addedObject)
                {
                  MainWindow::instance->showObject(addedObject);
                }
                // TODO: show error
              });
          });
      }

      m_buttonAdd->setMenu(menu);

      m_toolbar->addWidget(m_buttonAdd);

      m_buttonAdd->setEnabled(method->getAttributeBool(AttributeName::Enabled, true));
      connect(method, &Method::attributeChanged, this,
        [this](AttributeName name, QVariant value)
        {
          if(name == AttributeName::Enabled)
            m_buttonAdd->setEnabled(value.toBool());
        });
    }
    else
      Q_ASSERT(false); // unsupported method prototype
  }

  m_actionEdit = m_toolbar->addAction(Theme::getIcon("edit"), Locale::tr("list:edit"),
    [this]()
    {
      for(const QString& id : getSelectedObjectIds())
        MainWindow::instance->showObject(id);
    });
  m_actionEdit->setEnabled(false);

  if(Method* method = m_object->getMethod("remove"))
  {
    m_actionDelete = new MethodAction(Theme::getIcon("delete"), *method,
      [this]()
      {
        for(const QString& id : getSelectedObjectIds())
          callMethod(m_actionDelete->method(), nullptr, id);
      });
    m_actionDelete->setForceDisabled(true);
    m_toolbar->addAction(m_actionDelete);
  }

  if(Method* method = m_object->getMethod("input_monitor"))
  {
    m_actionInputMonitor = new MethodAction(Theme::getIcon("input_monitor"), *method,
      [this]()
      {
        m_requestIdInputMonitor = m_actionInputMonitor->method().call(
          [](const ObjectPtr& inputMonitor, Message::ErrorCode)
          {
            if(inputMonitor)
              MainWindow::instance->showObject(inputMonitor);
          });
      });
  }

  if(Method* method = m_object->getMethod("input_monitor_channel"))
  {
    if(const auto values = method->getAttribute(AttributeName::Values, QVariant()).toList(); !values.isEmpty())
    {
      const QVariantList aliasKeys = method->getAttribute(AttributeName::AliasKeys, QVariant()).toList();
      const QVariantList aliasValues = method->getAttribute(AttributeName::AliasValues, QVariant()).toList();

      QMenu* menu = new QMenu(this);
      for(const auto& value : values)
      {
        QString text;
        if(int index = aliasKeys.indexOf(value); index != -1)
          text = Locale::instance->parse(aliasValues[index].toString());
        else
          text = value.toString();

        menu->addAction(text,
          [this, channel=value.toUInt()]()
          {
            //cancelRequest(m_requestIdInputMonitor);

            m_requestIdInputMonitor = callMethodR<ObjectPtr>(m_actionInputMonitorChannel->method(),
              [this](const ObjectPtr& inputMonitor, Message::ErrorCode /*ec*/)
              {
                m_requestIdInputMonitor = Connection::invalidRequestId;
                if(inputMonitor)
                  MainWindow::instance->showObject(inputMonitor);
              },
              channel);
          });
      }

      m_actionInputMonitorChannel = new MethodAction(Theme::getIcon("input_monitor"), *method);
      m_actionInputMonitorChannel->setMenu(menu);
    }
  }

  if(Method* method = m_object->getMethod("output_keyboard"))
  {
    m_actionOutputKeyboard = new MethodAction(Theme::getIcon("output_keyboard"), *method,
      [this]()
      {
        //cancelRequest(m_requestIdOutputKeyboard);

        m_requestIdOutputKeyboard = m_actionOutputKeyboard->method().call(
          [this](const ObjectPtr& outputKeyboard, Message::ErrorCode)
          {
            m_requestIdOutputKeyboard = Connection::invalidRequestId;
            if(outputKeyboard)
              MainWindow::instance->showObject(outputKeyboard);
          });
      });
  }

  if(Method* method = m_object->getMethod("output_keyboard_channel"))
  {
    if(const auto values = method->getAttribute(AttributeName::Values, QVariant()).toList(); !values.isEmpty())
    {
      const QVariantList aliasKeys = method->getAttribute(AttributeName::AliasKeys, QVariant()).toList();
      const QVariantList aliasValues = method->getAttribute(AttributeName::AliasValues, QVariant()).toList();

      QMenu* menu = new QMenu(this);
      for(const auto& value : values)
      {
        QString text;
        if(int index = aliasKeys.indexOf(value); index != -1)
          text = Locale::instance->parse(aliasValues[index].toString());
        else
          text = value.toString();

        menu->addAction(text,
          [this, channel=value.toUInt()]()
          {
            //cancelRequest(m_requestIdOutputKeyboard);

            m_requestIdOutputKeyboard = callMethodR<ObjectPtr>(m_actionOutputKeyboardChannel->method(),
              [this](const ObjectPtr& outputKeyboard, Message::ErrorCode /*ec*/)
              {
                m_requestIdOutputKeyboard = Connection::invalidRequestId;
                if(outputKeyboard)
                  MainWindow::instance->showObject(outputKeyboard);
              },
              channel);
          });
      }

      m_actionOutputKeyboardChannel = new MethodAction(Theme::getIcon("output_keyboard"), *method);
      m_actionOutputKeyboardChannel->setMenu(menu);
    }
  }

  if(m_actionInputMonitor || m_actionInputMonitorChannel || m_actionOutputKeyboard || m_actionOutputKeyboardChannel)
  {
    m_toolbar->addSeparator();
    if(m_actionInputMonitor)
      m_toolbar->addAction(m_actionInputMonitor);
    if(m_actionInputMonitorChannel)
    {
      m_toolbar->addAction(m_actionInputMonitorChannel);
      if(auto* button = qobject_cast<QToolButton*>(m_toolbar->widgetForAction(m_actionInputMonitorChannel)))
        connect(m_actionInputMonitorChannel, &QAction::triggered, button, &QToolButton::showMenu);
    }
    if(m_actionOutputKeyboard)
      m_toolbar->addAction(m_actionOutputKeyboard);
    if(m_actionOutputKeyboardChannel)
    {
      m_toolbar->addAction(m_actionOutputKeyboardChannel);
      if(auto* button = qobject_cast<QToolButton*>(m_toolbar->widgetForAction(m_actionOutputKeyboardChannel)))
        connect(m_actionOutputKeyboardChannel, &QAction::triggered, button, &QToolButton::showMenu);
    }
  }

  {
    QAction* startAll = nullptr;
    if(Method* method = m_object->getMethod("start_all"))
      startAll = new MethodAction(Theme::getIcon("run"), *method);

    QAction* stopAll = nullptr;
    if(Method* method = m_object->getMethod("stop_all"))
      stopAll = new MethodAction(Theme::getIcon("stop"), *method);

    if(startAll || stopAll)
    {
      m_toolbar->addSeparator();
      if(startAll)
        m_toolbar->addAction(startAll);
      if(stopAll)
        m_toolbar->addAction(stopAll);
    }
  }
}

ObjectListWidget::~ObjectListWidget()
{
  m_object->connection()->cancelRequest(m_requestId);
}

void ObjectListWidget::objectDoubleClicked(const QString& id)
{
  SubWindowType type = SubWindowType::Object;
  if(m_object->classId() == "list.board")
    type = SubWindowType::Board;
  MainWindow::instance->showObject(id, "", type);
}

void ObjectListWidget::tableDoubleClicked(const QModelIndex& index)
{
  const QString id = m_tableWidget->getRowObjectId(index.row());
  if(!id.isEmpty())
    objectDoubleClicked(id);
}

QStringList ObjectListWidget::getSelectedObjectIds() const
{
  QStringList ids;

  if(auto* model = m_tableWidget->selectionModel(); model && model->hasSelection())
    for(const auto& index : model->selectedRows())
      if(QString id = m_tableWidget->getRowObjectId(index.row()); !id.isEmpty())
        ids.append(id);

  return ids;
}

void ObjectListWidget::tableSelectionChanged()
{
  tableSelectionChanged(m_tableWidget->selectionModel() && m_tableWidget->selectionModel()->hasSelection());
}

void ObjectListWidget::tableSelectionChanged(bool hasSelection)
{
  if(m_actionEdit)
    m_actionEdit->setEnabled(hasSelection);
  if(m_actionDelete)
    m_actionDelete->setForceDisabled(!hasSelection);
}
