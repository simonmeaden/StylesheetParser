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
#include "stylesheethighlighter.h"
#include "stylesheetparser/stylesheetedit.h"

namespace StylesheetEditor {

StylesheetHighlighter::StylesheetHighlighter(StylesheetEdit* editor)
  : QSyntaxHighlighter(editor->document())
  , m_editor(editor)
{
  m_back = QColor("white");/*editor->palette().brush(QPalette::Background)*/;
  setWidgetFormat(QColor("#800080"), m_back, QFont::Light);
  setPseudoStateFormat(QColor("#808000"), m_back, QFont::Light);
  setPseudoStateMarkerFormat(QColor(Qt::black), m_back, QFont::Light);
  setSubControlFormat(QColor("#CE5C00"), m_back, QFont::Light);
  setSubControlMarkerFormat(QColor(Qt::black), m_back, QFont::Light);
  setValueFormat(QColor("orangered"), m_back, QFont::Light);
  setBadValueFormat(QColor(Qt::black), m_back, QFont::Light, true,
                    QTextCharFormat::WaveUnderline, QColor(Qt::red));
  setPropertyFormat(QColor("mediumblue"), m_back, QFont::Light);
  setPropertyMarkerFormat(QColor(Qt::black), m_back, QFont::Light);
  setPropertyEndMarkerFormat(QColor(Qt::black), m_back, QFont::Light);
  setStartBraceFormat(QColor(Qt::black), m_back, QFont::Light);
  setEndBraceFormat(QColor(Qt::black), m_back, QFont::Light);
  setBraceMatchFormat(QColor(Qt::red), QColor("lightgreen"), QFont::Normal);
  setCommentFormat(QColor("limegreen"), m_back, QFont::Light);
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
  auto nodes = m_editor->nodes();

  if (text.isEmpty() || nodes->isEmpty()) {
    return;
  }

  QTextBlock block = currentBlock();
  auto blockStart = block.position();
  auto blockLength = block.length();
  auto blockEnd = blockStart + blockLength;

  for (auto key : nodes->keys()) {
    auto node = nodes->value(key);
    auto type = node->type();
    auto length = node->length();
    auto nodeStart = node->start();
    auto nodeEnd = nodeStart + length;

    if (nodeEnd < blockStart) {
      continue;

    } else if (nodeStart >= blockEnd) {
      break;
    }

    if (nodeStart < blockStart && nodeEnd > blockStart) {
      nodeStart = 0;

    } else {
      nodeStart -= blockStart;
    }

    if (nodeEnd > blockEnd) {
      nodeEnd = blockEnd - blockStart;

    } else {
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

    case Node::CommentStartMarkerType:
      setFormat(nodeStart, length, m_commentFormat);
      break;

    case Node::CommentType:
      setFormat(nodeStart, length, m_commentFormat);
      break;

    case Node::CommentEndMarkerType:
      setFormat(nodeStart, length, m_commentFormat);
      break;

    case Node::PropertyType: {

      PropertyNode* pNode = qobject_cast<PropertyNode*>(node);

      if (pNode->hasPropertyMarker()) {
        setFormat(nodeStart, length, m_propertyFormat);

      } else {
        setFormat(nodeStart, length, m_badValueFormat);
      }

      if (pNode) {
        QStringList values = pNode->values();
        QList<PropertyNode::Check> checks = pNode->checks();
        QList<int> offsets = pNode->offsets();
        PropertyNode::Check check;
        QString value;
        int offset;
        int start;

        for (int i = 0; i < values.length(); i++) {
          value = values.at(i);
          check = checks.at(i);
          offset = offsets.at(i);

          start = nodeStart + offset;

          if (check == PropertyNode::GoodValue) {
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

    case Node::PropertyEndMarkerType:
      setFormat(nodeStart, length, m_propertyEndMarkerFormat);
      break;

    case Node::StartBraceType: {
      StartBraceNode* startbrace = qobject_cast<StartBraceNode*>(node);

      if (startbrace->isBraceAtCursor()) {
        setFormat(nodeStart, length, m_braceMatchFormat);

      } else {
        setFormat(nodeStart, length, m_startBraceFormat);
      }

      break;
    }

    case Node::EndBraceType: {
      EndBraceNode* endbrace = qobject_cast<EndBraceNode*>(node);

      if (endbrace->isBraceAtCursor()) {
        setFormat(nodeStart, length, m_braceMatchFormat);

      } else {
        setFormat(nodeStart, length, m_endBraceFormat);
      }

      break;
    }

    default:
      break;
    }
  }
}

void StylesheetHighlighter::setWidgetFormat(QBrush color, QBrush back, QFont::Weight weight)
{
  m_widgetFormat.setFontWeight(weight);
  m_widgetFormat.setForeground(color);
  m_widgetFormat.setBackground(back);
}

void StylesheetHighlighter::setPseudoStateFormat(QBrush color, QBrush back, QFont::Weight weight)
{
  m_pseudoStateFormat.setFontWeight(weight);
  m_pseudoStateFormat.setForeground(color);
  m_pseudoStateFormat.setBackground(back);
}

void StylesheetHighlighter::setPseudoStateMarkerFormat(QBrush color, QBrush back, QFont::Weight weight)
{
  m_pseudoStateMarkerFormat.setFontWeight(weight);
  m_pseudoStateMarkerFormat.setForeground(color);
  m_pseudoStateMarkerFormat.setBackground(back);
}

void StylesheetHighlighter::setSubControlFormat(QBrush color, QBrush back, QFont::Weight weight)
{
  m_subControlFormat.setFontWeight(weight);
  m_subControlFormat.setForeground(color);
  m_subControlFormat.setBackground(back);
}

void StylesheetHighlighter::setSubControlMarkerFormat(QBrush color, QBrush back, QFont::Weight weight)
{
  m_subControlMarkerFormat.setFontWeight(weight);
  m_subControlMarkerFormat.setForeground(color);
  m_subControlMarkerFormat.setBackground(back);
}

void StylesheetEditor::StylesheetHighlighter::setValueFormat(QBrush color, QBrush back, QFont::Weight weight)
{
  m_valueFormat.setFontWeight(weight);
  m_valueFormat.setForeground(color);
  m_valueFormat.setBackground(back);
}

void StylesheetHighlighter::setBadValueFormat(QBrush color, QBrush back,
    QFont::Weight weight,
    bool underline,
    QTextCharFormat::UnderlineStyle underlineStyle,
    QColor underlineColor)
{
  m_badValueFormat.setFontWeight(weight);
  m_badValueFormat.setForeground(color);
  m_badValueFormat.setBackground(back);
  m_badValueFormat.setFontUnderline(underline);
  m_badValueFormat.setUnderlineStyle(underlineStyle);
  m_badValueFormat.setUnderlineColor(underlineColor);
}

void StylesheetHighlighter::setPropertyFormat(QBrush color, QBrush back, QFont::Weight weight)
{
  m_propertyFormat.setFontWeight(weight);
  m_propertyFormat.setForeground(color);
  m_propertyFormat.setBackground(back);
}

void StylesheetHighlighter::setPropertyMarkerFormat(QBrush color, QBrush back, QFont::Weight weight)
{
  m_propertyMarkerFormat.setFontWeight(weight);
  m_propertyMarkerFormat.setForeground(color);
  m_propertyMarkerFormat.setBackground(back);
}

void StylesheetHighlighter::setPropertyEndMarkerFormat(QBrush color, QBrush back, QFont::Weight weight)
{
  m_propertyEndMarkerFormat.setFontWeight(weight);
  m_propertyEndMarkerFormat.setForeground(color);
  m_propertyEndMarkerFormat.setBackground(back);
}

void StylesheetHighlighter::setStartBraceFormat(QBrush color, QBrush back, QFont::Weight weight)
{
  m_startBraceFormat.setFontWeight(weight);
  m_startBraceFormat.setForeground(color);
  m_startBraceFormat.setBackground(back);
}

void StylesheetHighlighter::setEndBraceFormat(QBrush color, QBrush back, QFont::Weight weight)
{
  m_endBraceFormat.setFontWeight(weight);
  m_endBraceFormat.setForeground(color);
  m_endBraceFormat.setBackground(back);
}

void StylesheetHighlighter::setBraceMatchFormat(QBrush color, QBrush back, QFont::Weight weight)
{
  m_braceMatchFormat.setFontWeight(weight);
  m_braceMatchFormat.setForeground(color);
  m_braceMatchFormat.setBackground(back);
}

void StylesheetHighlighter::setCommentFormat(QBrush color, QBrush back, QFont::Weight weight)
{
  m_commentFormat.setFontWeight(weight);
  m_commentFormat.setForeground(color);
  m_commentFormat.setBackground(back);
}

QBrush StylesheetHighlighter::widget() const
{
  return m_widgetFormat.foreground();
}

QBrush StylesheetHighlighter::pseudostate() const
{
  return m_pseudoStateFormat.foreground();
}

QBrush StylesheetHighlighter::pseudostatemarker() const
{
  return m_pseudoStateMarkerFormat.foreground();
}

QBrush StylesheetHighlighter::subcontrol() const
{
  return m_subControlFormat.foreground();
}

QBrush StylesheetHighlighter::subcontrolmarker() const
{
  return m_subControlMarkerFormat.foreground();
}

QBrush StylesheetHighlighter::value() const
{
  return m_valueFormat.foreground();
}

QBrush StylesheetHighlighter::badvalue() const
{
  return m_badValueFormat.foreground();
}

QBrush StylesheetHighlighter::property() const
{
  return m_propertyFormat.foreground();
}

QBrush StylesheetHighlighter::propertymarker() const
{
  return m_propertyMarkerFormat.foreground();
}

QBrush StylesheetHighlighter::startbrace() const
{
  return m_startBraceFormat.foreground();
}

QBrush StylesheetHighlighter::endbrace() const
{
  return m_endBraceFormat.foreground();
}

QBrush StylesheetHighlighter::bracematch() const
{
  return m_braceMatchFormat.foreground();
}

QBrush StylesheetHighlighter::bracematchBack() const
{
  return m_braceMatchFormat.background();
}

QBrush StylesheetHighlighter::comment() const
{
  return m_commentFormat.foreground();
}

QBrush StylesheetHighlighter::back() const
{
  return m_back;
}

QBrush StylesheetHighlighter::underlineColor() const
{
  return m_badValueFormat.underlineColor();
}

int StylesheetHighlighter::widgetWeight() const
{
  return m_widgetFormat.fontWeight();
}

int StylesheetHighlighter::pseudostateWeight() const
{
  return m_pseudoStateFormat.fontWeight();
}

int StylesheetHighlighter::pseudostatemarkerWeight() const
{
  return m_pseudoStateMarkerFormat.fontWeight();
}

int StylesheetHighlighter::subcontrolWeight() const
{
  return m_subControlFormat.fontWeight();
}

int StylesheetHighlighter::subcontrolmarkerWeight() const
{
  return m_subControlMarkerFormat.fontWeight();
}

int StylesheetHighlighter::valueWeight() const
{
  return m_valueFormat.fontWeight();
}

int StylesheetHighlighter::commentWeight() const
{
  return m_commentFormat.fontWeight();
}

int StylesheetHighlighter::bracematchWeight() const
{
  return m_braceMatchFormat.fontWeight();
}

int StylesheetHighlighter::endbraceWeight() const
{
  return m_endBraceFormat.fontWeight();
}

int StylesheetHighlighter::startbraceWeight() const
{
  return m_startBraceFormat.fontWeight();
}

int StylesheetHighlighter::propertymarkerWeight() const
{
  return m_propertyMarkerFormat.fontWeight();
}

int StylesheetHighlighter::propertyWeight() const
{
  return m_propertyFormat.fontWeight();
}

int StylesheetHighlighter::badvalueWeight() const
{
  return m_badValueFormat.fontWeight();

}

bool StylesheetHighlighter::badUnderline() const
{
  return m_badUnderline;
}

QTextCharFormat::UnderlineStyle StylesheetHighlighter::underlinestyle() const
{
  return m_badValueFormat.underlineStyle();
}

} // end of StylesheetParser
