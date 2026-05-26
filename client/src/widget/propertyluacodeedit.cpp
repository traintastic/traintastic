/**
 * client/src/widget/propertyluacodeedit.cpp
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

#include "propertyluacodeedit.hpp"
#include "../network/property.hpp"
#include <QFontDatabase>
#include <QPainter>
#include <QTextBlock>

namespace {

const auto indent = QStringLiteral("  ");

}

PropertyLuaCodeEdit::PropertyLuaCodeEdit(Property& property, QWidget* parent) :
  QPlainTextEdit(parent),
  m_property{property},
  m_lineNumbers{new LineNumbers(this)}
{
  Q_ASSERT(m_property.type() == ValueType::String);
  setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  setReadOnly(!m_property.getAttributeBool(AttributeName::Enabled, true));
  setVisible(m_property.getAttributeBool(AttributeName::Visible, true));
  new Highlighter(document());
  setPlainText(m_property.toString());
  connect(&m_property, &Property::valueChangedString, this,
    [this](const QString& value)
    {
      if(toPlainText() != value)
        setPlainText(value);
    });
  connect(&m_property, &Property::attributeChanged, this,
    [this](AttributeName name, const QVariant& value)
    {
      switch(name)
      {
        case AttributeName::Enabled:
          setReadOnly(!value.toBool());
          break;

        case AttributeName::Visible:
          setVisible(value.toBool());
          break;

        default:
          break;
      }
    });
  connect(this, &PropertyLuaCodeEdit::textChanged, [this](){ m_property.setValueString(toPlainText()); });
  connect(this, &PropertyLuaCodeEdit::blockCountChanged, [this](int){ updateViewportMargins(); });
  connect(this, &PropertyLuaCodeEdit::updateRequest,
    [this](const QRect& rect, int dy)
    {
      if(dy)
        m_lineNumbers->scroll(0, dy);
      else
        m_lineNumbers->update(0, rect.y(), m_lineNumbers->width(), rect.height());

      if(rect.contains(viewport()->rect()))
        updateViewportMargins();
    });
  updateViewportMargins();
}

int PropertyLuaCodeEdit::calcLineNumbersWidth() const
{
  int chars = 1;
  int max = qMax(1, blockCount());
  while((max /= 10) != 0)
    chars++;

  return chars * fontMetrics().averageCharWidth() + fontMetrics().averageCharWidth() / 2;
}

void PropertyLuaCodeEdit::updateViewportMargins()
{
  setViewportMargins(calcLineNumbersWidth(), 0, 0, 0);
}

void PropertyLuaCodeEdit::keyPressEvent(QKeyEvent* event)
{
  const bool shiftModifier = (event->modifiers() & Qt::ShiftModifier);
  if(event->key() == Qt::Key_Tab && !shiftModifier)
  {
    if(textCursor().hasSelection())
    {
      indentSelection();
    }
    else
    {
      textCursor().insertText(indent);
    }
    return;
  }
  if(event->key() == Qt::Key_Backtab || (event->key() == Qt::Key_Tab && shiftModifier))
  {
    unindentSelection();
    return;
  }
  QPlainTextEdit::keyPressEvent(event);
}

void PropertyLuaCodeEdit::resizeEvent(QResizeEvent* event)
{
  QPlainTextEdit::resizeEvent(event);

  QRect r = contentsRect();
  r.setWidth(calcLineNumbersWidth());
  m_lineNumbers->setGeometry(r);
}

void PropertyLuaCodeEdit::paintLineNumbers(QPaintEvent* event)
{
    QPainter painter(m_lineNumbers);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, m_lineNumbers->width() - fontMetrics().averageCharWidth() / 4, fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void PropertyLuaCodeEdit::indentSelection()
{
  QTextCursor cursor = textCursor();

  if(!cursor.hasSelection())
  {
    return;
  }

  cursor.beginEditBlock();

  const int start = cursor.selectionStart();
  int end = cursor.selectionEnd();

  cursor.setPosition(start);
  cursor.movePosition(QTextCursor::StartOfLine);

  while(cursor.position() < end)
  {
    cursor.insertText(indent);
    end += indent.size();
    cursor.movePosition(QTextCursor::Down);
    cursor.movePosition(QTextCursor::StartOfLine);
  }

  cursor.endEditBlock();
}

void PropertyLuaCodeEdit::unindentSelection()
{
  QTextCursor cursor = textCursor();

  cursor.beginEditBlock();

  const int start = cursor.selectionStart();
  int end = cursor.selectionEnd();

  cursor.setPosition(start);
  cursor.movePosition(QTextCursor::StartOfLine);

  while(cursor.position() < end)
  {
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, indent.size());
    const QString leading = cursor.selectedText();
    cursor.movePosition(QTextCursor::StartOfLine);
    for(auto c : leading)
    {
      if(c != ' ')
      {
        break;
      }
      cursor.deleteChar();
      end--;
    }
    cursor.clearSelection();
    cursor.movePosition(QTextCursor::Down);
  }

  cursor.endEditBlock();
}


PropertyLuaCodeEdit::LineNumbers::LineNumbers(PropertyLuaCodeEdit* parent) :
  QWidget(parent)
{
}

QSize PropertyLuaCodeEdit::LineNumbers::sizeHint() const
{
  return QSize(static_cast<PropertyLuaCodeEdit*>(parent())->calcLineNumbersWidth(), 0);
}

void PropertyLuaCodeEdit::LineNumbers::paintEvent(QPaintEvent* event)
{
  static_cast<PropertyLuaCodeEdit*>(parent())->paintLineNumbers(event);
}


PropertyLuaCodeEdit::Highlighter::Highlighter(QTextDocument* parent) :
  QSyntaxHighlighter(parent)
{
  const auto keywords = {
    QStringLiteral("\\band\\b"),
    QStringLiteral("\\bbreak\\b"),
    QStringLiteral("\\bdo\\b"),
    QStringLiteral("\\belse\\b"),
    QStringLiteral("\\belseif\\b"),
    QStringLiteral("\\bend\\b"),
    QStringLiteral("\\bfalse\\b"),
    QStringLiteral("\\bfor\\b"),
    QStringLiteral("\\bfunction\\b"),
    QStringLiteral("\\bgoto\\b"),
    QStringLiteral("\\bif\\b"),
    QStringLiteral("\\bin\\b"),
    QStringLiteral("\\blocal\\b"),
    QStringLiteral("\\bnil\\b"),
    QStringLiteral("\\bnot\\b"),
    QStringLiteral("\\bor\\b"),
    QStringLiteral("\\brepeat\\b"),
    QStringLiteral("\\breturn\\b"),
    QStringLiteral("\\bthen\\b"),
    QStringLiteral("\\btrue\\b"),
    QStringLiteral("\\buntil\\b"),
    QStringLiteral("\\bwhile\\b"),
  };
  for(const auto& regex : keywords)
    m_rules.append(Rule(regex, Qt::cyan, QFont::Bold));

  const auto numbers = {
    QStringLiteral("\\b(-|\\+|)[0-9]+\\b"), // integer, decimal
    QStringLiteral("\\b(-|\\+|)0(x|X)[0-9a-fA-F]+\\b"), // integer, hex
    QStringLiteral("\\b(|-|\\+)[0-9]+(\\.[0-9]*|)((e|E)(|-|\\+)[0-9]+|)\\b"), // float, decimal
    QStringLiteral("\\b(|-|\\+)0(x|X)([0-9a-fA-F]*\\.[0-9a-fA-F]*|[0-9a-fA-F]+)((p|P)(|-|\\+)[0-9a-fA-F]+|)\\b"), // float, hex
  };
  for(const auto& regex : numbers)
    m_rules.append(Rule(regex, Qt::magenta));

  const auto globals = {
    QStringLiteral("\\bmath(?=\\.)"),
    QStringLiteral("\\bstring(?=\\.)"),
    QStringLiteral("\\btable(?=\\.)"),
    QStringLiteral("\\blog(?=\\.)"),
    QStringLiteral("\\bworld(?=\\.)"),
    QStringLiteral("\\bset(?=\\.)"),
    QStringLiteral("\\benum(?=\\.)"),
    QStringLiteral("\\bpv(?=\\.)"),
  };
  for(const auto& regex : globals)
    m_rules.append(Rule(regex, QColor(0xFF, 0x8C, 0x00)));

  // function
  m_rules.append(Rule("\\b[a-zA-Z_][a-zA-Z0-9_]*(?=\\()", QColor(0x66, 0xCD, 0xAA)));

  // string
  m_rules.append(Rule("(\".*?[^\\\\]\"|\"\")", Qt::darkGreen));

  // single line comment
  m_rules.append(Rule("--+.*", Qt::gray));

  /*
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    const QString keywordPatterns[] = {
        QStringLiteral("\\bchar\\b"), QStringLiteral("\\bclass\\b"), QStringLiteral("\\bconst\\b"),
        QStringLiteral("\\bdouble\\b"), QStringLiteral("\\benum\\b"), QStringLiteral("\\bexplicit\\b"),
        QStringLiteral("\\bfriend\\b"), QStringLiteral("\\binline\\b"), QStringLiteral("\\bint\\b"),
        QStringLiteral("\\blong\\b"), QStringLiteral("\\bnamespace\\b"), QStringLiteral("\\boperator\\b"),
        QStringLiteral("\\bprivate\\b"), QStringLiteral("\\bprotected\\b"), QStringLiteral("\\bpublic\\b"),
        QStringLiteral("\\bshort\\b"), QStringLiteral("\\bsignals\\b"), QStringLiteral("\\bsigned\\b"),
        QStringLiteral("\\bslots\\b"), QStringLiteral("\\bstatic\\b"), QStringLiteral("\\bstruct\\b"),
        QStringLiteral("\\btemplate\\b"), QStringLiteral("\\btypedef\\b"), QStringLiteral("\\btypename\\b"),
        QStringLiteral("\\bunion\\b"), QStringLiteral("\\bunsigned\\b"), QStringLiteral("\\bvirtual\\b"),
        QStringLiteral("\\bvoid\\b"), QStringLiteral("\\bvolatile\\b"), QStringLiteral("\\bbool\\b")
    };
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
//! [0] //! [1]
    }
//! [1]

//! [2]
    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegularExpression(QStringLiteral("\\bQ[A-Za-z]+\\b"));
    rule.format = classFormat;
    highlightingRules.append(rule);
//! [2]

//! [3]
    singleLineCommentFormat.setForeground(Qt::red);
    rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::red);
//! [3]

//! [4]
    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);
//! [4]

//! [5]
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
    rule.format = functionFormat;
    highlightingRules.append(rule);
//! [5]

//! [6]
    commentStartExpression = QRegularExpression(QStringLiteral("/\\*"));
    commentEndExpression = QRegularExpression(QStringLiteral("\\* /")); */
}
//! [6]

//! [7]
void PropertyLuaCodeEdit::Highlighter::highlightBlock(const QString &text)
{
    for (const Rule &rule : std::as_const(m_rules)) {
        QRegularExpressionMatchIterator matchIterator = rule.regex.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
//! [7] //! [8]
    setCurrentBlockState(0);
//! [8]
/*
//! [9]
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

//! [9] //! [10]
    while (startIndex >= 0) {
//! [10] //! [11]
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
    */
}
