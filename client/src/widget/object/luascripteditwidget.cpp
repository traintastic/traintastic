/**
 * client/src/widget/object/luascripteditwidget.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2025 Reinder Feenstra
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

#include "luascripteditwidget.hpp"
#include <QFormLayout>
#include <QToolBar>
#include <QPlainTextEdit>
#include <QDesktopServices>
#include <QFile>
#include <version.hpp>
#include <traintastic/locale/locale.hpp>
#include <traintastic/utils/standardpaths.hpp>
#include "../../misc/methodaction.hpp"
#include "../../network/object.hpp"
#include "../../network/property.hpp"
#include "../../network/method.hpp"
#include "../propertylineedit.hpp"
#include "../propertycheckbox.hpp"
#include "../propertyluacodeedit.hpp"
#include "../../theme/theme.hpp"

LuaScriptEditWidget::LuaScriptEditWidget(const ObjectPtr& object, QWidget* parent) :
  AbstractEditWidget(object, parent),
  m_propertyState{nullptr},
  m_methodStart{nullptr},
  m_methodStop{nullptr},
  m_start{nullptr},
  m_stop{nullptr}
{
  buildForm();
}

LuaScriptEditWidget::LuaScriptEditWidget(const QString& id, QWidget* parent) :
  AbstractEditWidget(id, parent),
  m_propertyState{nullptr},
  m_methodStart{nullptr},
  m_methodStop{nullptr},
  m_start{nullptr},
  m_stop{nullptr}
{
}

void LuaScriptEditWidget::buildForm()
{
  setObjectWindowTitle();
  Theme::setWindowIcon(*this, m_object->classId());

  m_propertyState = dynamic_cast<Property*>(m_object->getProperty("state"));
  m_methodStart = m_object->getMethod("start");
  m_methodStop = m_object->getMethod("stop");

  if(Q_UNLIKELY(!m_propertyState || !m_methodStart || !m_methodStop))
    return;

  QVBoxLayout* l = new QVBoxLayout();

  QFormLayout* form = new QFormLayout();

  for(const char* name : {"id", "name"})
    if(Property* property = dynamic_cast<Property*>(m_object->getProperty(name)))
      form->addRow(property->displayName(), new PropertyLineEdit(*property, this));

  if(Property* property = dynamic_cast<Property*>(m_object->getProperty("disabled")))
    form->addRow(property->displayName(), new PropertyCheckBox(*property, this));

  QToolBar* toolbar = new QToolBar(this);
  m_start = toolbar->addAction(Theme::getIcon("run"), m_methodStart->displayName(),
    [this]()
    {
      m_methodStart->call();
    });
  m_start->setEnabled(m_methodStart->getAttributeBool(AttributeName::Enabled, true));
  connect(m_methodStart, &Method::attributeChanged, this,
    [this](AttributeName name, const QVariant& value)
    {
      if(name == AttributeName::Enabled)
        m_start->setEnabled(value.toBool());
    });

  m_stop = toolbar->addAction(Theme::getIcon("stop"), m_methodStop->displayName(),
    [this]()
    {
      m_methodStop->call();
    });
  m_stop->setEnabled(m_methodStop->getAttributeBool(AttributeName::Enabled, true));
  connect(m_methodStop, &Method::attributeChanged, this,
    [this](AttributeName name, const QVariant& value)
    {
      if(name == AttributeName::Enabled)
        m_stop->setEnabled(value.toBool());
    });


  if(auto* method = m_object->getMethod("clear_persistent_variables"))
  {
    QWidget* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacer->show();
    toolbar->addWidget(spacer);

    toolbar->addAction(new MethodAction(Theme::getIcon("clear_persistent_variables"), *method, this));
  }

  QWidget* spacer = new QWidget(this);
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  spacer->show();
  toolbar->addWidget(spacer);

  toolbar->addAction(Theme::getIcon("help"), Locale::tr("qtapp.mainmenu:help"),
    []()
    {
      const auto manual = QString::fromStdString((getLuaManualPath() / "index.html").string());
      if(QFile::exists(manual))
        QDesktopServices::openUrl(QUrl::fromLocalFile(manual));
      else
        QDesktopServices::openUrl(QString("https://traintastic.org/manual-lua?version=" TRAINTASTIC_VERSION_FULL));
    });

  l->addLayout(form);

  l->addWidget(toolbar);

  if(Property* property = dynamic_cast<Property*>(m_object->getProperty("code")))
    l->addWidget(new PropertyLuaCodeEdit(*property, this));

  setLayout(l);
}
