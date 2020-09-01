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
#include "stylesheetparser/stylesheetedit.h"

namespace StylesheetParser {

//=== StylesheetEdit ================================================================
StylesheetEdit::StylesheetEdit(QWidget* parent)
   : QTextEdit(parent)
   , m_parser(new Parser(this))
   , m_highlighter(new StylesheetHighligter(this))
{
}

void StylesheetEdit::setText(const QString& text)
{
   ParserState* state = m_parser->parse(text);
   QTextEdit::setText(text);

   if (!state->errors().testFlag(ParserState::NoError)) {
      // TODO error recovery
   }
}

QString StylesheetEdit::text()
{
   return toPlainText();
}

QList<Node*>* StylesheetEdit::nodes()
{
   return m_parser->nodes();
}

void StylesheetEdit::setNormalFormat(QColor color, QFont::Weight weight)
{
   m_highlighter->setNormalFormat(color, weight);
}

void StylesheetEdit::setNormalFormat(Qt::GlobalColor color, QFont::Weight weight)
{
   m_highlighter->setNormalFormat(color, weight);
}

void StylesheetEdit::setNameFormat(QColor color, QFont::Weight weight)
{
   m_highlighter->setNameFormat(color, weight);
}

void StylesheetEdit::setNameFormat(Qt::GlobalColor color, QFont::Weight weight)
{
   m_highlighter->setNameFormat(color, weight);
}

void StylesheetEdit::setValueFormat(QColor color, QFont::Weight weight)
{
   m_highlighter->setValueFormat(color, weight);
}

void StylesheetEdit::setValueFormat(Qt::GlobalColor color, QFont::Weight weight)
{
   m_highlighter->setValueFormat(color, weight);
}







} // end of StylesheetParser
