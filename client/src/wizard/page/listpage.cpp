/**
 * client/src/wizard/page/listpage.cpp
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

#include "listpage.hpp"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <traintastic/locale/locale.hpp>

constexpr int originalIndexRole = Qt::UserRole;

ListPage::ListPage(QWidget* parent)
  : TextPage(parent)
  , m_searchBar{new QLineEdit(this)}
  , m_list{new QListWidget(this)}
{
  m_searchBar->setPlaceholderText(Locale::tr("wizard.page.list:search"));
  connect(m_searchBar, &QLineEdit::textChanged, this,
    [this](const QString& /*s*/)
    {
      updateItems();
    });

  auto* l = static_cast<QVBoxLayout*>(layout());
  l->addWidget(m_searchBar);
  l->addWidget(m_list);
}

int ListPage::selectedItemIndex() const
{
  if(const int row = m_list->currentIndex().row(); row >= 0)
  {
    return m_list->item(row)->data(originalIndexRole).toInt();
  }
  return -1; // none
}

void ListPage::setItems(QStringList items)
{
  m_items = std::move(items);
  updateItems();
}

void ListPage::updateItems()
{
  const auto filter = m_searchBar->text();

  m_list->clear();
  for(int i = 0; i < m_items.size(); ++i)
  {
    if(filter.isEmpty() || m_items[i].contains(filter, Qt::CaseInsensitive))
    {
      auto* item = new QListWidgetItem(m_items[i]);
      item->setData(originalIndexRole, i);
      m_list->addItem(item);
    }
  }
}
