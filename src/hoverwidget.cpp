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
#include "hoverwidget.h"

HoverWidget::HoverWidget(QWidget* parent)
  : QWidget(parent)
  , m_hOffset(40)
  , m_vOffset(0)
  , m_defForeground(Qt::red)
  , m_defBackground(QColor("#80FFE4E1"))
{}

void HoverWidget::paintEvent(QPaintEvent* /*event*/)
{
  if (m_width == 0 || m_height == 0) {
    return;
  }

  QPainter painter(this);
  painter.fillRect(m_hOffset, m_vOffset, m_width, m_height, m_background);
  painter.setPen(m_foreground);
  auto bottom = m_vOffset - 3;

  for (auto i = 0; i < m_text.count(); i++) {
    QString t = m_text.at(i);
    int h = m_heights.at(i);
    bottom += h;

    painter.drawText(m_hOffset + 5, bottom, t);
  }
}

int HoverWidget::verticalOffset() const
{
  return m_vOffset;
}

void HoverWidget::setVerticalOffset(int vOffset)
{
  m_vOffset = vOffset;
}

int HoverWidget::horizontalOffset() const
{
  return m_hOffset;
}

void HoverWidget::setHorizontalOffset(int offset)
{
  m_hOffset = offset;
}

void HoverWidget::setPosition(QPoint pos)
{
  setGeometry(
    pos.x() + m_hOffset, pos.y(), m_width, m_height);
  update();
}

QColor HoverWidget::defaultBackground() const
{
  return m_defBackground;
}

void HoverWidget::setDefaultBackground(const QColor& background)
{
  m_defBackground = background;
}

void HoverWidget::showHover(QPoint pos)
{
  update();
}

void HoverWidget::hideHover()
{
  m_width = m_height = 0;
  update();
}

QColor HoverWidget::defaultForeground() const
{
  return m_defForeground;
}

void HoverWidget::setDefaultForeground(const QColor& fore)
{
  m_defForeground = fore;
}

void HoverWidget::setHoverText(const QString& text, QColor foreground, QColor background)
{
  m_foreground = (foreground.isValid() ? foreground : m_defForeground);
  m_background = (background.isValid() ? background : m_defBackground);
  m_text = text.split("\n");
  QFontMetrics fm = fontMetrics();
  m_width = m_height = 0;
  m_heights.clear();

  for (auto t : m_text) {
    QRect rect = fm.boundingRect(t);
    m_width = (rect.width() > m_width ? rect.width() : m_width);
    m_height += rect.height();
    m_heights.append(rect.height());
  }

  m_width += 10;

  update();
}
