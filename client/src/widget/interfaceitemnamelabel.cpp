/**
 * client/src/widget/interfaceitemnamelabel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2024 Reinder Feenstra
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

#include "interfaceitemnamelabel.hpp"
#include "../network/interfaceitem.hpp"
#include "../theme/theme.hpp"

InterfaceItemNameLabel::InterfaceItemNameLabel(InterfaceItem& item, QWidget* parent)
    : QWidget(parent), m_item(item)
{
    m_label = new QLabel(m_item.displayName(), this);
    m_label->setVisible(m_item.getAttributeBool(AttributeName::Visible, true));

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    layout->addWidget(m_label);

    const QString help = m_item.helpText();
    if (!help.isEmpty())
    {
        m_helpBtn = new QToolButton(this);
        m_helpBtn->setIcon(Theme::getIcon("help"));   // portable icon
        m_helpBtn->setToolTip(help);
        m_helpBtn->setAutoRaise(true);
        m_helpBtn->setCursor(Qt::PointingHandCursor);
        m_helpBtn->setIconSize(QSize(12,12));          // small

        layout->addWidget(m_helpBtn, 0, Qt::AlignTop);
    }
    else
    {
        m_helpBtn = nullptr;
    }

    layout->addStretch();

    connect(&m_item, &InterfaceItem::attributeChanged, this,
        [this](AttributeName name, const QVariant& value)
        {
            switch(name)
            {
                case AttributeName::Visible:
                    m_label->setVisible(value.toBool());
                    break;
                case AttributeName::DisplayName:
                    m_label->setText(m_item.displayName());
                    break;
                default:
                    break;
            }
        });
}
