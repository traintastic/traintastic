/**
 * client/src/subwindow/subwindow.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_SUBWINDOW_SUBWINDOW_HPP
#define TRAINTASTIC_CLIENT_SUBWINDOW_SUBWINDOW_HPP

#include <QMdiSubWindow>
#include "subwindowtype.hpp"
#include "../network/objectptr.hpp"

class Connection;

class SubWindow : public QMdiSubWindow
{
  Q_OBJECT

  private:
    const SubWindowType m_type;
    std::shared_ptr<Connection> m_connection;
    int m_requestId;
    QString m_id;

    QString settingsGroupName() const;

    void restoreSizeFromSettings();

  protected:
    SubWindow(SubWindowType type, QWidget* parent = nullptr);
    SubWindow(SubWindowType type, std::shared_ptr<Connection> connection, const QString& id, QWidget* parent = nullptr);

    void showEvent(QShowEvent*) final;

    virtual QWidget* createWidget(const ObjectPtr& object) = 0;
    virtual QSize defaultSize() const { return QSize(400, 300); }

    void setObject(const ObjectPtr& object);

  public:
    ~SubWindow() override;

    inline const QString& id() const { return m_id; }
    inline SubWindowType type() const { return m_type; }
    inline QString windowId() const { return windowId(m_type, m_id); }
    inline static QString windowId(SubWindowType type, const QString& id) { return toString(type).append("/").append(id); }

  signals:
    void objectIdChanged(SubWindow* window, const QString& id);
};

#endif
