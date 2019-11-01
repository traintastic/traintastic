/**
 * client/src/widget/alertwidget.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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

#ifndef CLIENT_WIDGET_ALERTWIDGET_HPP
#define CLIENT_WIDGET_ALERTWIDGET_HPP

#include <QWidget>

class AlertWidget : public QWidget
{
  Q_OBJECT

  public:
    enum class Type
    {
      Error,
    };

  protected:
    Type m_type;
    QString m_text;
    QColor m_backgroundColor;
    QColor m_borderColor;
    QColor m_textColor;

    void paintEvent(QPaintEvent* event) final;
    void updateColors();

  public:
    static AlertWidget* error(const QString& text, QWidget* parent = nullptr);

    AlertWidget(QWidget* parent = nullptr);

    Type type() const { return m_type; }
    void setType(Type value);

    const QString& text() const { return m_text; }
    void setText(const QString& value);
};

#endif
