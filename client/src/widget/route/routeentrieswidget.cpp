/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "routeentrieswidget.hpp"
#include "routeentriesmodel.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolBar>
#include <QListView>
#include <QItemDelegate>
#include <QStackedWidget>
#include <QPainter>
#include <QApplication>
#include <traintastic/enum/trainrouteentrysource.hpp>
#include "../createform.hpp"
#include "../../network/callmethod.hpp"
#include "../../network/method.hpp"
#include "../../network/object.hpp"
#include "../../dialog/objectselectlistdialog.hpp"
#include "../../misc/methodaction.hpp"
#include "../../theme/theme.hpp"

namespace {

class RouteEntriesListItemDelegate : public QItemDelegate
{
public:
  inline RouteEntriesListItemDelegate(QListView* parent)
    : QItemDelegate(parent)
  {
  }

  inline void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const final
  {
    const auto palette = QApplication::palette();
    const auto textColor = palette.color(QPalette::Active, QPalette::Text);
    const auto textColorDisabled = palette.color(QPalette::Disabled, QPalette::Text);
    auto iconRect = option.rect;
    iconRect.setWidth(option.rect.height());
    const int iconLineMargin = 3 * iconRect.width() / 7;

    auto* model = qobject_cast<QListView*>(parent())->model();
    const auto name = model->data(index, Qt::DisplayRole).toString();

    QTextOption textOption;
    textOption.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    if((option.state & QStyle::State_Selected) != 0)
    {
      painter->fillRect(option.rect, palette.brush(QPalette::Highlight));
    }

    painter->setRenderHint(QPainter::Antialiasing, true);

    painter->setPen(textColor);
    painter->setBrush(textColor);

    switch(static_cast<RouteEntriesModel::Icon>(model->data(index, RouteEntriesModel::iconRole).toUInt()))
    {
      using enum RouteEntriesModel::Icon;

      case None:
        break;

      case First:
        painter->fillRect(iconRect.adjusted(iconLineMargin, iconLineMargin, -iconLineMargin, 0), textColor);
        painter->fillRect(iconRect.adjusted(iconLineMargin / 2, iconLineMargin, -iconLineMargin / 2, -iconLineMargin), textColor);
        break;

      case LineWithDot:
      {
        const double r = iconRect.width() / 4.0;
        painter->drawEllipse(iconRect.center().toPointF(), r, r);
        [[fallthrough]];
      }
      case Line:
      {
        painter->fillRect(iconRect.adjusted(iconLineMargin, 0, -iconLineMargin, 0), textColor);
        break;
      }
      case Last:
        painter->fillRect(iconRect.adjusted(iconLineMargin, 0, -iconLineMargin, -iconLineMargin), textColor);
        painter->fillRect(iconRect.adjusted(iconLineMargin / 2, iconLineMargin, -iconLineMargin / 2, -iconLineMargin), textColor);
        break;
    }

    const auto source = static_cast<TrainRouteEntrySource>(model->data(index, RouteEntriesModel::sourceRole).toUInt());
    if(source == TrainRouteEntrySource::Resolver)
    {
      painter->setPen(textColorDisabled);
    }

    painter->drawText(option.rect.adjusted(iconRect.width() + iconRect.width() / 8, 0, 0, 0), name, textOption);
  }
};

}

RouteEntriesWidget::RouteEntriesWidget(ObjectPtr object, QWidget* parent)
  : QWidget(parent)
  , m_object{std::move(object)}
  , m_methodAddBlock{m_object->getMethod("add_block")}
  , m_methodRemove{m_object->getMethod("remove")}
  , m_list{new QListView(this)}
  , m_stack{new QStackedWidget(this)}
{
  auto* toolbar = new QToolBar(this);

  if(m_methodAddBlock)
  {
    toolbar->addAction(new MethodAction(Theme::getIcon("add"), *m_methodAddBlock,
      [this]()
      {
        std::make_unique<ObjectSelectListDialog>(*m_methodAddBlock, false, this)->exec();
      }, toolbar));
  }
  if(m_methodRemove)
  {
    m_actionRemove = new MethodAction(Theme::getIcon("remove"), *m_methodRemove,
      [this]()
      {
        if(auto index = m_list->currentIndex(); index.isValid())
        {
          callMethod(*m_methodRemove, nullptr, static_cast<RouteEntriesModel*>(m_list->model())->getObject(index.row()));
        }
      }, toolbar);
    m_actionRemove->setForceDisabled(true);
    toolbar->addAction(m_actionRemove);
  }

  m_list->setItemDelegate(new RouteEntriesListItemDelegate(m_list));
  m_list->setModel(new RouteEntriesModel(m_object, m_list));

  m_stack->addWidget(new QWidget(this)); // empty page

  connect(m_list->selectionModel(), &QItemSelectionModel::selectionChanged, this,
    [this](const QItemSelection& selected, const QItemSelection& /*deselected*/)
    {
      const bool hasSelection = !selected.isEmpty();
      const auto source = static_cast<TrainRouteEntrySource>(hasSelection ? m_list->model()->data(selected.indexes().first(), RouteEntriesModel::sourceRole).toUInt() : 0);

      QWidget* page = nullptr;

      if(source == TrainRouteEntrySource::Explicit)
      {
        if(auto entry = static_cast<RouteEntriesModel*>(m_list->model())->getObject(selected.indexes().first().row()))
        {
          if(auto it = m_routeEntryWidgets.find(entry.get()); it != m_routeEntryWidgets.end())
          {
            page = it->second;
          }
          else
          {
            page = createFormWidget(*entry, this);
            connect(entry.get(), &Object::dead, page,
              [page]()
              {
                delete page;
              });
            m_stack->addWidget(page);
            m_routeEntryWidgets.emplace(entry.get(), page);
          }
        }
      }

      if(page)
      {
        m_stack->setCurrentWidget(page);
      }
      else
      {
        m_stack->setCurrentIndex(0); // empty page
      }

      if(m_actionRemove) [[likely]]
      {
        m_actionRemove->setForceDisabled(!hasSelection || source == TrainRouteEntrySource::Resolver || m_list->model()->rowCount() == 0);
      }
    });
  connect(m_list->model(), &QAbstractListModel::modelReset,
    [this]()
    {
      if(m_list->model()->rowCount() == 0)
      {
        m_actionRemove->setForceDisabled(true);
      }
    });

  auto* main = new QHBoxLayout(this);
  auto* left = new QVBoxLayout();
  left->addWidget(toolbar);
  left->addWidget(m_list);
  main->addLayout(left);
  main->addWidget(m_stack);
}
