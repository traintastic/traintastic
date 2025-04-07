/**
 * client/src/widget/objectlist/stackedobjectlistwidget.cpp
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

#include "stackedobjectlistwidget.hpp"

#include <QEvent>
#include <QLabel>
#include <QListView>
#include <QMenu>
#include <QMessageBox>
#include <QStackedWidget>
#include <QToolBar>
#include <QVBoxLayout>
#include <QIdentityProxyModel>
#include <QGuiApplication>

#include <traintastic/locale/locale.hpp>

#include "../createwidget.hpp"
#include "../tablewidget.hpp"
#include "../methodicon.hpp"
#include "../../mainwindow.hpp"
#include "../../network/object.hpp"
#include "../../network/method.hpp"
#include "../../network/connection.hpp"
#include "../../network/error.hpp"
#include "../../network/callmethod.hpp"
#include "../../network/tablemodel.hpp"
#include "../../theme/theme.hpp"
#include "../../misc/methodaction.hpp"

namespace
{
  class StackedObjectListProxyModel final : public QIdentityProxyModel
  {
    public:
      StackedObjectListProxyModel(QAbstractItemModel* sourceModel)
        : QIdentityProxyModel()
      {
        setSourceModel(sourceModel);
      }

      QVariant data(const QModelIndex &index, int role) const final
      {
        if (role == Qt::ToolTipRole)
        {
          return Locale::tr("stacked_object_list:click_to_edit_ctrl_click_to_open_in_a_new_window");
        }
        return QIdentityProxyModel::data(index, role);
      }
  };
}

StackedObjectListWidget::StackedObjectListWidget(const ObjectPtr& object, QWidget* parent)
  : QWidget(parent)
  , m_object{object}
  , m_navBar{new QToolBar(this)}
  , m_stack{new QStackedWidget(this)}
  , m_list{new QListView(this)}
  , m_listEmptyLabel{new QLabel(Locale::tr("stacked_object_list:list_is_empty"), m_list)}
  , m_requestId{Connection::invalidRequestId}
{
  m_navBar->hide();

  m_navBar->addAction(Theme::getIcon("previous_page"), Locale::tr("stacked_object_list:back"), this, &StackedObjectListWidget::back);

  m_navLabel = new QLabel(this);
  m_navLabel->setAlignment(Qt::AlignCenter);
  m_navLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  m_navLabel->show();
  m_navBar->addWidget(m_navLabel);

  if(auto* method = object->getMethod("delete"))
  {
    m_actionRemove = new MethodAction(Theme::getIcon("delete"), *method,
      [this]()
      {
        if(!m_listObjectId.isEmpty())
        {
          callMethod(m_actionRemove->method(), nullptr, m_listObjectId);
          back();
        }
      });
    m_navBar->addAction(m_actionRemove);
  }

  connect(m_stack, &QStackedWidget::currentChanged,
    [this](int index)
    {
      m_navBar->setVisible(index > 0);
    });

  m_requestId = object->connection()->getTableModel(object,
    [this](const TableModelPtr& tableModel, std::optional<const Error> error)
    {
      m_requestId = Connection::invalidRequestId;

      if(tableModel)
      {
        m_tableModel = tableModel;
        m_tableModel->setRegionAll(true);
        m_list->setModel(new StackedObjectListProxyModel(m_tableModel.get()));
        connect(m_tableModel.get(), &TableModel::modelReset,
          [this]()
          {
            m_listEmptyLabel->setVisible(m_tableModel->rowCount() == 0);
          });
      }
      else if(error)
      {
        QMessageBox::critical(this, "Error", error->toString());
      }
    });

  if(auto* create = object->getMethod("create"))
  {
    if(create->argumentTypes().size() == 0) // Create method witout argument
    {
      m_create = new MethodIcon(*create, Theme::getIcon("circle/add"), m_list);
    }
    else if(create->argumentTypes().size() == 1)
    {
      m_createMenu = new QMenu(this);
      m_createMenu->installEventFilter(this);

      QStringList classList = create->getAttribute(AttributeName::ClassList, QVariant()).toStringList();
      for(const QString& classId : classList)
      {
        QAction* action = m_createMenu->addAction(Locale::tr("class_id:" + classId));
        action->setData(classId);
        connect(action, &QAction::triggered, this,
          [this, create, action]()
          {
            cancelRequest();

            m_requestId = create->call(action->data().toString(),
              [this](const ObjectPtr& addedObject, std::optional<const Error> error)
              {
                m_requestId = Connection::invalidRequestId;

                if(addedObject)
                {
                  show(addedObject);
                }
                else if(error)
                {
                  QMessageBox::critical(this, "Error", error->toString());
                }
              });
          });
      }

      m_create = new MethodIcon(*create, Theme::getIcon("circle/add"),
        [this]()
        {
          m_createMenu->popup(m_create->mapToGlobal(m_create->rect().topRight()));
        }, m_list);
    }

    m_create->setEnabled(create->getAttributeBool(AttributeName::Enabled, true));
    if(!create->getAttributeBool(AttributeName::Visible, true))
    {
      m_create->hide();
    }

    m_create->installEventFilter(this);
  }

  m_list->installEventFilter(this);
  m_list->setSelectionMode(QListView::NoSelection);
  connect(m_list, &QListView::clicked,
    [this](const QModelIndex &index)
    {
      if(!m_tableModel) /*[[unlikely]]*/
      {
        return;
      }

      const bool openInSubWindow = (QGuiApplication::queryKeyboardModifiers() & Qt::ControlModifier);

      cancelRequest();

      m_requestId = m_object->connection()->getObject(m_tableModel->getRowObjectId(index.row()),
        [this, openInSubWindow](const ObjectPtr& selectedObject, std::optional<const Error> error)
        {
           m_requestId = Connection::invalidRequestId;

          if(selectedObject)
          {
            if(openInSubWindow)
            {
              MainWindow::instance->showObject(selectedObject);
            }
            else
            {
              show(selectedObject);
            }
          }
          else if(error)
          {
            QMessageBox::critical(this, "Error", error->toString());
          }
        });
    });

  m_listEmptyLabel->installEventFilter(this);
  m_listEmptyLabel->setWordWrap(true);

  m_stack->addWidget(m_list);

  auto* l = new QVBoxLayout();
  l->setContentsMargins(0, 0, 0, 0);
  l->addWidget(m_navBar);
  l->addWidget(m_stack);
  setLayout(l);
}

StackedObjectListWidget::~StackedObjectListWidget()
{
  cancelRequest();
}

bool StackedObjectListWidget::eventFilter(QObject* object, QEvent* event)
{
  if(m_listEmptyLabel->isVisible() && ((object == m_list && event->type() == QEvent::Resize) || (object == m_listEmptyLabel && event->type() == QEvent::Show)))
  {
    m_listEmptyLabel->setMaximumWidth(qRound(width() * 0.9f));
    m_listEmptyLabel->adjustSize();
    m_listEmptyLabel->setFixedHeight(m_listEmptyLabel->heightForWidth(m_listEmptyLabel->maximumWidth()));
    m_listEmptyLabel->move((rect().bottomRight() - m_listEmptyLabel->rect().bottomRight()) / 2);
  }
  if(m_create && ((object == m_list && event->type() == QEvent::Resize) || (object == m_create && event->type() == QEvent::Show)))
  {
    auto pnt = m_create->rect().bottomRight();
    pnt = m_list->rect().bottomRight() - pnt - pnt / 3;
    m_create->move(pnt.x(), pnt.y());
  }
  if(m_createMenu && object == m_createMenu && event->type() == QEvent::Show)
  {
    m_createMenu->move(m_createMenu->pos() - QPoint{m_createMenu->width(), m_createMenu->height()});
    return true;
  }
  return QWidget::eventFilter(object, event);
}

void StackedObjectListWidget::cancelRequest()
{
  if(m_requestId != Connection::invalidRequestId)
  {
    m_object->connection()->cancelRequest(m_requestId);
  }
}

void StackedObjectListWidget::back()
{
  if(m_stack->currentIndex() > 0)
  {
    m_listObjectId.clear();
    delete m_stack->currentWidget();
  }
}

void StackedObjectListWidget::show(const ObjectPtr& listObject)
{
  if(auto* w = createWidget(listObject, this)) /*[[likely]]*/
  {
    m_listObjectId = listObject->getPropertyValueString("id");
    connect(w, &QWidget::windowTitleChanged, m_navLabel, &QLabel::setText);
    m_navLabel->setText(w->windowTitle());
    m_stack->setCurrentIndex(m_stack->addWidget(w));
  }
}
