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
#include "stylesheetparser/stylesheethighlighter.h"
#include "stylesheetparser/stylesheetedit.h"

namespace StylesheetParser {

StylesheetHighligter::StylesheetHighligter(StylesheetEdit* editor)
  : QSyntaxHighlighter(editor)
{
  m_nodes = editor->nodes();

  setNormalFormat(Qt::black);
  setNameFormat(Qt::blue);
  setWidgetFormat(Qt::blue);
  setPseudoStateFormat(Qt::cyan);
  setPseudoStateMarkerFormat(Qt::green);
  setSubControlFormat(Qt::red);
  setSubControlMarkerFormat(QColor("pink"));
  setValueFormat(Qt::magenta);
}

void StylesheetHighligter::highlightBlock(const QString& text)
{
  if (text.isEmpty() || m_nodes->isEmpty()) {
    return;
  }

  for (auto node : *m_nodes) {
    auto pos = node->start();
    int length;
    Node::Type type = node->type();

    switch (type) {
    case Node::NewlineNodeType:
      break;

    case Node::PseudoStateType:
      length = qobject_cast<PseudoStateNode*>(node)->length();
      setFormat(pos, length, m_pseudoStateFormat);
      break;

    case Node::PseudoStateMarkerType:
      length = qobject_cast<PseudoStateMarkerNode*>(node)->length();
      setFormat(pos, length, m_pseudoStateMarkerFormat);
      break;

    case Node::SubControlType:
      length = qobject_cast<SubControlNode*>(node)->length();
      setFormat(pos, length, m_subControlFormat);
      break;

    case Node::SubControlMarkerType:
      length = qobject_cast<SubControlMarkerNode*>(node)->length();
      setFormat(pos, length, m_subControlMarkerFormat);
      break;

    case Node::WidgetType:
      length = qobject_cast<WidgetNode*>(node)->length();
      setFormat(pos, length, m_widgetFormat);
      break;

    case Node::NameType:
      length = qobject_cast<NameNode*>(node)->length();
      setFormat(pos, length, m_nameFormat);
      break;

    case Node::ValueType:
      length = qobject_cast<ValueNode*>(node)->value().length();
      setFormat(pos, length, m_valueFormat);
      break;

    default:
      length = node->length();
      setFormat(pos, length, m_baseFormat);
    }
  }
}

void StylesheetHighligter::setNormalFormat(QColor color, QFont::Weight weight)
{
  m_baseFormat.setFontWeight(weight);
  m_baseFormat.setForeground(QBrush(color));
}

void StylesheetHighligter::setNormalFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_baseFormat.setFontWeight(weight);
  m_baseFormat.setForeground(color);
}

void StylesheetHighligter::setWidgetFormat(QColor color, QFont::Weight weight)
{
  m_widgetFormat.setFontWeight(weight);
  m_widgetFormat.setForeground(QBrush(color));
}

void StylesheetHighligter::setWidgetFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_widgetFormat.setFontWeight(weight);
  m_widgetFormat.setForeground(color);
}

void StylesheetHighligter::setPseudoStateFormat(QColor color, QFont::Weight weight)
{
  m_pseudoStateFormat.setFontWeight(weight);
  m_pseudoStateFormat.setForeground(QBrush(color));
}

void StylesheetHighligter::setPseudoStateFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_pseudoStateFormat.setFontWeight(weight);
  m_pseudoStateFormat.setForeground(color);
}

void StylesheetHighligter::setPseudoStateMarkerFormat(QColor color, QFont::Weight weight)
{
  m_pseudoStateMarkerFormat.setFontWeight(weight);
  m_pseudoStateMarkerFormat.setForeground(QBrush(color));
}

void StylesheetHighligter::setPseudoStateMarkerFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_pseudoStateMarkerFormat.setFontWeight(weight);
  m_pseudoStateMarkerFormat.setForeground(color);
}

void StylesheetHighligter::setSubControlFormat(QColor color, QFont::Weight weight)
{
  m_subControlFormat.setFontWeight(weight);
  m_subControlFormat.setForeground(QBrush(color));
}

void StylesheetHighligter::setSubControlFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_subControlFormat.setFontWeight(weight);
  m_subControlFormat.setForeground(color);
}

void StylesheetHighligter::setSubControlMarkerFormat(QColor color, QFont::Weight weight)
{
  m_subControlMarkerFormat.setFontWeight(weight);
  m_subControlMarkerFormat.setForeground(QBrush(color));
}

void StylesheetHighligter::setSubControlMarkerFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_subControlMarkerFormat.setFontWeight(weight);
  m_subControlMarkerFormat.setForeground(color);
}

void StylesheetParser::StylesheetHighligter::setValueFormat(QColor color, QFont::Weight weight)
{
  m_valueFormat.setFontWeight(weight);
  m_valueFormat.setForeground(QBrush(color));
}

void StylesheetHighligter::setValueFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_valueFormat.setFontWeight(weight);
  m_valueFormat.setForeground(color);
}

void StylesheetHighligter::setNameFormat(QColor color, QFont::Weight weight)
{
  m_nameFormat.setFontWeight(weight);
  m_nameFormat.setForeground(QBrush(color));
}

void StylesheetHighligter::setNameFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_nameFormat.setFontWeight(weight);
  m_nameFormat.setForeground(color);
}

} // end of StylesheetParser
