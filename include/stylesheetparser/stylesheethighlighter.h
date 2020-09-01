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
#ifndef STYLESHEETHIGHLIGHTER_H
#define STYLESHEETHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QList>

#include "stylesheetparser/node.h"

namespace StylesheetParser {

class StylesheetEdit;

class StylesheetHighligter : public QSyntaxHighlighter
{
public:
  explicit StylesheetHighligter(StylesheetEdit* editor);

  void highlightBlock(const QString& text);

  void setNormalFormat(QColor color, QFont::Weight weight = QFont::Normal);
  void setNormalFormat(Qt::GlobalColor color, QFont::Weight weight = QFont::Normal);
  void setWidgetFormat(QColor color, QFont::Weight weight = QFont::Normal);
  void setWidgetFormat(Qt::GlobalColor color, QFont::Weight weight = QFont::Normal);
  void setPseudoStateFormat(QColor color, QFont::Weight weight = QFont::Normal);
  void setPseudoStateFormat(Qt::GlobalColor color, QFont::Weight weight = QFont::Normal);
  void setPseudoStateMarkerFormat(QColor color, QFont::Weight weight = QFont::Normal);
  void setPseudoStateMarkerFormat(Qt::GlobalColor color, QFont::Weight weight = QFont::Normal);
  void setSubControlFormat(QColor color, QFont::Weight weight = QFont::Normal);
  void setSubControlFormat(Qt::GlobalColor color, QFont::Weight weight = QFont::Normal);
  void setSubControlMarkerFormat(QColor color, QFont::Weight weight = QFont::Normal);
  void setSubControlMarkerFormat(Qt::GlobalColor color, QFont::Weight weight = QFont::Normal);
  void setNameFormat(QColor color, QFont::Weight weight = QFont::Normal);
  void setNameFormat(Qt::GlobalColor color, QFont::Weight weight = QFont::Normal);
  void setValueFormat(QColor color, QFont::Weight weight = QFont::Normal);
  void setValueFormat(Qt::GlobalColor color, QFont::Weight weight = QFont::Normal);

private:
  QList<Node*>* m_nodes;
  QTextCharFormat m_baseFormat, m_widgetFormat, m_nameFormat, m_valueFormat,
                  m_pseudoStateFormat, m_pseudoStateMarkerFormat, m_subControlFormat,
                  m_subControlMarkerFormat;
};

} // end of StylesheetParser

#endif // STYLESHEETHIGHLIGHTER_H
