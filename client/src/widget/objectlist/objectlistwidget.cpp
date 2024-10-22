/**
 * client/src/widget/objectlist/objectlistwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2024 Reinder Feenstra
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
#include <traintastic/locale/locale.hpp>
#include <traintastic/enum/outputchannel.hpp>
#include "../tablewidget.hpp"
#include "../../network/connection.hpp"
#include "../../network/object.hpp"
#include "../../network/method.hpp"
#include "../../network/callmethod.hpp"
#include "../../theme/theme.hpp"
#include "../../misc/methodaction.hpp"
#include "../../dialog/objectselectlistdialog.hpp"
#include "../../utils/enum.hpp"


#include "../../mainwindow.hpp"



#include <QMenu>
#include <QToolButton>



ObjectListWidget::ObjectListWidget(const ObjectPtr& object_, QWidget* parent) :
  ListWidget(object_, parent),
  m_requestIdInputMonitor{Connection::invalidRequestId},
  m_requestIdOutputKeyboard{Connection::invalidRequestId},
  m_toolbar{new QToolBar()}
{
  if(Method* method = object()->getMethod("create");
     method && method->resultType() == ValueType::Object)
  {
    if(method->argumentTypes().size() == 0) // Create method witout argument
    {
      m_actionCreate = m_toolbar->addAction(Theme::getIcon("add"), method->displayName(),
        [this, method]()
        {
          if(m_requestIdCreate != Connection::invalidRequestId)
            object()->connection()->cancelRequest(m_requestIdCreate);

          m_requestIdCreate = method->call(
            [this](const ObjectPtr& addedObject, std::optional<const Error> /*error*/)
            {
              m_requestIdCreate = Connection::invalidRequestId;
              if(addedObject)
              {
                objectCreated(addedObject);
              }
              // TODO: show error
            });
        });
      m_actionCreate->setEnabled(method->getAttributeBool(AttributeName::Enabled, true));
      connect(method, &Method::attributeChanged, this,
        [this](AttributeName name, QVariant value)
        {
          if(name == AttributeName::Enabled)
            m_actionCreate->setEnabled(value.toBool());
        });
    }
    else if(method->argumentTypes().size() == 1 && method->argumentTypes()[0] == ValueType::String)
    {
      m_buttonCreate = new QToolButton(m_toolbar);
      m_buttonCreate->setIcon(Theme::getIcon("add"));
      m_buttonCreate->setText(method->displayName());
      m_buttonCreate->setPopupMode(QToolButton::InstantPopup);

      QMenu* menu = new QMenu(m_buttonCreate);

      QStringList classList = method->getAttribute(AttributeName::ClassList, QVariant()).toStringList();
      for(const QString& classId : classList)
      {
        QAction* action = menu->addAction(Locale::tr("class_id:" + classId));
        action->setData(classId);
        connect(action, &QAction::triggered, this,
          [this, method, action]()
          {
            if(m_requestIdCreate != Connection::invalidRequestId)
              object()->connection()->cancelRequest(m_requestIdCreate);

            m_requestIdCreate = method->call(action->data().toString(),
              [this](const ObjectPtr& addedObject, std::optional<const Error> /*error*/)
              {
                m_requestIdCreate = Connection::invalidRequestId;
                if(addedObject)
                {
                  objectCreated(addedObject);
                }
                // TODO: show error
              });
          });
      }

      if(object_->classId() == "list.interface")
      {
        menu->addSeparator();
        menu->addAction(Theme::getIcon("wizard"), Locale::tr("list:setup_using_wizard") + "...",
          []()
          {
            MainWindow::instance->showAddInterfaceWizard();
          });
      }

      m_buttonCreate->setMenu(menu);

      m_toolbar->addWidget(m_buttonCreate);

      m_buttonCreate->setEnabled(method->getAttributeBool(AttributeName::Enabled, true));
      connect(method, &Method::attributeChanged, this,
        [this](AttributeName name, QVariant value)
        {
          if(name == AttributeName::Enabled)
            m_buttonCreate->setEnabled(value.toBool());
        });
    }
    else
      Q_ASSERT(false); // unsupported method prototype
  }

  if(Method* method = object()->getMethod("add");
     method &&
     method->argumentTypes().size() == 1 &&  method->argumentTypes()[0] == ValueType::Object &&
     method->resultType() == ValueType::Invalid)
  {
    const bool multiSelect = object_->classId() == "list.train_vehicle";
    m_actionAdd = m_toolbar->addAction(Theme::getIcon("add"), method->displayName(),
      [this, method, multiSelect]()
      {
        std::make_unique<ObjectSelectListDialog>(*method, multiSelect, this)->exec();
      });
    m_actionAdd->setEnabled(method->getAttributeBool(AttributeName::Enabled, true));
    connect(method, &Method::attributeChanged, this,
      [this](AttributeName name, QVariant value)
      {
        if(name == AttributeName::Enabled)
          m_actionAdd->setEnabled(value.toBool());
      });
  }

  if(hasEdit())
  {
    m_actionEdit = m_toolbar->addAction(Theme::getIcon("edit"), Locale::tr("list:edit"),
      [this]()
      {
        for(const QString& id : getSelectedObjectIds())
          MainWindow::instance->showObject(id);
      });
    m_actionEdit->setEnabled(false);
  }

  if(Method* method = object()->getMethod("remove"))
  {
    m_actionRemove = new MethodAction(Theme::getIcon("remove"), *method,
      [this]()
      {
        for(const QString& id : getSelectedObjectIds())
          callMethod(m_actionRemove->method(), nullptr, id);
      });
    m_actionRemove->setForceDisabled(true);
    m_toolbar->addAction(m_actionRemove);
  }

  if(Method* method = object()->getMethod("delete"))
  {
    m_actionDelete = new MethodAction(Theme::getIcon("delete"), *method,
      [this]()
      {
        for(const QString& id : getSelectedObjectIds())
          callMethod(m_actionDelete->method(),
            [this](std::optional<const Error> error)
            {
              if(error)
              {
                error->show(this);
              }
            },
            id);
      });
    m_actionDelete->setForceDisabled(true);
    m_toolbar->addAction(m_actionDelete);
  }

  if(Method* move = object()->getMethod("move"))
  {
    m_actionMoveUp = new MethodAction(Theme::getIcon("up"), *move,
      [this]()
      {
        if(auto* model = m_tableWidget->selectionModel(); model && model->hasSelection())
        {
          if(const auto rows = model->selectedRows(); !rows.empty())
          {
            const int row = rows[0].row();
            callMethod(m_actionMoveUp->method(), nullptr, row, row - 1);
            m_tableWidget->selectRow(row - 1);
          }
        }
      });
    //Override default method name
    m_actionMoveUp->setText(Locale::tr("list:move_up"));
    m_actionMoveUp->setForceDisabled(true);

    m_actionMoveDown = new MethodAction(Theme::getIcon("down"), *move,
      [this]()
      {
        if(auto* model = m_tableWidget->selectionModel(); model && model->hasSelection())
        {
          if(const auto rows = model->selectedRows(); !rows.empty())
          {
            const int row = rows[0].row();
            callMethod(m_actionMoveDown->method(), nullptr, row, row + 1);
            m_tableWidget->selectRow(row + 1);
          }
        }
      });
    //Override default method name
    m_actionMoveDown->setText(Locale::tr("list:move_down"));
    m_actionMoveDown->setForceDisabled(true);
  }

  if(Method* method = object()->getMethod("reverse"))
  {
    m_actionReverse = new MethodAction(Theme::getIcon("reverse"), *method);
  }

  if(Method* method = object()->getMethod("input_monitor"))
  {
    m_actionInputMonitor = new MethodAction(Theme::getIcon("input_monitor"), *method,
      [this]()
      {
        m_requestIdInputMonitor = m_actionInputMonitor->method().call(
          [](const ObjectPtr& inputMonitor, std::optional<const Error>)
          {
            if(inputMonitor)
              MainWindow::instance->showObject(inputMonitor);
          });
      });
  }

  if(Method* method = object()->getMethod("input_monitor_channel"))
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

        menu->addAction(text, this,
          [this, channel=value.toUInt()]()
          {
            //cancelRequest(m_requestIdInputMonitor);

            m_requestIdInputMonitor = callMethodR<ObjectPtr>(m_actionInputMonitorChannel->method(),
              [this](const ObjectPtr& inputMonitor, std::optional<const Error> /*error*/)
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

  if(Method* method = object()->getMethod("output_keyboard"))
  {
    const auto values = method->getAttribute(AttributeName::Values, QVariant()).toList();
    if(values.size() > 1)
    {
      QMenu* menu = new QMenu(this);
      for(const auto& value : values)
      {
        menu->addAction(translateEnum(EnumName<OutputChannel>::value, value.toLongLong()), this,
          [this, channel=static_cast<OutputChannel>(value.toUInt())]()
          {
            //cancelRequest(m_requestIdOutputKeyboard);

            m_requestIdOutputKeyboard = callMethodR<ObjectPtr>(m_actionOutputKeyboard->method(),
              [this](const ObjectPtr& outputKeyboard, std::optional<const Error> /*error*/)
              {
                m_requestIdOutputKeyboard = Connection::invalidRequestId;
                if(outputKeyboard)
                  MainWindow::instance->showObject(outputKeyboard);
              },
              channel);
          });
      }

      m_actionOutputKeyboard = new MethodAction(Theme::getIcon("output_keyboard"), *method);
      m_actionOutputKeyboard->setMenu(menu);
    }
    else if(!values.empty())
    {
      m_actionOutputKeyboard = new MethodAction(Theme::getIcon("output_keyboard"), *method,
          [this, channel=static_cast<OutputChannel>(values.first().toUInt())]()
          {
            //cancelRequest(m_requestIdOutputKeyboard);

            m_requestIdOutputKeyboard = callMethodR<ObjectPtr>(m_actionOutputKeyboard->method(),
              [this](const ObjectPtr& outputKeyboard, std::optional<const Error> /*error*/)
              {
                m_requestIdOutputKeyboard = Connection::invalidRequestId;
                if(outputKeyboard)
                  MainWindow::instance->showObject(outputKeyboard);
              },
              channel);
          });
    }
  }

  if(m_actionMoveUp || m_actionMoveDown)
  {
    m_toolbar->addSeparator();
    if(m_actionMoveUp)
      m_toolbar->addAction(m_actionMoveUp);
    if(m_actionMoveDown)
      m_toolbar->addAction(m_actionMoveDown);
  }

  if(m_actionReverse)
  {
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_actionReverse);
  }

  if(m_actionInputMonitor || m_actionInputMonitorChannel || m_actionOutputKeyboard)
  {
    if(!m_toolbar->actions().empty())
    {
      m_toolbar->addSeparator();
    }
    if(m_actionInputMonitor)
      m_toolbar->addAction(m_actionInputMonitor);
    if(m_actionInputMonitorChannel)
    {
      m_toolbar->addAction(m_actionInputMonitorChannel);
      if(auto* button = qobject_cast<QToolButton*>(m_toolbar->widgetForAction(m_actionInputMonitorChannel)))
        connect(m_actionInputMonitorChannel, &QAction::triggered, button, &QToolButton::showMenu);
    }
    if(m_actionOutputKeyboard)
    {
      m_toolbar->addAction(m_actionOutputKeyboard);
      if(auto* button = qobject_cast<QToolButton*>(m_toolbar->widgetForAction(m_actionOutputKeyboard)))
        connect(m_actionOutputKeyboard, &QAction::triggered, button, &QToolButton::showMenu);
    }
  }

  {
    QAction* startAll = nullptr;
    if(Method* method = object()->getMethod("start_all"))
      startAll = new MethodAction(Theme::getIcon("run"), *method);

    QAction* stopAll = nullptr;
    if(Method* method = object()->getMethod("stop_all"))
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

  if(auto* method = object()->getMethod("clear_persistent_variables"))
  {
    QWidget* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacer->show();
    m_toolbar->addWidget(spacer);

    m_toolbar->addAction(new MethodAction(Theme::getIcon("clear_persistent_variables"), *method, this));
  }

  if(!m_toolbar->actions().empty())
  {
    static_cast<QVBoxLayout*>(this->layout())->insertWidget(0, m_toolbar);
  }
  else
  {
    delete m_toolbar;
    m_toolbar = nullptr;
  }
}

ObjectListWidget::~ObjectListWidget()
{
  object()->connection()->cancelRequest(m_requestIdCreate);
  object()->connection()->cancelRequest(m_requestIdInputMonitor);
  object()->connection()->cancelRequest(m_requestIdOutputKeyboard);
}

void ObjectListWidget::objectDoubleClicked(const QString& id)
{
  if(hasEdit())
  {
    MainWindow::instance->showObject(id);
  }
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

void ObjectListWidget::objectCreated(const ObjectPtr& object)
{
  MainWindow::instance->showObject(object);
}

void ObjectListWidget::tableSelectionChanged()
{
  tableSelectionChanged(m_tableWidget->selectionModel() && m_tableWidget->selectionModel()->hasSelection());
}

void ObjectListWidget::tableSelectionChanged(bool hasSelection)
{
  if(m_actionEdit)
    m_actionEdit->setEnabled(hasSelection);
  if(m_actionRemove)
    m_actionRemove->setForceDisabled(!hasSelection);
  if(m_actionDelete)
    m_actionDelete->setForceDisabled(!hasSelection);
  if(m_actionMoveUp || m_actionMoveDown)
  {
    const int selectedRow = m_tableWidget->selectionModel() && m_tableWidget->selectionModel()->selectedRows().length() == 1 ? m_tableWidget->selectionModel()->selectedRows()[0].row() : -1;
    if(m_actionMoveUp)
      m_actionMoveUp->setForceDisabled(!hasSelection || selectedRow == 0);
    if(m_actionMoveDown)
      m_actionMoveDown->setForceDisabled(!hasSelection || selectedRow == (m_tableWidget->model()->rowCount() - 1));
  }
}

bool ObjectListWidget::hasEdit() const
{
  if(object()->classId() == "list.output")
  {
    return false;
  }
  return true;
}
