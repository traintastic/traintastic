/**
 * client/src/wizard/page/radiopage.hpp
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

#ifndef TRAINTASTIC_CLIENT_WIZARD_PAGE_RADIOPAGE_HPP
#define TRAINTASTIC_CLIENT_WIZARD_PAGE_RADIOPAGE_HPP

#include "textpage.hpp"

class QButtonGroup;

class RadioPage : public TextPage
{
  protected:
    QButtonGroup* m_group;
    QLabel* m_bottomText;

  public:
    explicit RadioPage(QWidget* parent = nullptr);

    int currentIndex() const;

    void addItem(const QString& label, bool checked = false, bool disabled = false);
    void clear();

    void setBottomText(const QString& text);
};

#endif
