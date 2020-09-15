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

StylesheetHighlighter::StylesheetHighlighter(StylesheetEdit *editor)
  : QSyntaxHighlighter(editor->document())
{
  m_nodes = editor->nodes();

  setNormalFormat(Qt::black);
  setNameFormat(Qt::lightGray);
  setWidgetFormat(Qt::blue);
  setPseudoStateFormat(Qt::darkCyan);
  setPseudoStateMarkerFormat(Qt::black);
  setSubControlFormat(Qt::red);
  setSubControlMarkerFormat(Qt::black);
  setValueFormat(Qt::magenta);
  setBadValueFormat(Qt::green);
  setPropertyFormat(Qt::blue);
  setPropertyMarkerFormat(Qt::black);
}

int StylesheetHighlighter::setNodeEnd(int nodeEnd, int blockEnd)
{
  if (blockEnd < nodeEnd) {
    return blockEnd;
  }

  return nodeEnd;
}

int StylesheetHighlighter::setNodeStart(int nodeStart, int blockStart)
{
  if (blockStart > nodeStart) {
    nodeStart = blockStart;
  }

  return nodeStart;
}

void StylesheetHighlighter::highlightBlock(const QString& text)
{  
  if (text.isEmpty() || m_nodes->isEmpty()) {
    return;
  }

  QTextBlock block = currentBlock();
  auto blockStart = block.position();
  auto blockEnd = blockStart + block.length();

  for (auto basenode : *m_nodes) {
    if (!basenode) {
      continue;
    }

    Node* node = basenode;

    while (true) {
      Node::Type type = node->type();
      int nodeStart = node->start();
      int nodeEnd;
      int length;

      if (type != Node::ValueType) {
        nodeEnd = nodeStart + node->length();
        nodeStart -= blockStart;
        nodeEnd = nodeStart + node->length();
        nodeStart = (nodeStart < 0 ? 0 : nodeStart);
        nodeEnd = (nodeEnd > blockEnd ? blockEnd : nodeEnd);

        length = nodeEnd - nodeStart;

        if (length <= 0) {
          if (!node->next) {
            break;
          }

          node = node->next;
          continue;
        }
      }

      switch (type) {
      case Node::NewlineType:
        break;

      case Node::PseudoStateType:
        setFormat(nodeStart, length, m_pseudoStateFormat);
        break;

      case Node::PseudoStateMarkerType:
        setFormat(nodeStart, length, m_pseudoStateMarkerFormat);
        break;

      case Node::SubControlType:
        setFormat(nodeStart, length, m_subControlFormat);
        break;

      case Node::SubControlMarkerType:
        setFormat(nodeStart, length, m_subControlMarkerFormat);
        break;

      case Node::WidgetType:
        setFormat(nodeStart, length, m_widgetFormat);
        break;

      case Node::NameType:
        setFormat(nodeStart, length, m_nameFormat);
        break;

      case Node::ValueType: {
        ValueNode* vNode = qobject_cast<ValueNode*>(node);
        QStringList values = vNode->values();
        QList<bool> checks = vNode->checks();
        QList<int> offsets = vNode->offsets();
        bool check;
        QString value;
        int offset;
        int start = vNode->start();

        for (int i = 0; i < values.length(); i++) {
          value = values.at(i);
          check = checks.at(i);
          offset = offsets.at(i);

          start = nodeStart - blockStart + offset;//setNodeStart(nodeStart + offset, blockEnd);

          //        auto end = setNodeEnd(nodeStart + offset + length, blockEnd);
          //        length = end - start;

          if (check) {
            setFormat(start, value.length(), m_valueFormat);

          } else {
            setFormat(start, value.length(), m_badValueFormat);
          }
        }

        break;
      }

      case Node::PropertyType:
        setFormat(nodeStart, length, m_propertyFormat);
        break;

      case Node::PropertyMarkerType:
        setFormat(nodeStart, length, m_propertyMarkerFormat);
        break;

      default:
        setFormat(nodeStart, length, m_baseFormat);
      }

      if (!node->next) {
        break;
      }

      node = node->next;
    }
  }
}

void StylesheetHighlighter::setNormalFormat(QColor color, QFont::Weight weight)
{
  m_baseFormat.setFontWeight(weight);
  m_baseFormat.setForeground(QBrush(color));
}

void StylesheetHighlighter::setNormalFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_baseFormat.setFontWeight(weight);
  m_baseFormat.setForeground(color);
}

void StylesheetHighlighter::setWidgetFormat(QColor color, QFont::Weight weight)
{
  m_widgetFormat.setFontWeight(weight);
  m_widgetFormat.setForeground(QBrush(color));
}

void StylesheetHighlighter::setWidgetFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_widgetFormat.setFontWeight(weight);
  m_widgetFormat.setForeground(color);
}

void StylesheetHighlighter::setPseudoStateFormat(QColor color, QFont::Weight weight)
{
  m_pseudoStateFormat.setFontWeight(weight);
  m_pseudoStateFormat.setForeground(QBrush(color));
}

void StylesheetHighlighter::setPseudoStateFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_pseudoStateFormat.setFontWeight(weight);
  m_pseudoStateFormat.setForeground(color);
}

void StylesheetHighlighter::setPseudoStateMarkerFormat(QColor color, QFont::Weight weight)
{
  m_pseudoStateMarkerFormat.setFontWeight(weight);
  m_pseudoStateMarkerFormat.setForeground(QBrush(color));
}

void StylesheetHighlighter::setPseudoStateMarkerFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_pseudoStateMarkerFormat.setFontWeight(weight);
  m_pseudoStateMarkerFormat.setForeground(color);
}

void StylesheetHighlighter::setSubControlFormat(QColor color, QFont::Weight weight)
{
  m_subControlFormat.setFontWeight(weight);
  m_subControlFormat.setForeground(QBrush(color));
}

void StylesheetHighlighter::setSubControlFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_subControlFormat.setFontWeight(weight);
  m_subControlFormat.setForeground(color);
}

void StylesheetHighlighter::setSubControlMarkerFormat(QColor color, QFont::Weight weight)
{
  m_subControlMarkerFormat.setFontWeight(weight);
  m_subControlMarkerFormat.setForeground(QBrush(color));
}

void StylesheetHighlighter::setSubControlMarkerFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_subControlMarkerFormat.setFontWeight(weight);
  m_subControlMarkerFormat.setForeground(color);
}

void StylesheetParser::StylesheetHighlighter::setValueFormat(QColor color, QFont::Weight weight)
{
  m_valueFormat.setFontWeight(weight);
  m_valueFormat.setForeground(QBrush(color));
}

void StylesheetHighlighter::setValueFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_valueFormat.setFontWeight(weight);
  m_valueFormat.setForeground(color);
}

void StylesheetHighlighter::setBadValueFormat(QColor color, QFont::Weight weight)
{
  m_badValueFormat.setFontWeight(weight);
  m_badValueFormat.setForeground(color);
  m_badValueFormat.setFontUnderline(true);
  m_badValueFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
  m_badValueFormat.setUnderlineColor(QColor("red"));
}

void StylesheetHighlighter::setBadValueFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_badValueFormat.setFontWeight(weight);
  m_badValueFormat.setForeground(color);
  m_badValueFormat.setFontUnderline(true);
  m_badValueFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
  m_badValueFormat.setUnderlineColor(QColor("red"));
}

void StylesheetHighlighter::setPropertyFormat(QColor color, QFont::Weight weight)
{
  m_propertyFormat.setFontWeight(weight);
  m_propertyFormat.setForeground(color);
}

void StylesheetHighlighter::setPropertyFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_propertyFormat.setFontWeight(weight);
  m_propertyFormat.setForeground(color);
}

void StylesheetHighlighter::setPropertyMarkerFormat(QColor color, QFont::Weight weight)
{
  m_propertyMarkerFormat.setFontWeight(weight);
  m_propertyMarkerFormat.setForeground(color);
}

void StylesheetHighlighter::setPropertyMarkerFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_propertyMarkerFormat.setFontWeight(weight);
  m_propertyMarkerFormat.setForeground(color);
}

void StylesheetHighlighter::setNameFormat(QColor color, QFont::Weight weight)
{
  m_nameFormat.setFontWeight(weight);
  m_nameFormat.setForeground(QBrush(color));
}

void StylesheetHighlighter::setNameFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_nameFormat.setFontWeight(weight);
  m_nameFormat.setForeground(color);
}

} // end of StylesheetParser
