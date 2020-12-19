/*
  Copyright 2020 Simon Meaden

Permission is hereby granted, free of charge, to any person obtaining a copy of this
  software and associated documentation files (the "Software"), to deal in the Software
  without restriction, including without limitation the rights to use, copy, modify, merge,
  publish, distribute, sublicense, and/or sell copies of the Software, and to permit
                                                                         persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or
  substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
    PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
      SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
        */
#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QWidget>
#include <QPainter>

class StylesheetEditor;

class LineNumberArea : public QWidget
{
public:
  LineNumberArea(StylesheetEditor* editor = nullptr);

  QSize sizeHint() const override;

  QColor foreSelected() const;
  void setForeSelected(const QColor& fore);
  QColor foreUnselected() const;
  void setForeUnselected(const QColor& fore);
  QColor back() const;
  void setBack(const QColor& back);

  void setLineNumber(int lineNumber);
  int currentLineNumber() const;
  int lineNumberAreaWidth();

  QFont::Weight weight() const;
  void setWeight(const QFont::Weight& weight);

  int left() const;
  void setLeft(int left);

  int lineCount() const;
  void setLineCount(int value);

protected:
  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

private:
  StylesheetEditor* m_editor;
  QColor m_foreSelected, m_foreUnselected, m_back;
  QFont::Weight m_weight;
  int m_currentLineNumber, m_left, m_lineCount;
};

#endif // LINENUMBERAREA_H
