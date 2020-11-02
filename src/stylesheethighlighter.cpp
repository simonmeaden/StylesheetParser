/*
   Copyright 2020 Simon Meaden

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
#include "stylesheethighlighter.h"
#include "stylesheetparser/stylesheetedit.h"

StylesheetHighlighter::StylesheetHighlighter(StylesheetEdit* editor)
  : QSyntaxHighlighter(editor->document())
  , m_editor(editor)
{
  m_back = QColor("white"); /*editor->palette().brush(QPalette::Background)*/
  //  setWidgetFormat(QColor("#800080"), m_back, QFont::Light);
  setWidgetFormat(QColor("olive"), m_back, QFont::Light);
  setPseudoStateFormat(QColor("#808000"), m_back, QFont::Light);
  setPseudoStateMarkerFormat(QColor(Qt::black), m_back, QFont::Light);
  setSubControlFormat(QColor("#CE5C00"), m_back, QFont::Light);
  setSubControlMarkerFormat(QColor(Qt::black), m_back, QFont::Light);
  setPropertyFormat(QColor("mediumblue"), m_back, QFont::Light);
  //  setPropertyMarkerFormat(QColor(Qt::black), m_back, QFont::Light);
  //  setPropertyEndMarkerFormat(QColor(Qt::black), m_back, QFont::Light);
  setPropertyMarkerFormat(QColor("cyan"), m_back, QFont::Light);
  setPropertyEndMarkerFormat(QColor("mediumseagreen"), m_back, QFont::Light);
  setValueFormat(QColor("orangered"), m_back, QFont::Light);
  //  setStartBraceFormat(QColor(Qt::black), m_back, QFont::Light);
  //  setEndBraceFormat(QColor(Qt::black), m_back, QFont::Light);
  setStartBraceFormat(QColor("darkorange"), m_back, QFont::Light);
  setEndBraceFormat(QColor("cornflowerblue"), m_back, QFont::Light);
  setBraceMatchFormat(QColor(Qt::red), QColor("lightgreen"), QFont::Normal);
  setCommentFormat(QColor("darkmagenta"), m_back, QFont::Light);
}

int
StylesheetHighlighter::setNodeEnd(int nodeEnd, int blockEnd)
{
  if (blockEnd < nodeEnd) {
    return blockEnd;
  }

  return nodeEnd;
}

int
StylesheetHighlighter::setNodeStart(int nodeStart, int blockStart)
{
  if (blockStart > nodeStart) {
    nodeStart = blockStart;
  }

  return nodeStart;
}

void
StylesheetHighlighter::formatProperty(PropertyNode* property)
{
  if (property) {
    auto length = property->name().length();

    if (property->hasPropertyMarker()) {
      setFormat(property->start(), length, m_propertyFormat);
      setFormat(property->start() + property->propertyMarkerOffset(),
                1,
                m_propertyMarkerFormat);

    } else if (!property->isValidProperty()) {
      setFormat(property->start(), length, m_badPropertyFormat);

    } else {
      setFormat(property->start(), length, m_badPropertyFormat);
    }

    QStringList values = property->values();
    QList<PropertyCheck> checks = property->checks();

    QList<int> offsets = property->offsets();
    PropertyCheck check;
    QString value;
    int offset;
    int start;

    for (int i = 0; i < values.length(); i++) {
      value = values.at(i);
      check = checks.at(i);
      offset = offsets.at(i);
      length = value.length();
      start = property->start() + offset;

      if (check == PropertyCheck::GoodValue ||
          check == PropertyCheck::ValidPropertyType) {
        setFormat(start, length, m_valueFormat);

      } else {
        setFormat(start, length, m_badValueFormat);
      }
    }
  }

  if (property->hasEndMarker()) {
    setFormat(property->start() + property->propertyEndMarkerOffset(),
              1,
              m_propertyEndMarkerFormat);
  }
}

NodeIsIn
StylesheetHighlighter::nodeInBlock(const QTextBlock& block,
                                   Node* node,
                                   int& nodeStart)
{
  auto blockStart = block.position();
  auto blockLength = block.length();
  auto blockEnd = blockStart + blockLength;
  auto length = node->length();
  nodeStart = node->start();
  auto nodeEnd = nodeStart + length;

  if (nodeEnd < blockStart) {
    return NodeIsIn::BeforeNode;

  } else if (nodeStart >= blockEnd) {
    return NodeIsIn::AfterNode;
  }

  if (nodeStart < blockStart && nodeEnd > blockStart) {
    nodeStart = 0;

  } else {
    nodeStart -= blockStart;
  }

  //  if (nodeEnd > blockEnd) {
  //    nodeEnd = blockEnd - blockStart;

  //  } else {
  //    nodeEnd -= blockStart;
  //  }
  return NodeIsIn::InNode;
}

void
StylesheetHighlighter::highlightBlock(const QString& text)
{
  auto nodes = m_editor->nodes();
  auto block = currentBlock();

  if (text.isEmpty() || nodes.isEmpty()) {
    return;
  }

  for (auto key : nodes.keys()) {
    auto node = nodes.value(key);
    auto type = node->type();
    int nodeStart;

    auto isin = nodeInBlock(block, node, nodeStart);
    switch (isin) {
      case NodeIsIn::BeforeNode:
        continue;
      case NodeIsIn::AfterNode:
        return;
      default:
        break;
    }

    switch (type) {
      case NodeType::NewlineType:
        break;

      case NodeType::WidgetType: {

        WidgetNode* widget = qobject_cast<WidgetNode*>(node);

        if (widget->isWidgetValid()) {
          setFormat(nodeStart, widget->name().length(), m_widgetFormat);

        } else {
          setFormat(nodeStart, widget->name().length(), m_badWidgetFormat);
        }

        if (widget->hasExtension()) {
          auto extname = widget->extensionName();
          if (widget->isSubControl()) {
            if (widget->isExtensionValid()) {
              setFormat(nodeStart + widget->markerOffset(),
                        2,
                        m_subControlMarkerFormat);
              setFormat(nodeStart + widget->extensionOffset(),
                        extname.length(),
                        m_subControlFormat);
            } else {
              setFormat(nodeStart + widget->markerOffset(),
                        2,
                        m_subControlMarkerFormat);
              setFormat(nodeStart + widget->extensionOffset(),
                        extname.length(),
                        m_badSubControlFormat);
            }
          } else if (widget->isPseudoState()) {
            if (widget->isExtensionValid()) {
              setFormat(nodeStart + widget->markerOffset(),
                        1,
                        m_pseudoStateMarkerFormat);
              setFormat(nodeStart + widget->extensionOffset(),
                        extname.length(),
                        m_pseudoStateFormat);
            } else {
              setFormat(nodeStart + widget->markerOffset(),
                        1,
                        m_pseudoStateMarkerFormat);
              setFormat(nodeStart + widget->extensionOffset(),
                        extname.length(),
                        m_badPseudoStateFormat);
            }
          } else {
            // TODO error has extension but wrong type?
          }
        }

        bool hasstart = widget->hasStartBrace();
        bool hasend = widget->hasEndBrace();
        if (hasstart) {
          auto offset = widget->startBraceOffset();
          if (hasend) {
            setFormat(nodeStart + offset, 1, m_startBraceFormat);
          } else {
            setFormat(nodeStart + offset, 1, m_badStartBraceFormat);
          }
        }

        for (int i = 0; i < widget->propertyCount(); i++) {
          auto property = widget->property(i);
          formatProperty(property);
        }

        if (hasend) {
          auto offset = widget->endBraceOffset();
          if (hasstart) {
            setFormat(nodeStart + offset, 1, m_endBraceFormat);
          } else {
            setFormat(nodeStart + offset, 1, m_badEndBraceFormat);
          }
        }

        break;
      }

      case NodeType::BadNodeType:
        setFormat(nodeStart, node->length(), m_badValueFormat);
        break;

//      case NodeType::CommentStartMarkerType:
//        setFormat(nodeStart, node->length(), m_commentFormat);
//        break;

      case NodeType::CommentType:
        setFormat(nodeStart, node->length(), m_commentFormat);
        break;

//      case NodeType::CommentEndMarkerType:
//        setFormat(nodeStart, node->length(), m_commentFormat);
//        break;

      case NodeType::PropertyType: {
        PropertyNode* property = qobject_cast<PropertyNode*>(node);
        formatProperty(property);
        break;
      }

        //      case NodeType::PropertyEndMarkerType:
        //        setFormat(nodeStart, length, m_propertyEndMarkerFormat);
        //        break;

      case NodeType::StartBraceType: {
        StartBraceNode* startbrace = qobject_cast<StartBraceNode*>(node);
        if (startbrace->isBraceAtCursor()) {
          if (startbrace->hasEndBrace()) {
            setFormat(nodeStart, node->length(), m_braceMatchFormat);
          } else {
            setFormat(nodeStart, node->length(), m_badBraceMatchFormat);
          }
        } else {
          if (startbrace->hasEndBrace()) {
            setFormat(nodeStart, node->length(), m_startBraceFormat);
          } else {
            setFormat(nodeStart, node->length(), m_badStartBraceFormat);
          }
        }
        break;
      }

      case NodeType::EndBraceType: {
        EndBraceNode* endbrace = qobject_cast<EndBraceNode*>(node);
        if (endbrace->isBraceAtCursor()) {
          if (endbrace->hasStartBrace()) {
            setFormat(nodeStart, node->length(), m_braceMatchFormat);
          } else {
            setFormat(nodeStart, node->length(), m_badBraceMatchFormat);
          }
        } else {
          if (endbrace->hasStartBrace()) {
            setFormat(nodeStart, node->length(), m_endBraceFormat);
          } else {
            setFormat(nodeStart, node->length(), m_badEndBraceFormat);
          }
        }
        break;
      }

      default:
        break;
    }
  }
}

void
StylesheetHighlighter::setWidgetFormat(QBrush color,
                                       QBrush back,
                                       QFont::Weight weight)
{
  m_widgetFormat.setFontWeight(weight);
  m_widgetFormat.setForeground(color);
  m_widgetFormat.setBackground(back);
  m_badWidgetFormat = QTextCharFormat(m_widgetFormat);
  m_badWidgetFormat.setUnderlineColor(QColor(Qt::red));
  m_badWidgetFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
}

void
StylesheetHighlighter::setPseudoStateFormat(QBrush color,
                                            QBrush back,
                                            QFont::Weight weight)
{
  m_pseudoStateFormat.setFontWeight(weight);
  m_pseudoStateFormat.setForeground(color);
  m_pseudoStateFormat.setBackground(back);
  m_badPseudoStateFormat = QTextCharFormat(m_pseudoStateFormat);
  m_badPseudoStateFormat.setUnderlineColor(QColor(Qt::red));
  m_badPseudoStateFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
}

void
StylesheetHighlighter::setPseudoStateMarkerFormat(QBrush color,
                                                  QBrush back,
                                                  QFont::Weight weight)
{
  m_pseudoStateMarkerFormat.setFontWeight(weight);
  m_pseudoStateMarkerFormat.setForeground(color);
  m_pseudoStateMarkerFormat.setBackground(back);
}

void
StylesheetHighlighter::setSubControlFormat(QBrush color,
                                           QBrush back,
                                           QFont::Weight weight)
{
  m_subControlFormat.setFontWeight(weight);
  m_subControlFormat.setForeground(color);
  m_subControlFormat.setBackground(back);
  m_badSubControlFormat = QTextCharFormat(m_subControlFormat);
  m_badSubControlFormat.setUnderlineColor(QColor(Qt::red));
  m_badSubControlFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
}

void
StylesheetHighlighter::setSubControlMarkerFormat(QBrush color,
                                                 QBrush back,
                                                 QFont::Weight weight)
{
  m_subControlMarkerFormat.setFontWeight(weight);
  m_subControlMarkerFormat.setForeground(color);
  m_subControlMarkerFormat.setBackground(back);
}

void
StylesheetHighlighter::setValueFormat(QBrush color,
                                      QBrush back,
                                      QFont::Weight weight)
{
  m_valueFormat.setFontWeight(weight);
  m_valueFormat.setForeground(color);
  m_valueFormat.setBackground(back);
  m_badValueFormat = QTextCharFormat(m_badValueFormat);
  m_badValueFormat.setUnderlineColor(QColor(Qt::red));
  m_badValueFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
}

void
StylesheetHighlighter::setPropertyFormat(QBrush color,
                                         QBrush back,
                                         QFont::Weight weight)
{
  m_propertyFormat.setFontWeight(weight);
  m_propertyFormat.setForeground(color);
  m_propertyFormat.setBackground(back);
  m_badPropertyFormat = QTextCharFormat(m_propertyFormat);
  m_badPropertyFormat.setUnderlineColor(QColor(Qt::red));
  m_badPropertyFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
}

void
StylesheetHighlighter::setPropertyMarkerFormat(QBrush color,
                                               QBrush back,
                                               QFont::Weight weight)
{
  m_propertyMarkerFormat.setFontWeight(weight);
  m_propertyMarkerFormat.setForeground(color);
  m_propertyMarkerFormat.setBackground(back);
}

void
StylesheetHighlighter::setPropertyEndMarkerFormat(QBrush color,
                                                  QBrush back,
                                                  QFont::Weight weight)
{
  m_propertyEndMarkerFormat.setFontWeight(weight);
  m_propertyEndMarkerFormat.setForeground(color);
  m_propertyEndMarkerFormat.setBackground(back);
}

void
StylesheetHighlighter::setStartBraceFormat(QBrush color,
                                           QBrush back,
                                           QFont::Weight weight)
{
  m_startBraceFormat.setFontWeight(weight);
  m_startBraceFormat.setForeground(color);
  m_startBraceFormat.setBackground(back);
  m_badStartBraceFormat = QTextCharFormat(m_startBraceFormat);
  m_badStartBraceFormat.setUnderlineColor(QColor(Qt::red));
  m_badStartBraceFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
}

void
StylesheetHighlighter::setEndBraceFormat(QBrush color,
                                         QBrush back,
                                         QFont::Weight weight)
{
  m_endBraceFormat.setFontWeight(weight);
  m_endBraceFormat.setForeground(color);
  m_endBraceFormat.setBackground(back);
  m_badEndBraceFormat = QTextCharFormat(m_endBraceFormat);
  m_badEndBraceFormat.setUnderlineColor(QColor(Qt::red));
  m_badEndBraceFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
}

void
StylesheetHighlighter::setBraceMatchFormat(QBrush color,
                                           QBrush back,
                                           QFont::Weight weight)
{
  m_braceMatchFormat.setFontWeight(weight);
  m_braceMatchFormat.setForeground(color);
  m_braceMatchFormat.setBackground(back);
  m_badBraceMatchFormat = QTextCharFormat(m_braceMatchFormat);
  m_badBraceMatchFormat.setUnderlineColor(QColor(Qt::red));
  m_badBraceMatchFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
}

void
StylesheetHighlighter::setCommentFormat(QBrush color,
                                        QBrush back,
                                        QFont::Weight weight)
{
  m_commentFormat.setFontWeight(weight);
  m_commentFormat.setForeground(color);
  m_commentFormat.setBackground(back);
}

QBrush
StylesheetHighlighter::widget() const
{
  return m_widgetFormat.foreground();
}

QBrush
StylesheetHighlighter::pseudostate() const
{
  return m_pseudoStateFormat.foreground();
}

QBrush
StylesheetHighlighter::pseudostatemarker() const
{
  return m_pseudoStateMarkerFormat.foreground();
}

QBrush
StylesheetHighlighter::subcontrol() const
{
  return m_subControlFormat.foreground();
}

QBrush
StylesheetHighlighter::subcontrolmarker() const
{
  return m_subControlMarkerFormat.foreground();
}

QBrush
StylesheetHighlighter::value() const
{
  return m_valueFormat.foreground();
}

QBrush
StylesheetHighlighter::badvalue() const
{
  return m_badValueFormat.foreground();
}

QBrush
StylesheetHighlighter::property() const
{
  return m_propertyFormat.foreground();
}

QBrush
StylesheetHighlighter::propertymarker() const
{
  return m_propertyMarkerFormat.foreground();
}

QBrush
StylesheetHighlighter::startbrace() const
{
  return m_startBraceFormat.foreground();
}

QBrush
StylesheetHighlighter::endbrace() const
{
  return m_endBraceFormat.foreground();
}

QBrush
StylesheetHighlighter::bracematch() const
{
  return m_braceMatchFormat.foreground();
}

QBrush
StylesheetHighlighter::bracematchBack() const
{
  return m_braceMatchFormat.background();
}

QBrush
StylesheetHighlighter::comment() const
{
  return m_commentFormat.foreground();
}

QBrush
StylesheetHighlighter::back() const
{
  return m_back;
}

QBrush
StylesheetHighlighter::underlineColor() const
{
  return m_badValueFormat.underlineColor();
}

int
StylesheetHighlighter::widgetWeight() const
{
  return m_widgetFormat.fontWeight();
}

int
StylesheetHighlighter::pseudostateWeight() const
{
  return m_pseudoStateFormat.fontWeight();
}

int
StylesheetHighlighter::pseudostatemarkerWeight() const
{
  return m_pseudoStateMarkerFormat.fontWeight();
}

int
StylesheetHighlighter::subcontrolWeight() const
{
  return m_subControlFormat.fontWeight();
}

int
StylesheetHighlighter::subcontrolmarkerWeight() const
{
  return m_subControlMarkerFormat.fontWeight();
}

int
StylesheetHighlighter::valueWeight() const
{
  return m_valueFormat.fontWeight();
}

int
StylesheetHighlighter::commentWeight() const
{
  return m_commentFormat.fontWeight();
}

int
StylesheetHighlighter::bracematchWeight() const
{
  return m_braceMatchFormat.fontWeight();
}

int
StylesheetHighlighter::endbraceWeight() const
{
  return m_endBraceFormat.fontWeight();
}

int
StylesheetHighlighter::startbraceWeight() const
{
  return m_startBraceFormat.fontWeight();
}

int
StylesheetHighlighter::propertymarkerWeight() const
{
  return m_propertyMarkerFormat.fontWeight();
}

int
StylesheetHighlighter::propertyWeight() const
{
  return m_propertyFormat.fontWeight();
}

int
StylesheetHighlighter::badvalueWeight() const
{
  return m_badValueFormat.fontWeight();
}

bool
StylesheetHighlighter::badUnderline() const
{
  return m_badUnderline;
}

QTextCharFormat::UnderlineStyle
StylesheetHighlighter::underlinestyle() const
{
  return m_badValueFormat.underlineStyle();
}
