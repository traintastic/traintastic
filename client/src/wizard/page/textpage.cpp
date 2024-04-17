/**
 * client/src/wizard/page/textpage.cpp
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

#include "textpage.hpp"
#include <QVBoxLayout>
#include <QLabel>

TextPage::TextPage(QWidget* parent)
  : WizardPage(parent)
  , m_text{new QLabel(this)}
{
    m_text->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    m_text->setWordWrap(true);
    m_text->setOpenExternalLinks(true);

    QVBoxLayout* l = new QVBoxLayout();
    l->addWidget(m_text);
    setLayout(l);
}

QString TextPage::text() const
{
  return m_text->text();
}

void TextPage::setText(const QString& value)
{
  m_text->setText(value);
}
