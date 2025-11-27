/**
 * client/src/widget/propertyluacodeedit.hpp
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

#ifndef TRAINTASTIC_CLIENT_WIDGET_PROPERTYLUACODEEDIT_HPP
#define TRAINTASTIC_CLIENT_WIDGET_PROPERTYLUACODEEDIT_HPP

#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QRegularExpression>

class Property;

class PropertyLuaCodeEdit : public QPlainTextEdit
{
  private:
    class LineNumbers final: public QWidget
    {
      protected:
        void paintEvent(QPaintEvent* event) final;

      public:
        LineNumbers(PropertyLuaCodeEdit* parent);
        QSize sizeHint() const final;
    };

    class Highlighter final : public QSyntaxHighlighter
    {
      private:
        struct Rule
        {
          QRegularExpression regex;
          QTextCharFormat format;

          Rule()
          {
          }

          Rule(const QString& _regex, const QBrush& color, QFont::Weight fontWeight = QFont::Normal) :
            regex{_regex}
          {
            format.setForeground(color);
            format.setFontWeight(fontWeight);
          }
        };
        QVector<Rule> m_rules;
/*
        QRegularExpression commentStartExpression;
        QRegularExpression commentEndExpression;

        QTextCharFormat keywordFormat;
        QTextCharFormat classFormat;
        QTextCharFormat singleLineCommentFormat;
        QTextCharFormat multiLineCommentFormat;
        QTextCharFormat quotationFormat;
        QTextCharFormat functionFormat;
*/
      protected:
        void highlightBlock(const QString &text) final;

      public:
        Highlighter(QTextDocument* parent = nullptr);
    };

    void indentSelection();
    void unindentSelection();

  protected:
    Property& m_property;
    LineNumbers* m_lineNumbers;

    int calcLineNumbersWidth() const;
    void updateViewportMargins();
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void paintLineNumbers(QPaintEvent* event);

  public:
    PropertyLuaCodeEdit(Property& property, QWidget* parent = nullptr);
};

#endif
