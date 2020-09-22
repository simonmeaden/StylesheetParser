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

namespace StylesheetEditor {

StylesheetHighlighter::StylesheetHighlighter(StylesheetEdit* editor)
  : QSyntaxHighlighter(editor->document())
  , m_editor(editor)
{
  //  m_nodes = editor->nodes();

  setNormalFormat(Qt::black, QFont::Light);
  setWidgetFormat(Qt::blue, QFont::Light);
  setPseudoStateFormat(Qt::darkCyan, QFont::Light);
  setPseudoStateMarkerFormat(Qt::black, QFont::Light);
  setSubControlFormat(Qt::red, QFont::Light);
  setSubControlMarkerFormat(Qt::black, QFont::Light);
  setValueFormat(Qt::magenta, QFont::Light);
  setBadValueFormat(Qt::black, QFont::Light, true, QTextCharFormat::WaveUnderline, QColor("red"));
  setPropertyFormat(Qt::blue, QFont::Light);
  setPropertyMarkerFormat(Qt::black, QFont::Light);
  setStartBraceFormat(Qt::red, QFont::Bold);
  setEndBraceFormat(Qt::blue, QFont::Bold);
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
  NodeList* nodes = m_editor->nodes();

  if (text.isEmpty()) {
    return;
  }

  QTextBlock block = currentBlock();
  auto blockStart = block.position();
  auto blockLength = block.length();
  auto blockEnd = blockStart + blockLength;

  for (int i = 0; i < nodes->size(); i++) {
    Node* node = nodes->at(i);
    qWarning();

    while (node) {
      Node::Type type = node->type();
      int length = node->length();
      int nodeStart = node->start();
      int nodeEnd = node->start() + length;

      if (nodeEnd < blockStart) {
        node = node->next;
        continue;

      } else if (nodeStart >= blockEnd) {
        break;

      } else if (nodeStart < blockStart && nodeEnd > blockStart) {
        nodeStart = 0;
        nodeEnd = (nodeEnd > blockEnd ? blockLength : nodeEnd - blockStart);

      } else if (nodeStart > blockStart) {
        nodeStart -= blockStart;
        nodeEnd = (nodeEnd > blockEnd ? blockEnd - blockStart : nodeEnd - blockStart);

      } else {
        nodeStart -= blockStart;
        nodeEnd -= blockStart;
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

      case Node::BadNodeType:
        setFormat(nodeStart, length, m_badValueFormat);
        break;

      case Node::PropertyType: {
        setFormat(nodeStart, length, m_propertyFormat);

        PropertyNode* pNode = qobject_cast<PropertyNode*>(node);

        if (pNode) {
          QStringList values = pNode->values();
          QList<bool> checks = pNode->checks();
          QList<int> offsets = pNode->offsets();
          bool check;
          QString value;
          int offset;
          int start;

          for (int i = 0; i < values.length(); i++) {
            value = values.at(i);
            check = checks.at(i);
            offset = offsets.at(i);

            start = nodeStart + offset;

            if (check) {
              setFormat(start, value.length(), m_valueFormat);

            } else {
              setFormat(start, value.length(), m_badValueFormat);
            }
          }
        }

        break;
      }

      case Node::PropertyMarkerType:
        setFormat(nodeStart, length, m_propertyMarkerFormat);
        break;

      case Node::StartBraceType:
        setFormat(nodeStart, length, m_startBraceFormat);
        break;

      case Node::EndBraceType:
        setFormat(nodeStart, length, m_endBraceFormat);
        break;

      case Node::NodeType:
      case Node::BaseNodeType:
      case Node::CharNodeType:
      case Node::NameType:
      case Node::SemiColonType:
      case Node::ColonNodeType:
      case Node::PropertyEndType:
        setFormat(nodeStart, length, m_baseFormat);
        break;
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

void StylesheetEditor::StylesheetHighlighter::setValueFormat(QColor color, QFont::Weight weight)
{
  m_valueFormat.setFontWeight(weight);
  m_valueFormat.setForeground(QBrush(color));
}

void StylesheetHighlighter::setValueFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_valueFormat.setFontWeight(weight);
  m_valueFormat.setForeground(color);
}

void StylesheetHighlighter::setBadValueFormat(QColor color,
    QFont::Weight weight,
    bool underline,
    QTextCharFormat::UnderlineStyle underlineStyle,
    QColor underlineColor)
{
  m_badValueFormat.setFontWeight(weight);
  m_badValueFormat.setForeground(color);
  m_badValueFormat.setFontUnderline(underline);
  m_badValueFormat.setUnderlineStyle(underlineStyle);
  m_badValueFormat.setUnderlineColor(underlineColor);
}

void StylesheetHighlighter::setBadValueFormat(
  Qt::GlobalColor color,
  QFont::Weight weight,
  bool underline,
  QTextCharFormat::UnderlineStyle underlineStyle,
  QColor underlineColor)
{
  m_badValueFormat.setFontWeight(weight);
  m_badValueFormat.setForeground(color);
  m_badValueFormat.setFontUnderline(underline);
  m_badValueFormat.setUnderlineStyle(underlineStyle);
  m_badValueFormat.setUnderlineColor(underlineColor);
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

void StylesheetHighlighter::setStartBraceFormat(QColor color, QFont::Weight weight)
{
  m_startBraceFormat.setFontWeight(weight);
  m_startBraceFormat.setForeground(color);
}

void StylesheetHighlighter::setStartBraceFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_startBraceFormat.setFontWeight(weight);
  m_startBraceFormat.setForeground(color);
}

void StylesheetHighlighter::setEndBraceFormat(QColor color, QFont::Weight weight)
{
  m_endBraceFormat.setFontWeight(weight);
  m_endBraceFormat.setForeground(color);
}

void StylesheetHighlighter::setEndBraceFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_endBraceFormat.setFontWeight(weight);
  m_endBraceFormat.setForeground(color);
}

} // end of StylesheetParser
