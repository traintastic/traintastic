/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_WIDGET_THROTTLE_THROTTLEBUTTON_HPP
#define TRAINTASTIC_CLIENT_WIDGET_THROTTLE_THROTTLEBUTTON_HPP

#include <QWidget>

class ThrottleButton : public QWidget
{
  Q_OBJECT

  private:
    QString m_resource;
    QString m_text;
    QColor m_color;
    QPoint m_mousePressPos;

  protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

  public:
    explicit ThrottleButton(QWidget* parent = nullptr);
    explicit ThrottleButton(const QString& text_, QWidget* parent = nullptr);

    QSize minimumSizeHint() const override;

    const QColor& color() const;
    void setColor(const QColor& value);

    const QString& resource() const;
    void setResource(const QString& value);

    const QString& text() const;
    void setText(const QString& value);

  signals:
    void pressed();
    void released();
    void clicked();
};

#endif
