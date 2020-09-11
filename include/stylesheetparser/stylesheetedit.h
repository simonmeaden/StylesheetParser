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
#ifndef STYLESHEETEDIT_H
#define STYLESHEETEDIT_H

//#include <QTextEdit>
#include <QPlainTextEdit>

#include "parser.h"
#include "stylesheetparser/stylesheethighlighter.h"

namespace StylesheetParser {

class StylesheetEdit : public /*QPlainTextEdit*/ QTextEdit
{

public:
   explicit StylesheetEdit(QWidget* parent = nullptr);

   void setText(const QString& text);
   void setPlainText(const QString& text);

   NodeList *nodes();

   void showNewlineMarkers(bool show);

   //! Sets a new color/fontweight pair for the highlighter base format
   void setNormalFormat(QColor color, QFont::Weight weight=QFont::Normal);
   //! Sets a new color/fontweight pair for the highlighter base format
   void setNormalFormat(Qt::GlobalColor color, QFont::Weight weight=QFont::Normal);
   //! Sets a new color/fontweight pair for the highlighter name format
   void setNameFormat(QColor color, QFont::Weight weight=QFont::Normal);
   //! Sets a new color/fontweight pair for the highlighter name format
   void setNameFormat(Qt::GlobalColor color, QFont::Weight weight=QFont::Normal);
   //! Sets a new color/fontweight pair for the highlighter value format
   void setValueFormat(QColor color, QFont::Weight weight=QFont::Normal);
   //! Sets a new color/fontweight pair for the highlighter value format
   void setValueFormat(Qt::GlobalColor color, QFont::Weight weight=QFont::Normal);
   //! Sets a new color/fontweight pair for the highlighter widget name format
   void setWidgetFormat(QColor color, QFont::Weight weight = QFont::Normal);
   //! Sets a new color/fontweight pair for the highlighter widget name format
   void setWidgetFormat(Qt::GlobalColor color, QFont::Weight weight = QFont::Normal);
   //! Sets a new color/fontweight pair for the highlighter pseudo state format
   void setPseudoStateFormat(QColor color, QFont::Weight weight = QFont::Normal);
   //! Sets a new color/fontweight pair for the highlighter pseudo state format
   void setPseudoStateFormat(Qt::GlobalColor color, QFont::Weight weight = QFont::Normal);
   //! Sets a new color/fontweight pair for the highlighter pseudo state marker (:) format
   void setPseudoStateMarkerFormat(QColor color, QFont::Weight weight = QFont::Normal);
   //! Sets a new color/fontweight pair for the highlighter pseudo state marker (:) format
   void setPseudoStateMarkerFormat(Qt::GlobalColor color, QFont::Weight weight = QFont::Normal);
   //! Sets a new color/fontweight pair for the highlighter sub control format
   void setSubControlFormat(QColor color, QFont::Weight weight = QFont::Normal);
   //! Sets a new color/fontweight pair for the highlighter sub control format
   void setSubControlFormat(Qt::GlobalColor color, QFont::Weight weight = QFont::Normal);
   //! Sets a new color/fontweight pair for the highlighter sub control marker (::) format
   void setSubControlMarkerFormat(QColor color, QFont::Weight weight = QFont::Normal);
   //! Sets a new color/fontweight pair for the highlighter sub control marker (::) format
   void setSubControlMarkerFormat(Qt::GlobalColor color, QFont::Weight weight = QFont::Normal);
   void setPropertyFormat(QColor color, QFont::Weight weight = QFont::Normal);
   void setPropertyFormat(Qt::GlobalColor color, QFont::Weight weight = QFont::Normal);
   void setPropertyMarkerFormat(QColor color, QFont::Weight weight = QFont::Normal);
   void setPropertyMarkerFormat(Qt::GlobalColor color, QFont::Weight weight = QFont::Normal);

private:
   Parser* m_parser;
   StylesheetHighlighter* m_highlighter;
   QTextCursor m_cursor;
   Node* m_node;
   int m_cursorPos;

   void onTextChanged();
   void onCursorPositionChanged();
//   void onDocumentChanged(int pos, int charsRemoved, int charsAdded);

};

} // end of StylesheetParser

#endif // STYLESHEETEDIT_H
