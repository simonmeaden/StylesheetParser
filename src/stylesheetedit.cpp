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
  , m_highlighter(new StylesheetHighlighter(this))
{
  connect(this, &QTextEdit::cursorPositionChanged, this, &StylesheetEdit::onCursorPositionChanged);
  connect(this, &QTextEdit::textChanged, this, &StylesheetEdit::onTextChanged);
  //  connect(this->document(), &QTextDocument::contentsChange, this, &StylesheetEdit::onDocumentChanged);
}

void StylesheetEdit::setText(const QString& text)
{
  setPlainText(text);
}

void StylesheetEdit::setPlainText(const QString& text)
{
  ParserState* state = m_parser->parse(text);
  QTextEdit::setPlainText(text);

  if (!state->errors().testFlag(ParserState::NoError)) {
    // TODO error recovery
  }
}

NodeList* StylesheetEdit::nodes()
{
  return m_parser->nodes();
}

void StylesheetEdit::showNewlineMarkers(bool show)
{
  // TODO show/hide newline markers.
  // maybe detect position of line end and draw a [NL] box??
}

void StylesheetEdit::showLineNumbers(bool show)
{
  // TODO show/hide line numbers
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
  QTextCursor m_cursor;
  m_highlighter->setValueFormat(color, weight);
}

void StylesheetEdit::setValueFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_highlighter->setValueFormat(color, weight);
}

void StylesheetEdit::setWidgetFormat(QColor color, QFont::Weight weight)
{
  m_highlighter->setWidgetFormat(color, weight);
}

void StylesheetEdit::setWidgetFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_highlighter->setWidgetFormat(color, weight);
}

void StylesheetEdit::setPseudoStateFormat(QColor color, QFont::Weight weight)
{
  m_highlighter->setPseudoStateFormat(color, weight);
}

void StylesheetEdit::setPseudoStateFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_highlighter->setPseudoStateFormat(color, weight);
}

void StylesheetEdit::setPseudoStateMarkerFormat(QColor color, QFont::Weight weight)
{
  m_highlighter->setPseudoStateMarkerFormat(color, weight);
}

void StylesheetEdit::setPseudoStateMarkerFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_highlighter->setPseudoStateMarkerFormat(color, weight);
}

void StylesheetEdit::setSubControlFormat(QColor color, QFont::Weight weight)
{
  m_highlighter->setSubControlFormat(color, weight);
}

void StylesheetEdit::setSubControlFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_highlighter->setSubControlFormat(color, weight);
}

void StylesheetEdit::setSubControlMarkerFormat(QColor color, QFont::Weight weight)
{
  m_highlighter->setSubControlMarkerFormat(color, weight);
}

void StylesheetEdit::setSubControlMarkerFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_highlighter->setSubControlFormat(color, weight);
}

void StylesheetEdit::setPropertyFormat(QColor color, QFont::Weight weight)
{
  m_highlighter->setPropertyFormat(color, weight);
}

void StylesheetEdit::setPropertyFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_highlighter->setPropertyFormat(color, weight);
}

void StylesheetEdit::setPropertyMarkerFormat(QColor color, QFont::Weight weight)
{
  m_highlighter->setPropertyMarkerFormat(color, weight);
}

void StylesheetEdit::setPropertyMarkerFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_highlighter->setPropertyMarkerFormat(color, weight);
}

void StylesheetEdit::onTextChanged()
{
  if (!m_node) {
    return;
  }

  int anchor = m_cursor.anchor();
  int position = m_cursor.position();
  QString text = toPlainText();
  QString value, oldValue;
  QChar c;
  int start = m_node->start();
  int end, length;

  if (m_node->type() == Node::ValueType) {
    ValueNode* valNode = qobject_cast<ValueNode*>(m_node);


  } else {
    switch (m_node->type()) {
    case Node::WidgetType:
    case Node::PropertyType:
      oldValue = qobject_cast<BaseNode*>(m_node)->value();
      break;

    case Node::SubControlType:
    case Node::PseudoStateType:
      oldValue = qobject_cast<NameNode*>(m_node)->value();
      break;

    case Node::SubControlMarkerType:
      oldValue = "::";
      break;

    case Node::PseudoStateMarkerType:
      oldValue = ":";
      break;

    case Node::StartBraceType:
      oldValue = "{";
      break;

    case Node::EndBraceType:
      oldValue = "}";
      break;

    case Node::SemiColonType:
      oldValue = ";";
      break;

    default:
      break;
    }

    length = m_node->length();
    end = start + length;

    if (position == anchor) {
      // no selection
      for (int p = anchor; p >= start; p--) {
        c = text.at(p);

        if (c.isLetterOrNumber()) {
          value.prepend(c);
        }
      }

      for (int p = anchor; p < end; p++) {
        c = text.at(p);

        if (c.isLetterOrNumber()) {
          value.append(c);
        }
      }

      if (value != oldValue) {
        qWarning() << QString("New value(%1) != old value (%2)").arg(value).arg(oldValue);
      }

    } else {
      // a selection.
    }
  }
}

void StylesheetEdit::onCursorPositionChanged()
{
  m_cursor = textCursor();
  int cursorPos = m_cursor.anchor();
  int start, end;

  for (auto basenode : *nodes()) {
    if (!basenode) {
      return;
    }

    Node* node = basenode;

    while (true) {
      if (node->type() == Node::ValueType) {
        ValueNode* valNode = qobject_cast<ValueNode*>(node);
        QList<int> offsets = valNode->offsets();
        QStringList values = valNode->values();
        int offset, valStart;
        start = node->start();

        for (int i = 0; i < offsets.size(); i++) {
          offset = offsets[i];
          valStart = start + offset;
          end = valStart + values[i].length();

          if (cursorPos >= valStart && cursorPos < end) {
            m_node = node;
            break;
          }
        }

      } else {
        start = node->start();
        int length = node->length();
        end = start + length;

        if (cursorPos >= start && cursorPos < end) {
          m_node = node;
          break;
        }
      }

      if (node->next) {
        node = node->next;

      } else {
        break;
      }
    }
  }
}

//void StylesheetEdit::onDocumentChanged(int pos, int charsRemoved, int charsAdded)
//{
//  //  int start, end, len;
//  //  int count =
//  //  QString text = toPlainText();
//  //  for (auto basenode : *nodes()) {
//  //    Node* node = basenode;
//  //    if (!basenode) return;

//  //    while(true) {
//  //      start = node->start();
//  //      len = node->length();
//  //      end = start ;

//  //      if (pos >= start && pos < end) {
//  //        QString change = text.mid(pos, count);
//  //      }

//  //      if (node->next)
//  //        node = node->next;
//  //    }
//  //  }
//}







} // end of StylesheetParser
