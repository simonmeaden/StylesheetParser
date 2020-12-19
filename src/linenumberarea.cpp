/*
  Copyright 2020 Simon Meaden

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
        */
#include "linenumberarea.h"
#include "stylesheetedit_p.h"

LineNumberArea::LineNumberArea(StylesheetEditor* editor)
  : QWidget(editor)
  , m_editor(editor)
  , m_foreSelected(QColor("#808080"))
  , m_foreUnselected(QColor("#C5C5C5"))
  , m_back(QColor("#EEEFEF"))
  , m_currentLineNumber(1)
  , m_left(0)
{}

QSize
LineNumberArea::sizeHint() const
{
  return QSize(m_editor->lineNumberAreaWidth(), 0);
}

void
LineNumberArea::paintEvent(QPaintEvent* event)
{
  QRect rect;
  rect.setLeft(m_left);
  rect.setRight(m_left + event->rect().width());
  rect.setTop(event->rect().top());
  rect.setBottom(event->rect().bottom());
  QTextBlock block = m_editor->firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = qRound(m_editor->blockBoundingGeometry(block)
                     .translated(m_editor->contentOffset())
                     .top());
  int bottom = top + qRound(m_editor->blockBoundingRect(block).height());
  int height = m_editor->fontMetrics().height();
  double blockHeight = m_editor->blockBoundingRect(block).height();

  QPainter painter(this);
  painter.fillRect(rect, back());

  while (block.isValid()) {
    if (top <= rect.bottom()) {
      if (block.isVisible() && bottom >= rect.top()) {
        int number = blockNumber + 1;

        if (number == m_currentLineNumber) {
          painter.setPen(m_foreSelected);

        } else {
          painter.setPen(m_foreUnselected);
        }

        painter.drawText(
          0, top, width(), height, Qt::AlignRight, QString::number(number));
      }
    }

    block = block.next();
    top = bottom;
    bottom = top + qRound(blockHeight);
    ++blockNumber;
  }
}

int
LineNumberArea::left() const
{
  return m_left;
}

void
LineNumberArea::setLeft(int left)
{
  m_left = left;
}

int
LineNumberArea::currentLineNumber() const
{
  return m_currentLineNumber;
}

void
LineNumberArea::mousePressEvent(QMouseEvent* event)
{
  auto tc = m_editor->cursorForPosition(event->pos());
  auto lineNumber = m_editor->calculateLineNumber(tc);
  if (lineNumber >= 1) {
    m_editor->goToLine(lineNumber);
  } else
    QWidget::mousePressEvent(event);
}

void
LineNumberArea::mouseMoveEvent(QMouseEvent* event)
{
  QWidget::mouseMoveEvent(event);
}

void
LineNumberArea::mouseReleaseEvent(QMouseEvent* event)
{
  QWidget::mouseMoveEvent(event);
}

int LineNumberArea::lineCount() const
{
  return m_lineCount;
}

void LineNumberArea::setLineCount(int value)
{
  m_lineCount = value;
}

QFont::Weight
LineNumberArea::weight() const
{
  return m_weight;
}

void
LineNumberArea::setWeight(const QFont::Weight& weight)
{
  m_weight = weight;
  update();
}

QColor
LineNumberArea::back() const
{
  return m_back;
}

void
LineNumberArea::setBack(const QColor& back)
{
  m_back = back;
  update();
}

QColor
LineNumberArea::foreSelected() const
{
  return m_foreSelected;
}

void
LineNumberArea::setForeSelected(const QColor& fore)
{
  m_foreSelected = fore;
  update();
}

QColor
LineNumberArea::foreUnselected() const
{
  return m_foreUnselected;
}

void
LineNumberArea::setForeUnselected(const QColor& fore)
{
  m_foreUnselected = fore;
  update();
}

void
LineNumberArea::setLineNumber(int lineNumber)
{
  m_currentLineNumber = lineNumber;
  update();
}

int
LineNumberArea::lineNumberAreaWidth()
{
  int digits = 1;
  int max = qMax(1, m_editor->blockCount());

  while (max >= 10) {
    max /= 10;
    ++digits;
  }

  int space =
    3 +
    m_editor->fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

  return space;
}
