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
#ifndef HOVERWIDGET_H
#define HOVERWIDGET_H

#include <QWidget>
#include <QPainter>

class HoverWidget : public QWidget
{
public:
  HoverWidget(QWidget* parent = nullptr);

  void show(QPoint pos, const QString& text, QColor foreground = QColor("INVALID"),
            QColor background = QColor("INVALID"));

  QColor fore() const;
  void setFore(const QColor& fore);

  QColor defaultBackground() const;
  void setDefaultBackground(const QColor& background);

protected:
  void paintEvent(QPaintEvent*) override;

private:
  QStringList m_text;
  QList<int> m_heights;
  int m_height, m_width;
  QColor m_defForeground, m_foreground;
  QColor m_defBackground, m_background;
};

#endif // HOVERWIDGET_H
