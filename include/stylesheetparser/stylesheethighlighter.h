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

namespace StylesheetEditor {

class StylesheetEdit;

class StylesheetHighlighter : public QSyntaxHighlighter
{
public:
  explicit StylesheetHighlighter(StylesheetEdit* editor);

  void highlightBlock(const QString& text);

  void setNormalFormat(QColor color, QColor back, QFont::Weight weight);
  void setWidgetFormat(QColor color, QColor back, QFont::Weight weight);
  void setPseudoStateFormat(QColor color, QColor back, QFont::Weight weight);
  void setPseudoStateMarkerFormat(QColor color, QColor back, QFont::Weight weight);
  void setSubControlFormat(QColor color, QColor back, QFont::Weight weight);
  void setSubControlMarkerFormat(QColor color, QColor back, QFont::Weight weight);
  void setValueFormat(QColor color, QColor back, QFont::Weight weight);
  void setBadValueFormat(QColor color, QColor back, QFont::Weight weight, bool underline,
                         QTextCharFormat::UnderlineStyle underlineStyle,
                         QColor underlineColor);
  void setPropertyFormat(QColor color, QColor back, QFont::Weight weight);
  void setPropertyMarkerFormat(QColor color, QColor back, QFont::Weight weight);
  void setStartBraceFormat(QColor color, QColor back, QFont::Weight weight);
  void setEndBraceFormat(QColor color, QColor back, QFont::Weight weight);
  void setBraceMatchFormat(QColor color, QColor back, QFont::Weight weight);
  void setCommentFormat(QColor color, QColor back, QFont::Weight weight);

private:
  StylesheetEdit* m_editor;
  //  NodeList* m_nodes;
  QTextCharFormat m_baseFormat, m_widgetFormat, m_nameFormat, m_valueFormat, m_badValueFormat,
                  m_pseudoStateFormat, m_pseudoStateMarkerFormat, m_subControlFormat,
                  m_subControlMarkerFormat, m_propertyFormat, m_propertyMarkerFormat,
    m_startBraceFormat, m_endBraceFormat,
    m_braceMatchFormat, m_commentFormat;

  int setNodeEnd(int nodeEnd, int blockEnd);
  int setNodeStart(int nodeStart, int blockStart);
};

} // end of StylesheetParser

#endif // STYLESHEETHIGHLIGHTER_H
