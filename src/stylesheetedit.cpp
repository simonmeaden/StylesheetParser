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

namespace StylesheetEditor {

//=== StylesheetEdit
//================================================================
StylesheetEdit::StylesheetEdit(QWidget* parent)
  : QPlainTextEdit(parent)
  , m_datastore(new DataStore(this))
    //  , m_parser(new Parser(this, m_datastore, this))
  , m_highlighter(new StylesheetHighlighter(this))
  , m_nodes(new NodeList())
  , m_braceCount(0)
  , m_bracesMatched(true)
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
  QPlainTextEdit::setPlainText(text);
  parseInitialText(text);
  m_highlighter->rehighlight();
}

NodeList* StylesheetEdit::nodes()
{
  return m_nodes;
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

void StylesheetEdit::setBadValueFormat(QColor color, QFont::Weight weight, bool underline,
                                       QTextCharFormat::UnderlineStyle underlineStyle, QColor underlineColor)
{
  m_highlighter->setBadValueFormat(color, weight, underline, underlineStyle, underlineColor);
}

void StylesheetEdit::setBadValueFormat(Qt::GlobalColor color, QFont::Weight weight, bool underline,
                                       QTextCharFormat::UnderlineStyle underlineStyle, QColor underlineColor)
{
  m_highlighter->setBadValueFormat(color, weight, underline, underlineStyle, underlineColor);
}

void StylesheetEdit::setStartBraceFormat(QColor color, QFont::Weight weight)
{
  m_highlighter->setStartBraceFormat(color, weight);
}

void StylesheetEdit::setStartBraceFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_highlighter->setStartBraceFormat(color, weight);
}

void StylesheetEdit::setEndBraceFormat(QColor color, QFont::Weight weight)
{
  m_highlighter->setEndBraceFormat(color, weight);
}

void StylesheetEdit::setEndBraceFormat(Qt::GlobalColor color, QFont::Weight weight)
{
  m_highlighter->setEndBraceFormat(color, weight);
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

bool StylesheetEdit::event(QEvent* event)
{
  if (event->type() == QEvent::ToolTip) {
    QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
    QTextCursor cursor = cursorForPosition(helpEvent->pos());
    Data* data = getNodeAtCursor(cursor);

    if (data->node) {
      if (data->node->type() == Node::BadNodeType) {
        BadBlockNode* badnode = qobject_cast<BadBlockNode*>(data->node);

        if (badnode) {
          ParserState::Errors errors = badnode->errors();

          if (errors.testFlag(ParserState::InvalidPropertyMarker)) {
            QToolTip::showText(helpEvent->globalPos(), tr("Too many property markers!"));

          } else if (errors.testFlag(ParserState::InvalidPropertyValue)) {
            QToolTip::showText(helpEvent->globalPos(), tr("Property value is not valid for this property!"));

          } else if (errors.testFlag(ParserState::ValueIsAProperty)) {
            QToolTip::showText(helpEvent->globalPos(), tr("Value is a property!"));

          } else if (errors.testFlag(ParserState::PreviousBadNode)) {
            QToolTip::showText(helpEvent->globalPos(), tr("An earlier value is bad!"));

          } else {
            QToolTip::hideText();
            event->ignore();
          }
        }
      }

    } else {
      QToolTip::hideText();
      event->ignore();
    }

    return true;
  }

  return QPlainTextEdit::event(event);
}

QString StylesheetEditor::StylesheetEdit::getValueAtCursor(int pos, const QString& text)
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

QString StylesheetEditor::StylesheetEdit::getOldNodeValue(Data* data)
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

//void StylesheetEdit::updateNodes(Node* modifiedNode, int charsChanged)
//{
//  Node* basenode = modifiedNode;
//  Node* modified = modifiedNode;

//  // find modifiedNode's base node.
//  while (basenode->previous) {
//    basenode = basenode->previous;
//  }

//  // modify the rest of the nodes under this basenode.
//  while (modified->next) {
//    modified = modified->next;
//    modified->moveStart(charsChanged);
//  }

//  NodeList* nodelist = nodes();

//  // find the base node following this basenode.
//  int i = 0;

//  for (; i < nodelist->length();) {
//    auto base = nodelist->at(i++);

//    if (base == basenode) {
//      break;
//    }
//  }

//  // then skip over it and modify the rest.
//  for (; i < nodelist->length(); i++) {
//    modified = nodelist->at(i);
//    modified->moveStart(charsChanged);

//    while (modified->next) {
//      modified = modified->next;
//      modified->moveStart(charsChanged);
//    }
//  }
//}

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
  //  if (m_nodes->isEmpty()) {
  //    // initial text has not yet been parsed.
  //    return;
  //  }

  //  QString text = toPlainText();

  //  if (text.isEmpty()) {
  //    return;
  //  }

  //  Data* data = getNodeAtCursor(pos);
  //  ParserState* state =  new ParserState(this);

  //  if (data->node) {
  //    QString value, oldValue;

  //    if (data->node->type() == Node::ValueType) {
  //      ValueNode* valNode = qobject_cast<ValueNode*>(data->node);

  //      // We need the name of the property node to check for correctness.
  //      Node* node = valNode;

  //      if (data->prevNode->type() != Node::PropertyMarkerType) {
  //        // TODO error value must follow a property marker
  //      }


  //      node = data->prevNode->previous; // this should be the property node.

  //      if (node->type() != Node::PropertyType) {
  //        // TODO error marker must follow a property
  //      }

  //      PropertyNode* propNode = qobject_cast<PropertyNode*>(node);
  //      QString property;

  //      if (propNode) {
  //        property = propNode->value();
  //      }

  //      QList<int> offsets;
  //      QStringList values;
  //      QList<bool> checks;
  //      int valStart = valNode->start();
  //      // set up the various required nodes.
  //      m_propertynode = propNode;
  //      m_lastnode = data->prevNode;
  //      m_nextnode = valNode->next;

  //      // this will create a new ValueNode and reset the links to
  //      // and from tthe new Valuenode.
  //      parseValueBlock(text, valStart, values, checks, offsets, state, true);
  //      // then unset the nodes.
  //      m_propertynode = nullptr;
  //      m_lastnode = nullptr;
  //      m_nextnode = nullptr;
  //      // then get rid of the old value node.
  //      valNode->deleteLater();
  //      m_highlighter->rehighlight();

  //    } else { // not a ValueNode
  //    }
  //  }/* else if (data->nextNode) {

  //    // if next node is set then we are either between nodes, after
  //    // the last node or inside/after a value set
  //    if (charsAdded > 0) {
  //      if (data->node->type() == Node::ValueType) {
  //        ValueNode* valNode = qobject_cast<ValueNode*>(data->node);

  //      } else {
  //      }
  //    }
  //  }*/
}

int StylesheetEditor::StylesheetEdit::parseProperty(const QString& text, int start, int& pos, QString& block,
    Node** endnode)
{
  PropertyNode* property = new PropertyNode(block, getCursorForNode(start), this);
  QString propertyName = block;
  int propertyMarkerCount = 0;
  BadBlockNode* badnode = nullptr;

  if (m_lastnode) {
    setNodeLinks(property);

  } else {
    m_nodes->append(property);
  }

  while (!(block = findNext(text, pos)).isEmpty()) {
    if (block == ":") {
      propertyMarkerCount++;

      if (propertyMarkerCount == 1) {
        Node* marker = new PropertyMarkerNode(getCursorForNode((pos - block.length()) - start), this);
        setNodeLinks(marker);

      } else {
        badnode = new BadBlockNode(block,
                                   getCursorForNode((pos - block.length()) - start),
                                   ParserState::InvalidPropertyMarker,
                                   this);
        setNodeLinks(badnode);
      }

    } else if (block == ";") {
      *endnode = new PropertyEndMarkerNode(getCursorForNode(property->end()), this);
      setNodeLinks(*endnode);
      break;

    } else if (block == "}") {
      *endnode = new PropertyEndNode(getCursorForNode(property->end()), this);
      setNodeLinks(*endnode);
      break;

    } else {
      if (badnode) {
        badnode = new BadBlockNode(block,
                                   getCursorForNode((pos - block.length()) - start),
                                   ParserState::PreviousBadNode,
                                   this);
        setNodeLinks(badnode);

      } else {

        bool valid = m_datastore->isValidPropertyValue(propertyName, block);

        if (valid) {
          property->addValue(block, true, (pos - block.length()) - start);

        } else {
          if (m_datastore->containsProperty(block)) {
            badnode = new BadBlockNode(block,
                                       getCursorForNode((pos - block.length()) - start),
                                       ParserState::ValueIsAProperty,
                                       this);

          } else {
            badnode = new BadBlockNode(block,
                                       getCursorForNode((pos - block.length()) - start),
                                       ParserState::InvalidPropertyValue,
                                       this);
          }

          setNodeLinks(badnode);
        }
      }
    }
  }

  return property->end();
}

void StylesheetEdit::parseInitialText(const QString& text, int pos)
{
  m_braceCount = 0;
  QString block;
  m_lastnode = nullptr, m_propertynode = nullptr;
  int start;

  while (true) {
    if ((block = findNext(text, pos)).isEmpty()) {
      break;
    }

    start = pos - block.length();

    if (m_datastore->containsWidget(block)) {
      WidgetNode* widgetnode = new WidgetNode(block, getCursorForNode(start), this);
      m_nodes->append(widgetnode);
      m_lastnode = widgetnode;

      while (!(block = findNext(text, pos)).isEmpty()) {
        start = pos - block.length();

        if (block == "::") { // subcontrol
          Node* marker = new SubControlMarkerNode(getCursorForNode(start), this);
          setNodeLinks(marker);

          if (!(block = findNext(text, pos)).isEmpty()) {
            start = pos - block.length();

            if (m_datastore->containsSubControl(block)) {
              Node* subcontrol = new SubControlNode(block, getCursorForNode(start), this);
              setNodeLinks(subcontrol);

            } else {
              Node* badblock = new BadBlockNode(block, getCursorForNode(start), ParserState::InvalidSubControl, this);
              setNodeLinks(badblock);
              break;
            }
          }

        } else if (block == ":") { // pseudostate
          Node* marker = new PseudoStateMarkerNode(getCursorForNode(start), this);
          setNodeLinks(marker);

          if (!(block = findNext(text, pos)).isEmpty()) {
            start = pos - block.length();
            if (m_datastore->containsPseudoState(block)) {
              Node* pseudostate = new PseudoStateNode(block, getCursorForNode(start), this);
              setNodeLinks(pseudostate);

            }  else {
              Node* badblock = new BadBlockNode(block, getCursorForNode(start), ParserState::InvalidPseudoState, this);
              setNodeLinks(badblock);
              break;
            }
          }

        } else if (block == "{") { // start of widget block
          start = pos - block.length();
          m_braceCount++;
          Node* brace = new StartBraceNode(getCursorForNode(start), this);
          setNodeLinks(brace);

        } else if (block == "}") { // end of widget block
          start = pos - block.length();
          m_braceCount++;
          Node* brace = new EndBraceNode(getCursorForNode(start), this);
          setNodeLinks(brace);
          //          brace->next = nullptr; // might already be true?
          m_lastnode = nullptr;
          break;

        } else if (m_datastore->containsProperty(block)) {
          Node* endnode = nullptr;
          int end = parseProperty(text, start, pos, block, &endnode);

          // run out of text.
          if (!endnode) {
            endnode = new PropertyEndNode(getCursorForNode(end), this);
            setNodeLinks(endnode);
            break;
          }

        } else {
          start = pos - block.length();
          Node* badblock = new BadBlockNode(block, getCursorForNode(start), ParserState::InvalidWidget, this);
          setNodeLinks(badblock);
          break;
        }
      }

    } else if (m_datastore->containsProperty(block)) {
      Node* endnode = nullptr;
      int end = parseProperty(text, start, pos, block, &endnode);

      // run out of text.
      if (!endnode) {
        endnode = new PropertyEndNode(getCursorForNode(end), this);
        setNodeLinks(endnode);
        break;
      }
    }
  }
}

//      while (pos < text.length()) {
//        block = findNext(text, pos);

//        if (block == ":") { // pseudo state
//          if (m_braceCount == 0) {
//            WidgetNode* widget = qobject_cast<WidgetNode*>(m_lastnode);

//            if (widget) {
//              Node* marker = new PseudoStateMarkerNode(getCursorForNode(pos - block.length()), this);
//              setNodeLinks(m_lastnode, marker);
//              m_lastnode = marker;

//            } else {
//              Node* badblock = new BadBlockNode(block,
//                                                getCursorForNode(pos - block.length()),
//                                                ParserState::PseudoStateMarkerNotFollowingWidget,
//                                                this);
//              setNodeLinks(m_lastnode, badblock);
//              m_lastnode = badblock;
//            }

//          } else if (m_braceCount == 1) {  //    if (c.isLetter()) {
//            Node* marker = new PropertyMarkerNode(getCursorForNode(pos - block.length()), this);
//            setNodeLinks(m_lastnode, marker);
//            m_lastnode = marker;

//          } else {
//            // TODO missing end brace
//          }

//          continue;

//        } else if (m_braceCount == 1) {
//          if (m_propertynode) {
//            QStringList values;
//            QList<bool> checks;
//            QList<int> offsets;

//            // reset pos to start of block. parsevalueBlock() will start from beginning of values.
//            pos -= block.length();
//            parseValueBlock(text, pos, values, checks, offsets, state);
//            m_propertynode = nullptr;

//            continue;

//          } else {
//            if (m_datastore->containsProperty(block)) {
//              PropertyNode* property = new PropertyNode(block, getCursorForNode(pos - block.length()), this);
//              setNodeLinks(m_lastnode, property);
//              m_propertynode = property;

//              while (!findNext(text, pos).isEmpty()) {

//              }

//            } else {
//              Node* badblock = new BadBlockNode(block,
//                                                getCursorForNode(pos - block.length()),
//                                                ParserState::InvalidPropertyName,
//                                                this);
//              setNodeLinks(m_lastnode, badblock);
//            }

//            continue;
//          }

//        } else if (block == "::") { // sub control marker
//          WidgetNode* widget = qobject_cast<WidgetNode*>(m_lastnode);

//          if (widget) {
//            Node* marker = new SubControlMarkerNode(getCursorForNode(pos - block.length()), this);
//            setNodeLinks(m_lastnode, marker);
//            continue;

//          } else {
//            Node* badblock = new BadBlockNode(block,
//                                              getCursorForNode(pos - block.length()),
//                                              ParserState::SubControlMarkerNotFollowingWidget,
//                                              this);
//            setNodeLinks(m_lastnode, badblock);
//          }

//        } else if (block == "{") { // start brace
//          m_braceCount++;
//          Node* brace = new StartBraceNode(getCursorForNode(pos - block.length()), this);
//          setNodeLinks(m_lastnode, brace);
//          continue;

//        } else if (block == "}") { // end brace
//          m_braceCount--;
//          Node* brace = new EndBraceNode(getCursorForNode(pos - block.length()), this);
//          setNodeLinks(m_lastnode, brace);
//          continue;

//        } else if (block == ";") { // value end
//          Node* endvalues = new SemiColonNode(getCursorForNode(pos - block.length()), this);
//          setNodeLinks(m_lastnode, endvalues);
//          continue;

//        } else {
//          // TODO error???
//        }

//        if (!block.isEmpty()) {
//          PseudoStateMarkerNode* pseudostatemarker = qobject_cast<PseudoStateMarkerNode*>(m_lastnode);

//          if (pseudostatemarker) {
//            if (m_datastore->containsPseudoState(block)) {
//              Node* pseudostate = new PseudoStateNode(block, getCursorForNode(pos - block.length()), this);
//              setNodeLinks(m_lastnode, pseudostate);

//            } else {
//              Node* badblock = new BadBlockNode(block,
//                                                getCursorForNode(pos - block.length()),
//                                                ParserState::PseudoStateNeedsMarker,
//                                                this);
//              setNodeLinks(m_lastnode, badblock);
//            }

//            continue;
//          }

//          SubControlMarkerNode* subcontrolmarker = qobject_cast<SubControlMarkerNode*>(m_lastnode);

//          if (subcontrolmarker) {
//            if (m_datastore->containsSubControl(block)) {
//              Node* subcontrol = new SubControlNode(block, getCursorForNode(pos - block.length()), this);
//              setNodeLinks(m_lastnode, subcontrol);

//            } else {
//              Node* badblock = new BadBlockNode(block,
//                                                getCursorForNode(pos - block.length()),
//                                                ParserState::SubControlNeedsMarker,
//                                                this);
//              setNodeLinks(m_lastnode, badblock);
//            }

//            continue;
//          }
//        }
//      }

//      skipBlanks(text, pos);

//    } else if (m_datastore->containsProperty(block)) {
//      if (m_datastore->containsProperty(block)) {
//        PropertyNode* property = new PropertyNode(block, getCursorForNode(pos - block.length()), this);
//        setNodeLinks(m_lastnode, property);
//        m_propertynode = property;

//      } else {
//        Node* badblock = new BadBlockNode(block,
//                                          getCursorForNode(pos - block.length()),
//                                          ParserState::InvalidPropertyName,
//                                          this);
//        setNodeLinks(m_lastnode, badblock);
//      }

//    } else if (block == ':') {
//      if (m_braceCount == 0) {
//        Node* marker = new PropertyMarkerNode(getCursorForNode(pos - block.length()), this);
//        setNodeLinks(m_lastnode, marker);

//      } else {
//        if (m_propertynode) {
//          QStringList values;
//          QList<bool> checks;
//          QList<int> offsets;

//          // reset pos to start of block. parsevalueBlock() will start from beginning of values.
//          pos -= block.length();
//          parseValueBlock(text, pos, values, checks, offsets, state);
//          m_propertynode = nullptr;

//          continue;
//          //          QStringList values;
//          //          QList<bool> checks;
//          //          QList<int> offsets;
//          //          bool check = false;
//          //          QString propertyName = property->value();
//          //          int start = -1;

//          //          while (true) {
//          //            if (block.isEmpty() || block == ";" || block == "}") {
//          //              break;
//          //            }

//          //            if (start < 0) {
//          //              start = pos - block.length();
//          //            }

//          //            values.append(block);
//          //            check = m_datastore->isValidPropertyValue(propertyName, block);
//          //            checks.append(check);
//          //            // +1 to account for the
//          //            offsets.append(pos - start - block.length());
//          //            block = findNext(text, pos, state);
//          //            continue;
//          //          }

//          //          ValueNode* valuenode = new ValueNode(values, checks, offsets, getCursorForNode(start), this);
//          //          setNodeLinks(lastnode, valuenode);
//          //          propertynode = nullptr;

//          //          if (block == ";") { // value end
//          //            Node* endvalues = new SemiColonNode(getCursorForNode(pos - block.length()), this);
//          //            setNodeLinks(lastnode, endvalues);  //    if (c.isLetter()) {
//          //            continue;

//          //          }
//        }
//      }
//    }

//    if (pos >= text.length()) {
//      break;
//    }
//  }
//}

ParserState* StylesheetEdit::checkBraceCount(const QString& text)
{
  ParserState* state = new ParserState(this);

  for (int p = 0; p < text.length(); p++) {
    QChar c = text.at(p);

    if (c == '{') {
      m_braceCount++;

    } else if (c == '}') {
      m_braceCount--;
    }
  }

  if (m_braceCount == 0) {
    m_bracesMatched = true;

  } else {
    // reset braceCount to use in brace detection.
    m_braceCount = 0;
  }

  if (!m_bracesMatched) {
    if (m_braceCount > 0) {
      state->unsetError(ParserState::NoError);
      state->setError(ParserState::MismatchedBraceCount);
      state->setError(ParserState::MissingEndBrace);

      if (!state->errors().testFlag(ParserState::FatalError)) {
        state->setError(ParserState::NonFatalError);
      }

    } else if (m_braceCount < 0) {
      state->unsetError(ParserState::NoError);
      state->setError(ParserState::MismatchedBraceCount);
      state->setError(ParserState::MissingStartBrace);
    }
  }

  return state;
}

void StylesheetEdit::setNodeLinks(Node* node)
{
  if (m_lastnode) {
    m_lastnode->next = node;
    node->previous = m_lastnode;

  } else {
    node->previous = nullptr;
  }

  m_lastnode = node;
}

void StylesheetEdit::skipBlanks(const QString& text, int& pos)
{
  QChar c;

  for (; pos < text.length(); pos++) {
    c = text.at(pos);

    if (c.isSpace()/* || c == '\n' || c == '\r' || c == '\t'*/) {
      continue;

    } else {
      break;
    }
  }

}

QTextCursor StylesheetEdit::getCursorForNode(int position)
{
  QTextCursor c(document());
  c.movePosition(QTextCursor::Start);
  c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, position);
  return c;
}

QString StylesheetEdit::findNext(const QString& text, int& pos)
{
  QString block;
  QChar c;
  skipBlanks(text, pos);

  if (pos < text.length()) {
    c = text.at(pos);

    while (true) {
      if (c.isNull() || pos >= text.length()) {
        return block;
      }

      if (c.isLetterOrNumber() || c == '-') {
        if (!block.isEmpty()) {
          QChar b = block.back();

          if (b == '{' || b == '}' || b == ';' || b == ':') {
            return block;
          }
        }

        block += c;
        pos++;

      } else if (c.isSpace() && !block.isEmpty()) {
        return block;

      } else if (c == '{' || c == '}' || c == ';' || c == ':') {
        if (!block.isEmpty()) {
          if (block.back().isLetterOrNumber()) {
            // a possibly correct name/number string
            return block;
          }
        }

        if (block.length() == 0 || block.back() == c) {
          block += c;
          pos++;

        } else {
          return block;
        }
      }

      if (pos < text.length()) {
        c = text.at(pos);
      }
    }
  }

  return QString();
}

StylesheetEdit::Data* StylesheetEdit::getNodeAtCursor(QTextCursor cursor)
{
  Data* data = new Data();
  data->cursor = cursor;

  nodeAtCursorPosition(&data, cursor.anchor());

  return data;
}

StylesheetEdit::Data* StylesheetEdit::getNodeAtCursor(int position)
{
  Data* data = new Data();
  data->cursor = QTextCursor(document());

  nodeAtCursorPosition(&data, position);

  return data;
}

void StylesheetEdit::nodeAtCursorPosition(Data** data, int position)
{
  int start, end;

  for (auto basenode : *nodes()) {
    if (!basenode) {
      return;
    }

    Node* node = basenode;

    while (true) {
      if (!node) {
        return;
      }

      start = node->start();
      int length = node->length();
      end = start + length;

      if (position > end) {
        (*data)->prevNode = node;
        node = node->next;
        continue;

      } else if (position < start) {
        // already overshot the cursor.
        (*data)->nextNode = node;
        break;

      } else if (position >= start && position < end) {
        (*data)->node = node;
        break;
      }
    }
  }
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
