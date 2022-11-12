/**
 * client/src/widget/outputmapoutputactionwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#include "outputmapoutputactionwidget.hpp"
#include <QVBoxLayout>
#include "../network/object.hpp"
#include "../network/callmethod.hpp"
#include "../network/property.hpp"
#include "propertycombobox.hpp"

OutputMapOutputActionWidget::OutputMapOutputActionWidget(const ObjectPtr& item, const ObjectPtr& output, QWidget* parent) :
  QWidget(parent),
  m_getOutputActionRequestId{Connection::invalidRequestId}
{
  if(auto* m = item->getMethod("get_output_action"))
  {
    m_getOutputActionRequestId = callMethodR<ObjectPtr>(*m,
      [this](const ObjectPtr& r, Message::ErrorCode ec)
      {
        m_getOutputActionRequestId = Connection::invalidRequestId;
        if(Q_LIKELY(r && !ec))
        {
          m_object = r;

          if(auto* p = dynamic_cast<Property*>(m_object->getProperty("action")))
            layout()->addWidget(new PropertyComboBox(*p, this));
        }
      }, output);
  }

  QVBoxLayout* l = new QVBoxLayout();
  l->setContentsMargins(0, 0, 0, 0);
  setLayout(l);
}
