/**
 * client/src/widget/ledwidget.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_WIDGET_LEDWIDGET_HPP
#define TRAINTASTIC_CLIENT_WIDGET_LEDWIDGET_HPP

#include <QWidget>

class LEDWidget : public QWidget
{
  Q_OBJECT

  public:
    enum State {
      Undefined,
      Off,
      On,
    };

    struct Colors
    {
      QColor undefined;
      QColor off;
      QColor on;
      QColor border;
    };

  protected:
    const Colors& m_colors;
    bool m_mouseLeftButtonPressed;
    bool m_enabled;
    State m_state;
    QString m_text;

    void mousePressEvent(QMouseEvent* event) final;
    void mouseReleaseEvent(QMouseEvent* event) final;
    void paintEvent(QPaintEvent*) final;

  public:
    explicit LEDWidget(const Colors& colors, QWidget *parent = nullptr);

    bool enabled() const { return m_enabled; }
    void setEnabled(bool value);

    State state() const { return m_state; }
    void setState(State value);

    QString text() const { return m_text; }
    void setText(const QString& value);

   signals:
     void clicked();
};

#endif
