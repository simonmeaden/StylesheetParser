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
#include "stylesheetparser/stylesheetedit.h"

namespace StylesheetParser {

//=== StylesheetEdit
//================================================================
StylesheetEdit::StylesheetEdit(QWidget* parent)
  : QPlainTextEdit(parent)
  , m_datastore(new DataStore(this))
  , m_parser(new Parser(document(), m_datastore, this))
  , m_highlighter(new StylesheetHighlighter(this))
{
  m_lineNumberArea = new LineNumberArea(this);

  connect(this, &StylesheetEdit::blockCountChanged, this, &StylesheetEdit::updateLineNumberAreaWidth);
  connect(this, &StylesheetEdit::updateRequest, this, &StylesheetEdit::updateLineNumberArea);
  connect(this, &StylesheetEdit::cursorPositionChanged, this, &StylesheetEdit::highlightCurrentLine);

  updateLineNumberAreaWidth(0);
  highlightCurrentLine();
  //  connect(this,
  //          &QTextEdit::cursorPositionChanged,
  //          this,
  //          &StylesheetEdit::onCursorPositionChanged);
  //  connect(this, &QTextEdit::textChanged, this,
  //  &StylesheetEdit::onTextChanged);
  connect(this->document(),
          &QTextDocument::contentsChange,
          this,
          &StylesheetEdit::onDocumentChanged);
}

//void StylesheetEdit::setText(const QString& text)
//{
//  setPlainText(text);
//}

void StylesheetEdit::setPlainText(const QString& text)
{
  ParserState* state = m_parser->parse(text);

  if (!state->errors().testFlag(ParserState::NoError)) {
    // TODO error recovery
  }

  QPlainTextEdit::setPlainText(text);
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

//void StylesheetEdit::showLineNumbers(bool show)
//{
//  // TODO show/hide line numbers
//}

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

void StylesheetEdit::setPseudoStateFormat(Qt::GlobalColor color,
    QFont::Weight weight)
{
  m_highlighter->setPseudoStateFormat(color, weight);
}

void StylesheetEdit::setPseudoStateMarkerFormat(QColor color, QFont::Weight weight)
{
  m_highlighter->setPseudoStateMarkerFormat(color, weight);
}

void StylesheetEdit::setPseudoStateMarkerFormat(Qt::GlobalColor color,
    QFont::Weight weight)
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

void StylesheetEdit::setSubControlMarkerFormat(Qt::GlobalColor color,
    QFont::Weight weight)
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

void StylesheetEdit::setPropertyMarkerFormat(Qt::GlobalColor color,
    QFont::Weight weight)
{
  m_highlighter->setPropertyMarkerFormat(color, weight);
}

void StylesheetEdit::setLineNumberFormat(QColor foreground, QColor background, QFont::Weight weight)
{
  m_lineNumberArea->setFore(foreground);
  m_lineNumberArea->setBack(background);
  m_lineNumberArea->setWeight(weight);
}

void StylesheetEdit::setLineNumberFormat(Qt::GlobalColor foreground, Qt::GlobalColor background, QFont::Weight weight)
{
  m_lineNumberArea->setFore(foreground);
  m_lineNumberArea->setBack(background);
  m_lineNumberArea->setWeight(weight);
}

void StylesheetEdit::lineNumberAreaPaintEvent(QPaintEvent* event)
{
  QPainter painter(m_lineNumberArea);
  painter.fillRect(event->rect(), m_lineNumberArea->back());

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
  int bottom = top + qRound(blockBoundingRect(block).height());

  while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top()) {
      QString number = QString::number(blockNumber + 1);
      painter.setPen(m_lineNumberArea->fore());
      painter.drawText(0, top, m_lineNumberArea->width(), fontMetrics().height(),
                       Qt::AlignRight, number);
    }

    block = block.next();
    top = bottom;
    bottom = top + qRound(blockBoundingRect(block).height());
    ++blockNumber;
  }
}

int StylesheetEdit::lineNumberAreaWidth()
{
  int digits = 1;
  int max = qMax(1, blockCount());

  while (max >= 10) {
    max /= 10;
    ++digits;
  }

  int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

  return space;
}

void StylesheetEdit::resizeEvent(QResizeEvent* event)
{
  QPlainTextEdit::resizeEvent(event);

  QRect cr = contentsRect();
  m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

QString StylesheetParser::StylesheetEdit::getValueAtCursor(int pos, const QString& text)
{
  QChar c;
  QString value;
  int p = pos;

  while (true) {
    c = text.at(p);

    if (c.isLetterOrNumber()) {
      value.prepend(c);
      p--;

    } else {
      break;
    }
  }

  p = pos + 1;

  while (true) {
    c = text.at(p);

    if (c.isLetterOrNumber()) {
      value.append(c);
      p++;

    } else {
      break;
    }
  }

  return value;
}

QString StylesheetParser::StylesheetEdit::getOldNodeValue(Data* data)
{
  QString oldValue;

  switch (data->node->type()) {
  case Node::WidgetType:
  case Node::PropertyType:
    oldValue = qobject_cast<BaseNode*>(data->node)->value();
    break;

  case Node::SubControlType:
  case Node::PseudoStateType:
    oldValue = qobject_cast<NameNode*>(data->node)->value();
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

  //  case Node::ValueType:

  default:
    break;
  }

  return oldValue;
}

void StylesheetEdit::updateNodes(Node* modifiedNode, int charsChanged)
{
  Node* basenode = modifiedNode;
  Node* modified = modifiedNode;

  // find modifiedNode's base node.
  while (basenode->previous) {
    basenode = basenode->previous;
  }

  // modify the rest of the nodes under this basenode.
  while (modified->next) {
    modified = modified->next;
    modified->moveStart(charsChanged);
  }

  NodeList* nodelist = nodes();

  // find the base node following this basenode.
  int i = 0;

  for (; i < nodelist->length();) {
    auto base = nodelist->at(i++);

    if (base == basenode) {
      break;
    }
  }

  // then skip over it and modify the rest.
  for (; i < nodelist->length(); i++) {
    modified = nodelist->at(i);
    modified->moveStart(charsChanged);

    while (modified->next) {
      modified = modified->next;
      modified->moveStart(charsChanged);
    }
  }
}

void StylesheetEdit::updateLineNumberAreaWidth(int /*newBlockCount*/)
{
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void StylesheetEdit::highlightCurrentLine()
{
  QList<QTextEdit::ExtraSelection> extraSelections;

  if (!isReadOnly()) {
    QTextEdit::ExtraSelection selection;

    QColor lineColor = QColor(Qt::yellow).lighter(160);

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);
  }

  setExtraSelections(extraSelections);
}

void StylesheetEdit::updateLineNumberArea(const QRect& rect, int dy)
{
  if (dy) {
    m_lineNumberArea->scroll(0, dy);

  } else {
    m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
  }

  if (rect.contains(viewport()->rect())) {
    updateLineNumberAreaWidth(0);
  }
}

//void StylesheetEdit::onCursorPositionChanged()
//{
//  getNodeAtCursor();
//}

void StylesheetEdit::onDocumentChanged(int pos, int charsRemoved, int charsAdded)
{
  Data* data = getNodeAtCursor();

  if (data->node) {
    int anchor = m_cursor.anchor();
    QString text = toPlainText();
    QString value, oldValue;
    int start = data->node->start();
    int end, length;

    if (data->node->type() == Node::ValueType) {
      ValueNode* valNode = qobject_cast<ValueNode*>(data->node);

      // We need the name of the property node to check for correctness.
      Node* node = valNode;

      if (data->prevNode->type() != Node::PropertyMarkerType) {
        // TODO error value must follow a property marker
      }

      node = data->prevNode->previous; // this should be the property node.

      if (node->type() != Node::PropertyType) {
        // TODO error marker must follow a property
      }

      PropertyNode* propNode = qobject_cast<PropertyNode*>(node);
      QString property;

      if (propNode) {
        property = propNode->value();
      }

      QList<int> offsets = valNode->offsets();
      QStringList values = valNode->values();
      QList<bool> checks = valNode->checks();
      length = data->node->length();
      end = start + length;
      int size = offsets.size();
      int offset;
      int prevIndex, nextIndex;
      bool check = false;
      int charChanged = charsAdded - charsRemoved;

      if (data->valueIndex >= 0) {
        if (data->valueIndex == size && data->nextValueIndex == size) {
          // beyond last existing value
          int index = size;
          value = getValueAtCursor(pos, text);

          if (!value.isEmpty()) {
            values.append(value);
          }

          return;

        } else {
          // inside existing value
          oldValue = values.at(data->valueIndex);
          value = getValueAtCursor(pos, text);
          offset = offsets.at(data->valueIndex);

          if (oldValue != value) {
            if (data->valueIndex == size - 1) { // last item
              values[data->valueIndex] = value;

            } else {
              values[data->valueIndex] = value;

              for (int i = data->valueIndex + 1; i < size; i++) {
                offsets[i] = offsets.at(i) + charsAdded;
              }
            }

            check = m_datastore->isValidPropertyValue(property, value);
            checks[data->valueIndex] = check;
            updateNodes(data->node, charChanged);
            m_highlighter->rehighlight();
          }
        }

      } else if (data->valueIndex == -1) {
        // between existing values
        prevIndex = data->nextValueIndex - 1;
        nextIndex = data->nextValueIndex;
      }

    } else { // not a ValueNode
      oldValue = getOldNodeValue(data);

      length = data->node->length();
      end = start + length;

      if (pos == anchor) {
        // no selection
        value = getValueAtCursor(anchor, text);

        if (value != oldValue) {
          qWarning() << QString("New value(%1) != old value (%2)")
                     .arg(value)
                     .arg(oldValue);
        }

      } else {
        // TODO a selection???
      }
    }
  }/* else if (data->nextNode) {

    // if next node is set then we are either between nodes, after
    // the last node or inside/after a value set
    if (charsAdded > 0) {
      if (data->node->type() == Node::ValueType) {
        ValueNode* valNode = qobject_cast<ValueNode*>(data->node);

      } else {
      }
    }
  }*/
}

StylesheetEdit::Data* StylesheetEdit::getNodeAtCursor()
{
  Data* data = new Data();

  m_cursor = textCursor();
  int cursorPos = m_cursor.anchor();
  int start, end;

  for (auto basenode : *nodes()) {
    if (!basenode) {
      return data;
    }

    Node* node = basenode;

    while (true) {
      if (node->type() == Node::ValueType) {
        ValueNode* valNode = qobject_cast<ValueNode*>(node);
        QList<int> offsets = valNode->offsets();
        QStringList values = valNode->values();
        int offset, valStart, valEnd;
        start = node->start();
        end = start + offsets.back() + values.back().length();

        if (cursorPos > end) {
          data->prevNode = node;
          node = node->next;
          continue;
        }

        for (int i = 0; i < offsets.size(); i++) {
          offset = offsets[i];
          valStart = start + offset;
          valEnd = valStart + values[i].length();

          if (cursorPos > valEnd) {
            if (i == offsets.size() - 1) {
              data->node = node;
              data->valueIndex = i;
              data->nextValueIndex = data->valueIndex;
              break;
            }

            continue;

          } else if (cursorPos < valStart) {
            // already overshot the cursor.
            data->node = node;
            data->valueIndex = -1;
            data->nextValueIndex = i;
            break;

          } else if (cursorPos >= valStart && cursorPos <= valEnd) {
            data->node = node;
            data->valueIndex = i;
            data->nextValueIndex = -1;
            break;
          }
        }

        if (data->node) {
          break;
        }

      } else {
        start = node->start();
        int length = node->length();
        end = start + length;

        if (cursorPos > end) {
          data->prevNode = node;
          node = node->next;
          continue;

        } else if (cursorPos < start) {
          // already overshot the cursor.
          data->nextNode = node;
          break;

        } else if (cursorPos >= start && cursorPos < end) {
          data->node = node;
          break;
        }
      }
    }
  }

  return data;
}

LineNumberArea::LineNumberArea(StylesheetEdit* editor)
  : QWidget(editor)
  , m_codeEditor(editor)
  , m_fore(QColor("#B5B5B5"))
  , m_back(QColor("#E9E9E8"))
{}

QSize LineNumberArea::sizeHint() const
{
  return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent* event)
{
  m_codeEditor->lineNumberAreaPaintEvent(event);
}

QFont::Weight LineNumberArea::weight() const
{
  return m_weight;
}

void LineNumberArea::setWeight(const QFont::Weight& weight)
{
  m_weight = weight;
}

QColor LineNumberArea::back() const
{
  return m_back;
}

void LineNumberArea::setBack(const QColor& back)
{
  m_back = back;
}

QColor LineNumberArea::fore() const
{
  return m_fore;
}

void LineNumberArea::setFore(const QColor& fore)
{
  m_fore = fore;
}

} // end of StylesheetParser
