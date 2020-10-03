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
#include "stylesheetedit_p.h"

namespace StylesheetEditor {

//=== StylesheetEdit
//================================================================
StylesheetEdit::StylesheetEdit(QWidget* parent)
  : QPlainTextEdit(parent)
  , d_ptr(new StylesheetEditPrivate(this))
{
  initActions();
  setMouseTracking(true);
  connect(this,
          &StylesheetEdit::blockCountChanged,
          this,
          &StylesheetEdit::updateLineNumberAreaWidth);
  connect(this,
          &StylesheetEdit::updateRequest,
          this,
          &StylesheetEdit::updateLineNumberArea);
  connect(this,
          &StylesheetEdit::cursorPositionChanged,
          this,
          &StylesheetEdit::highlightCurrentLine);
  connect(this,
          &QPlainTextEdit::cursorPositionChanged,
          this,
          &StylesheetEdit::onCursorPositionChanged);
  connect(this->document(),
          &QTextDocument::contentsChange,
          this,
          &StylesheetEdit::onDocumentChanged);
  //  connect(this, &QTextEdit::textChanged, this,
  //  &StylesheetEdit::onTextChanged);
  updateLineNumberAreaWidth(0);
  highlightCurrentLine();
}

StylesheetEditPrivate::StylesheetEditPrivate(StylesheetEdit* parent)
  : q_ptr(parent)
  , m_datastore(new DataStore(q_ptr))
  , m_highlighter(new StylesheetHighlighter(q_ptr))
  , m_nodes(new QMap<QTextCursor, Node*>())
  , m_braceCount(0)
  , m_bracesMatched(true)
  , m_startComment(false)
  , m_hoverWidget(nullptr)
{
  m_lineNumberArea = new LineNumberArea(q_ptr);
}

void StylesheetEdit::setPlainText(const QString& text)
{
  QPlainTextEdit::setPlainText(text);
  d_ptr->setPlainText(text);
}

void StylesheetEditPrivate::setPlainText(const QString& text)
{
  parseInitialText(text);
  m_highlighter->rehighlight();
}

QMap<QTextCursor, Node*>* StylesheetEdit::nodes()
{
  return d_ptr->nodes();
}

QMap<QTextCursor, Node*>* StylesheetEditPrivate::nodes()
{
  return m_nodes;
}

void StylesheetEdit::showNewlineMarkers(bool show)
{
  d_ptr->showNewlineMarkers(show);
}

void StylesheetEditPrivate::showNewlineMarkers(bool show)
{
  // TODO show/hide newline markers.
  // maybe detect position of line end and draw a [NL] box??
}

QString StylesheetEdit::styleSheet() const
{
  return d_ptr->styleSheet();
}

QString StylesheetEditPrivate::styleSheet() const
{
  return m_stylesheet;
}

bool StylesheetEditPrivate::checkStylesheetColors(StylesheetData* data,
    QColor& color1,
    QColor& color2,
    QColor& color3)
{
  if (data->colors.count() == 1) {
    color1 = QColor(data->colors.at(0));
    color2 = QColor(Qt::white);
    color2 = QColor(Qt::red);

  } else if (data->colors.count() == 2) {
    color1 = QColor(data->colors.at(0));
    color2 = QColor(data->colors.at(1));
    color2 = QColor(Qt::red);
    return true;

  } else if (data->colors.count() == 3) {
    color1 = QColor(data->colors.at(0));
    color2 = QColor(data->colors.at(1));
    color3 = QColor(data->colors.at(1));
    return true;
  }

  return false;
}

void StylesheetEdit::setStyleSheet(const QString& stylesheet)
{
  d_ptr->setStyleSheet(stylesheet);
}

void StylesheetEditPrivate::setStyleSheet(const QString& stylesheet)
{
  // TODO implement standard stylesheet shit.
  m_stylesheet = stylesheet;
  QString sheet = stylesheet.simplified();
  QString reEditor, reBetweenCurly;

  reEditor = "StylesheetEdit[\\s]*{[\\-\\w:;\\s]*}";
  reBetweenCurly = "{([^}]*)}";

  QRegularExpression re;
  re.setPattern(reEditor);
  QRegularExpressionMatchIterator it = re.globalMatch(sheet);

  while (it.hasNext()) {
    QRegularExpressionMatch matcher = it.next();

    // detects StylesheetEdit widget
    for (auto match : matcher.capturedTexts()) {
      re.setPattern(reBetweenCurly);
      matcher = re.match(match);

      // matches curly braces
      if (matcher.hasMatch()) {
        match = matcher.captured(0);
        // remove curly braces.
        sheet = match.mid(1, match.length() - 2).toLower();

        int pos = 0;
        StylesheetData* data = nullptr;
        QColor color1, color2, color3;
        QTextCharFormat::UnderlineStyle underlineStyle =
          QTextCharFormat::NoUnderline;
        QFont::Weight fontWeight = QFont::Light;
        bool colorError = false;

        while (pos < sheet.length()) {
          data = getStylesheetProperty(sheet, pos);

          if (data->name.isEmpty()) {
            return;
          }

          color1 = (data->colors.size() > 0 ? data->colors.at(0) : QColor());
          color2 = (data->colors.size() > 1 ? data->colors.at(1) : QColor());
          color3 = (data->colors.size() > 2 ? data->colors.at(1) : QColor());
          colorError = checkStylesheetColors(data, color1, color2, color3);
          underlineStyle =
            (data->underline.size() > 0 ? data->underline.at(0)
             : QTextCharFormat::NoUnderline);
          fontWeight =
            (data->weights.size() > 0 ? data->weights.at(0) : QFont::Normal);
          // TODO ignore any more than 3 colours or one weight or underline
          // type,
          // TODO maybe drop earlier values.

          if (data->name == "widget") {
            m_highlighter->setWidgetFormat(color1, color2, fontWeight);
            continue;

          } else if (data->name == "subcontrol") {
            m_highlighter->setSubControlFormat(color1, color2, fontWeight);
            continue;

          } else if (data->name == "subcontrolmarker") {
            m_highlighter->setSubControlMarkerFormat(
              color1, color2, fontWeight);
            continue;

          } else if (data->name == "pseudostate") {
            m_highlighter->setPseudoStateFormat(color1, color2, fontWeight);
            continue;

          } else if (data->name == "pseudostatemarker") {
            m_highlighter->setPseudoStateMarkerFormat(
              color1, color2, fontWeight);
            continue;

          } else if (data->name == "property") {
            m_highlighter->setPropertyFormat(color1, color2, fontWeight);
            continue;

          } else if (data->name == "propertymarker") {
            m_highlighter->setPropertyMarkerFormat(color1, color2, fontWeight);
            continue;

          } else if (data->name == "value") {
            m_highlighter->setValueFormat(color1, color2, fontWeight);
            continue;

          } else if (data->name == "startbrace") {
            m_highlighter->setStartBraceFormat(color1, color2, fontWeight);
            continue;

          } else if (data->name == "endbrace") {
            m_highlighter->setEndBraceFormat(color1, color2, fontWeight);
            continue;

          } else if (data->name == "bracematch") {
            m_highlighter->setBraceMatchFormat(color1, color2, fontWeight);
            continue;

          } else if (data->name == "comment") {
            m_highlighter->setCommentFormat(color1, color2, fontWeight);
            continue;

          } else if (data->name == "bad") {
            m_highlighter->setBadValueFormat(
              color1, color2, fontWeight, true, underlineStyle, color3);
            continue;
          }
        }
      }
    }
  }
}

StylesheetData* StylesheetEditPrivate::getStylesheetProperty(const QString& sheet, int& pos)
{
  QString property, sep, value;
  StylesheetData* data = nullptr;

  property = findNext(sheet, pos);

  if (m_datastore->containsStylesheetProperty(property)) {
    data = new StylesheetData();
    data->name = property;

    if (pos < sheet.length()) {
      sep = findNext(sheet, pos);

      if (sep == ":") {
        while (true) {
          value = findNext(sheet, pos);

          if (value == ";" || value == "}") {
            break;
          }

          m_datastore->getIfValidStylesheetValue(property, value, data);
        }

      } else {
        // TODO show stylesheet error - no property seperator.
      }
    }
  }

  return data;
}

void StylesheetEdit::setValueFormat(
  QColor color, QColor back, QFont::Weight weight)
{
  d_ptr->setValueFormat(color, back, weight);
}

void StylesheetEditPrivate::setValueFormat(
  QColor color,
  QColor back,
  QFont::Weight weight)
{
  m_highlighter->setValueFormat(color, back, weight);
}

void StylesheetEdit::setWidgetFormat(
  QColor color,
  QColor back,
  QFont::Weight weight)
{
  d_ptr->setWidgetFormat(color, back, weight);
}

void StylesheetEditPrivate::setWidgetFormat(
  QColor color,
  QColor back,
  QFont::Weight weight)
{
  m_highlighter->setWidgetFormat(color, back, weight);
}

void StylesheetEdit::setPseudoStateFormat(
  QColor color,
  QColor back,
  QFont::Weight weight)
{
  d_ptr->setPseudoStateFormat(color, back, weight);
}

void StylesheetEditPrivate::setPseudoStateFormat(
  QColor color,
  QColor back,
  QFont::Weight weight)
{
  m_highlighter->setPseudoStateFormat(color, back, weight);
}

void StylesheetEdit::setPseudoStateMarkerFormat(
  QColor color,
  QColor back,
  QFont::Weight weight)
{
  d_ptr->setPseudoStateMarkerFormat(color, back, weight);
}

void StylesheetEditPrivate::setPseudoStateMarkerFormat(
  QColor color,
  QColor back,
  QFont::Weight weight)
{
  m_highlighter->setPseudoStateMarkerFormat(color, back, weight);
}

void StylesheetEdit::setSubControlFormat(
  QColor color,
  QColor back,
  QFont::Weight weight)
{
  d_ptr->setSubControlFormat(color, back, weight);
}
void StylesheetEditPrivate::setSubControlFormat(
  QColor color,
  QColor back,
  QFont::Weight weight)
{
  m_highlighter->setSubControlFormat(color, back, weight);
}

void StylesheetEdit::setSubControlMarkerFormat(
  QColor color,
  QColor back,
  QFont::Weight weight)
{
  d_ptr->setSubControlMarkerFormat(color, back, weight);
}

void StylesheetEditPrivate::setSubControlMarkerFormat(
  QColor color,
  QColor back,
  QFont::Weight weight)
{
  m_highlighter->setSubControlMarkerFormat(color, back, weight);
}

void StylesheetEdit::setPropertyFormat(
  QColor color,
  QColor back,
  QFont::Weight weight)
{
  d_ptr->setPropertyFormat(color, back, weight);
}

void StylesheetEditPrivate::setPropertyFormat(
  QColor color,
  QColor back,
  QFont::Weight weight)
{
  m_highlighter->setPropertyFormat(color, back, weight);
}

void StylesheetEdit::setPropertyMarkerFormat(
  QColor color,
  QColor back,
  QFont::Weight weight)
{
  d_ptr->setPropertyMarkerFormat(color, back, weight);
}

void StylesheetEditPrivate::setPropertyMarkerFormat(
  QColor color,
  QColor back,
  QFont::Weight weight)
{
  m_highlighter->setPropertyMarkerFormat(color, back, weight);
}

void StylesheetEditPrivate::setPropertyEndMarkerFormat(
  QColor color,
  QColor back,
  QFont::Weight weight)
{
  m_highlighter->setPropertyMarkerFormat(color, back, weight);
}

void StylesheetEdit::setLineNumberFormat(QColor color,
    QColor back,
    QFont::Weight weight)
{
  d_ptr->setLineNumberFormat(color, back, weight);
}

void StylesheetEditPrivate::setLineNumberFormat(QColor color,
    QColor back,
    QFont::Weight weight)
{
  m_lineNumberArea->setFore(color);
  m_lineNumberArea->setBack(back);
  m_lineNumberArea->setWeight(weight);
}

void StylesheetEdit::setBadValueFormat(
  QColor color,
  QColor back,
  QFont::Weight weight,
  bool underline,
  QTextCharFormat::UnderlineStyle underlineStyle,
  QColor underlineColor)
{
  d_ptr->setBadValueFormat(
    color, back, weight, underline, underlineStyle, underlineColor);
}

void StylesheetEditPrivate::setBadValueFormat(
  QColor color,
  QColor back,
  QFont::Weight weight,
  bool underline,
  QTextCharFormat::UnderlineStyle underlineStyle,
  QColor underlineColor)
{
  m_highlighter->setBadValueFormat(
    color, back, weight, underline, underlineStyle, underlineColor);
}

void StylesheetEdit::setStartBraceFormat(QColor color,
    QColor back,
    QFont::Weight weight)
{
  d_ptr->setStartBraceFormat(color, back, weight);
}

void StylesheetEditPrivate::setStartBraceFormat(QColor color,
    QColor back,
    QFont::Weight weight)
{
  m_highlighter->setStartBraceFormat(color, back, weight);
}

void StylesheetEdit::setEndBraceFormat(QColor color,
                                       QColor back,
                                       QFont::Weight weight)
{
  d_ptr->setEndBraceFormat(color, back, weight);
}

void StylesheetEditPrivate::setEndBraceFormat(QColor color,
    QColor back,
    QFont::Weight weight)
{
  m_highlighter->setEndBraceFormat(color, back, weight);
}

void StylesheetEdit::setBraceMatchFormat(QColor color,
    QColor back,
    QFont::Weight weight)
{
  d_ptr->setBraceMatchFormat(color, back, weight);
}

void StylesheetEditPrivate::setBraceMatchFormat(QColor color,
    QColor back,
    QFont::Weight weight)
{
  m_highlighter->setBraceMatchFormat(color, back, weight);
}

void StylesheetEdit::lineNumberAreaPaintEvent(QPaintEvent* event)
{
  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top =
    qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
  int bottom = top + qRound(blockBoundingRect(block).height());

  d_ptr->lineNumberAreaPaintEvent(block,
                                  blockNumber,
                                  top,
                                  bottom,
                                  fontMetrics().height(),
                                  blockBoundingRect(block).height(),
                                  event->rect());
}

void StylesheetEditPrivate::lineNumberAreaPaintEvent(QTextBlock block,
    int blockNumber,
    int top,
    int bottom,
    int height,
    double blockHeight,
    QRect rect)
{
  QPainter painter(m_lineNumberArea);
  painter.fillRect(rect, m_lineNumberArea->back());

  while (block.isValid() && top <= rect.bottom()) {
    if (block.isVisible() && bottom >= rect.top()) {
      QString number = QString::number(blockNumber + 1);
      painter.setPen(m_lineNumberArea->fore());
      painter.drawText(
        0, top, m_lineNumberArea->width(), height, Qt::AlignRight, number);
    }

    block = block.next();
    top = bottom;
    bottom = top + qRound(blockHeight);
    ++blockNumber;
  }
}

int StylesheetEdit::lineNumberAreaWidth()
{
  return d_ptr->lineNumberAreaWidth();
}

int StylesheetEditPrivate::lineNumberAreaWidth()
{
  int digits = 1;
  int max = qMax(1, q_ptr->blockCount());

  while (max >= 10) {
    max /= 10;
    ++digits;
  }

  int space =
    3 + q_ptr->fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

  return space;
}

void StylesheetEdit::resizeEvent(QResizeEvent* event)
{
  QPlainTextEdit::resizeEvent(event);

  d_ptr->resizeEvent(contentsRect());
}

void StylesheetEdit::mousePressEvent(QMouseEvent* event)
{
  QPlainTextEdit::mouseMoveEvent(event);
}

void StylesheetEdit::mouseMoveEvent(QMouseEvent* event)
{
  d_ptr->handleMouseMove(event->pos());
}

void StylesheetEdit::mouseReleaseEvent(QMouseEvent* event)
{
  QPlainTextEdit::mouseReleaseEvent(event);
}

void StylesheetEdit::mouseDoubleClickEvent(QMouseEvent* event)
{
  QPlainTextEdit::mouseDoubleClickEvent(event);
}

void StylesheetEditPrivate::resizeEvent(QRect cr)
{
  m_lineNumberArea->setGeometry(
    QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void StylesheetEdit::initActions()
{
  m_formatAct = new QAction("&Format", this);
  m_formatAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
  m_formatAct->setStatusTip(tr("Prettyfy the stylesheet"));
  connect(m_formatAct, &QAction::triggered, this, &StylesheetEdit::format);
}

void StylesheetEdit::contextMenuEvent(QContextMenuEvent* event)
{
  QMenu* menu = createStandardContextMenu();
  menu->addSeparator();
  menu->addAction(m_formatAct);
  menu->exec(event->globalPos());
}

void StylesheetEdit::format()
{
  // TODO format code nicely.
}

void StylesheetEditPrivate::drawHoverWidget(QPoint pos, QString text)
{
  if (!m_hoverWidget) {
    m_hoverWidget = new HoverWidget(q_ptr);
  }

  m_hoverWidget->setText(text);
  m_hoverWidget->setGeometry(
    pos.x() + 40, pos.y(), m_hoverWidget->width(), m_hoverWidget->height());
  m_hoverWidget->show();
}

void StylesheetEditPrivate::hideHoverWidget()
{
  if (m_hoverWidget) {
    m_hoverWidget->hide();
  }
}

void StylesheetEditPrivate::handleMouseMove(QPoint pos)
{
  //  QRect rect = q_ptr->rect();
  QTextCursor tc = q_ptr->cursorForPosition(pos);
  Node* node = getNodeAtCursor(tc).node;

  if (node && node->type() == Node::BadNodeType) {
    BadBlockNode* badNode = qobject_cast<BadBlockNode*>(node);

    if (badNode) {
      displayError(badNode, pos);
    }

  } else {
    hideHoverWidget();
  }
}

void StylesheetEditPrivate::displayError(BadBlockNode* badNode, QPoint pos)
{
  ParserState::Errors errors = badNode->errors();

  if (errors.testFlag(ParserState::InvalidWidget)) {
    drawHoverWidget(pos, q_ptr->tr("This is not a valid Widget"));

  } else if (errors.testFlag(ParserState::InvalidSubControl)) {
    drawHoverWidget(pos, q_ptr->tr("This is not a valid Sub-Control"));

  } else if (errors.testFlag(ParserState::InvalidPseudoState)) {
    drawHoverWidget(pos, q_ptr->tr("This is not a valid Pseudo-State"));

  } else if (errors.testFlag(ParserState::AnomalousMarkerType)) {
    drawHoverWidget(pos, q_ptr->tr("This could be either a Pseudo-State marker or a Sub-Control marker."));

  } else if (errors.testFlag(ParserState::AnomalousType)) {
    drawHoverWidget(pos, q_ptr->tr("The type of this is anomalous."));
  }
}

QString StylesheetEditPrivate::getValueAtCursor(int pos, const QString& text)
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

QString StylesheetEditPrivate::getOldNodeValue(CursorData* data)
{
  QString oldValue;

  switch (data->node->type()) {
  case Node::WidgetType:
  case Node::PropertyType:
    oldValue = qobject_cast<NameNode*>(data->node)->value();
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

void StylesheetEdit::updateLineNumberAreaWidth(int /*newBlockCount*/)
{
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void StylesheetEdit::highlightCurrentLine()
{
  d_ptr->highlightCurrentLine();
}

void StylesheetEditPrivate::highlightCurrentLine()
{
  QList<QTextEdit::ExtraSelection> extraSelections;

  if (!q_ptr->isReadOnly()) {
    QTextEdit::ExtraSelection selection;

    QColor lineColor = QColor(Qt::yellow).lighter(160);

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = q_ptr->textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);
  }

  q_ptr->setExtraSelections(extraSelections);
}

void StylesheetEdit::updateLineNumberArea(const QRect& rect, int dy)
{
  d_ptr->updateLineNumberArea(rect, dy);
}

void StylesheetEditPrivate::updateLineNumberArea(const QRect& rect, int dy)
{
  if (dy) {
    m_lineNumberArea->scroll(0, dy);

  } else {
    m_lineNumberArea->update(
      0, rect.y(), m_lineNumberArea->width(), rect.height());
  }

  if (rect.contains(q_ptr->viewport()->rect())) {
    q_ptr->updateLineNumberAreaWidth(0);
  }
}

void StylesheetEdit::onCursorPositionChanged()
{
  d_ptr->onCursorPositionChanged(textCursor());
}

void StylesheetEditPrivate::onCursorPositionChanged(QTextCursor textCursor)
{
  Node* node;
  CursorData data;
  data.cursor = textCursor;

  nodeAtCursorPosition(&data, data.cursor.anchor());
  node = data.node;

  if (!node) {
    return;
  }

  auto keys = m_nodes->keys();
  auto index = keys.indexOf(data.cursor);
  auto modified = false;

  for (auto startbrace : m_startbraces) {
    if (startbrace->isBraceAtCursor()) {
      startbrace->setBraceAtCursor(false);
      modified = true;
    }
  }

  for (auto endbrace : m_endbraces) {
    if (endbrace->isBraceAtCursor()) {
      endbrace->setBraceAtCursor(false);
      modified = true;
    }
  }

  if (node) {
    auto type = node->type();

    if (type == Node::StartBraceType) {

      for (auto i = index + 1; i < keys.size(); i++) {
        auto n = m_nodes->value(keys.at(i));

        if (n->type() == Node::EndBraceType) {
          EndBraceNode* endbrace = qobject_cast<EndBraceNode*>(n);
          endbrace->setBraceAtCursor(true);
          modified = true;
          break;
        }
      } // end for

    } else if (type == Node::EndBraceType) {
      for (auto i = index - 1; i >= 0; i--) {
        auto n = m_nodes->value(keys.at(i));

        if (n->type() == Node::StartBraceType) {
          StartBraceNode* startbrace = qobject_cast<StartBraceNode*>(n);
          startbrace->setBraceAtCursor(true);
          modified = true;
          break;
        }
      } // end for
    }   // end end brace type
  }     // end if node

  if (modified) {
    m_highlighter->rehighlight();
  }
}

void StylesheetEdit::onDocumentChanged(int pos, int charsRemoved, int charsAdded)
{
  d_ptr->onDocumentChanged(pos, charsRemoved, charsAdded);
}

Node* StylesheetEditPrivate::nextNode(QTextCursor cursor)
{
  QList<QTextCursor> keys = m_nodes->keys();
  int index = keys.indexOf(cursor) + 1; // next index

  if (index < keys.size()) {
    QTextCursor cursor = keys.value(index);
    return m_nodes->value(cursor);
  }

  return nullptr;
}

Node* StylesheetEditPrivate::previousNode(QTextCursor cursor)
{
  QList<QTextCursor> keys = m_nodes->keys();
  int index = keys.indexOf(cursor) - 1; // previous index

  if (index >= 0) {
    QTextCursor cursor = keys.value(index);
    return m_nodes->value(cursor);
  }

  return nullptr;
}

void StylesheetEditor::StylesheetEditPrivate::updatePropertyValues(
  int pos,
  PropertyNode* property,
  int charsAdded,
  int charsRemoved,
  const QString& newValue)
{
  bool updated = false;
  auto values = property->values();
  auto offsets = property->offsets();
  auto checks = property->checks();

  for (int i = 0; i < offsets.size(); i++) {
    auto offset = offsets[i];
    auto value = values[i];
    auto start = property->start() + offset;
    auto end = start + value.length();

    if (updated) {
      offsets[i] += charsAdded;
      offsets[i] -= charsRemoved;
      continue;
    }

    if (pos >= start && pos < end) {
      values.replace(i, newValue);

      if (m_datastore->isValidPropertyValue(property->name(), newValue)) {
        checks.replace(i, PropertyNode::GoodValue);
      }

      updated = true;
    }
  }

  property->setValues(values);
  property->setChecks(checks);
  property->setOffsets(offsets);
}

void StylesheetEditPrivate::onDocumentChanged(int pos,
    int charsRemoved,
    int charsAdded)
{
  if (m_nodes->isEmpty()) {
    // initial text has not yet been parsed.
    return;
  }

  QString text = q_ptr->toPlainText();

  if (text.isEmpty()) {
    return;
  }

  CursorData data = getNodeAtCursor(pos);
  Node* node = data.node;

  if (node) {
    QString value, newValue;

    switch (node->type()) {
    //    case Node::WidgetType: {
    //      WidgetNode* widget = qobject_cast<WidgetNode*>(node);
    //      value = widget->value();
    //      break;
    //    }

    case Node::PropertyType: {
      PropertyNode* property = qobject_cast<PropertyNode*>(node);
      QTextCursor cursor = q_ptr->textCursor();
      cursor.select(QTextCursor::WordUnderCursor);
      newValue = cursor.selectedText();

      if (newValue == ";") {
        Node* next;

        if (property->hasPropertyMarker()) {
          next = nextNode(property->cursor());
          next = nextNode(next->cursor());

        } else {
          next = nextNode(property->cursor());
        }

        if (next->type() == Node::PropertyEndType) {
          PropertyEndMarkerNode* marker = new PropertyEndMarkerNode(next->cursor(), q_ptr);
          m_nodes->insert(next->cursor(), marker);
          next->deleteLater();

          if (property->count() > 0) {
            auto checks = property->checks();
            auto check = checks.last();

            if (check == PropertyNode::MissingPropertyEnd) {
              checks.replace(checks.size() - 1, PropertyNode::GoodValue);
              property->setChecks(checks);
            }
          }
        }

      } else {
        updatePropertyValues(cursor.anchor(), property, charsAdded, charsRemoved, newValue);
      }

      break;
    }

    case Node::BadNodeType: {
      BadBlockNode* badNode = qobject_cast<BadBlockNode*>(node);
      value = badNode->name();
      ParserState::Errors errors = badNode->errors();

      if (errors.testFlag(ParserState::AnomalousType)) {
        QTextCursor cursor(badNode->cursor());
        cursor.select(QTextCursor::WordUnderCursor);
        newValue = cursor.selectedText();

        if (m_datastore->containsWidget(newValue)) {
          Node* widget = new WidgetNode(newValue, badNode->cursor(), q_ptr);
          m_nodes->insert(badNode->cursor(), widget);
          Node* next = nextNode(badNode->cursor());

          if (next) {
            if (next->type() == Node::BadSubControlMarkerType) {
              Node* subcontrolmarker = new SubControlMarkerNode(next->cursor(), q_ptr);
              m_nodes->insert(next->cursor(), subcontrolmarker);
              next->deleteLater();

            } else if (next->type() == Node::BadPseudoStateMarkerType) {
              Node* pseudostatemarker = new PseudoStateMarkerNode(next->cursor(), q_ptr);
              m_nodes->insert(next->cursor(), pseudostatemarker);
              next->deleteLater();
            }
          }

          badNode->deleteLater();

        } else if (m_datastore->containsSubControl(newValue)) {
          Node* subcontrol = new SubControlNode(newValue, badNode->cursor(), q_ptr);
          m_nodes->insert(badNode->cursor(), subcontrol);
          Node* prev = previousNode(badNode->cursor());

          if (prev) {
            if (prev->type() == Node::BadSubControlMarkerType) {
              Node* subcontrolmarker = new SubControlMarkerNode(prev->cursor(), q_ptr);
              m_nodes->insert(prev->cursor(), subcontrolmarker);
              prev->deleteLater();
            }
          }

          badNode->deleteLater();

        } else if (m_datastore->containsPseudoState(newValue)) {
          Node* pseudostate = new PseudoStateNode(newValue, badNode->cursor(), q_ptr);
          m_nodes->insert(badNode->cursor(), pseudostate);
          Node* prev = previousNode(badNode->cursor());

          if (prev) {
            if (prev->type() == Node::BadPseudoStateMarkerType) {
              Node* pseudostatemarker = new PseudoStateMarkerNode(prev->cursor(), q_ptr);
              m_nodes->insert(prev->cursor(), pseudostatemarker);
              prev->deleteLater();
            }
          }

          badNode->deleteLater();

        } else {
          // still bad, just update text.
          badNode->setName(newValue);
        }

      }

      if (errors.testFlag(ParserState::InvalidWidget)) {
        QTextCursor cursor(data.cursor);
        cursor.select(QTextCursor::WordUnderCursor);
        newValue = cursor.selectedText();

        if (m_datastore->containsWidget(newValue)) {
          badNode->deleteLater();
          Node* subcontrol = new WidgetNode(newValue, data.cursor, q_ptr);
          m_nodes->insert(data.cursor, subcontrol);

        } else {
          badNode->setName(newValue);
        }

      } else if (errors.testFlag(ParserState::InvalidSubControl)) {
        QTextCursor cursor(data.cursor);
        cursor.select(QTextCursor::WordUnderCursor);
        newValue = cursor.selectedText();

        if (m_datastore->containsSubControl(newValue)) {
          Node* subcontrol = new SubControlNode(newValue, data.cursor, q_ptr);
          m_nodes->insert(data.cursor, subcontrol);
          badNode->deleteLater();

        } else {
          badNode->setName(newValue);
        }

      } else if (errors.testFlag(ParserState::InvalidPseudoState)) {
        QTextCursor cursor(data.cursor);
        cursor.select(QTextCursor::WordUnderCursor);
        newValue = cursor.selectedText();

        if (m_datastore->containsPseudoState(newValue)) {
          badNode->deleteLater();
          Node* subcontrol =
            new PseudoStateNode(newValue, data.cursor, q_ptr);
          m_nodes->insert(data.cursor, subcontrol);

        } else {
          badNode->setName(newValue);
        }
      }

      break;
    }
    }

    m_highlighter->rehighlight();
  }
}

void StylesheetEditor::StylesheetEditPrivate::stashPropertyEndNode(int position,
    Node** endnode)
{
  QTextCursor cursor = getCursorForNode(position);
  *endnode = new PropertyEndNode(cursor, q_ptr);
  m_nodes->insert(cursor, *endnode);
}

void StylesheetEditor::StylesheetEditPrivate::stashPropertyEndMarkerNode(int position,
    Node** endnode)
{
  QTextCursor cursor = getCursorForNode(position);
  *endnode = new PropertyEndMarkerNode(cursor, q_ptr);
  m_nodes->insert(cursor, *endnode);
}

int StylesheetEditor::StylesheetEditPrivate::parseProperties(
  const QString& text,
  int start,
  int& pos,
  QString& block)
{
  Node* endnode = nullptr;
  QTextCursor cursor = getCursorForNode(start);
  PropertyNode* property = new PropertyNode(block, cursor, q_ptr);
  m_nodes->insert(cursor, property);
  int end = parsePropertyWithValues(cursor, property, text, start, pos, block, &endnode);

  // run out of text.
  if (!endnode) {
    QTextCursor cursor = getCursorForNode(end);
    endnode = new PropertyEndNode(cursor, q_ptr);
    m_nodes->insert(cursor, endnode);
  }

  return end;
}

int StylesheetEditPrivate::parsePropertyWithValues(
  QTextCursor cursor,
  PropertyNode* property,
  const QString& text,
  int start,
  int& pos,
  QString& block,
  Node** endnode)
{
  QString propertyName = block;


  while (!(block = findNext(text, pos)).isEmpty()) {
    if (block == ":") {
      if (!property->hasPropertyMarker()) {
        cursor = getCursorForNode(start + property->length());
        Node* marker = new PropertyMarkerNode(cursor, q_ptr);
        m_nodes->insert(cursor, marker);
        property->setPropertyMarkerExists(true);
      }

    } else if (block == "/*") { // comment start
      parseComment(text, pos);

    } else if (block == ";") {
      stashPropertyEndMarkerNode(property->end(), endnode);
      continue;

    } else if (block == "}") {
      if (!(*endnode)) { // already a property end node (;)
        stashPropertyEndNode(property->end(), endnode);
      }

      stashEndBrace(cursor.anchor());

      pos--; // step back from endbrace
      break;

    } else {
      bool valid = m_datastore->isValidPropertyValue(propertyName, block);

      if (valid) {
        property->addValue(
          block, PropertyNode::GoodValue, (pos - block.length()) - start);

      } else {
        if (m_datastore->containsProperty(block)) {
          Node* lastnode = m_nodes->last();

          if (!(lastnode->type() == Node::PropertyEndType ||
                lastnode->type() == Node::PropertyEndMarkerType)) {
            // the block is actually another property. Probably a missing ';'.
            // set the last check value to missing property end.
            property->setBadCheck(PropertyNode::MissingPropertyEnd);
          }

          if (!(*endnode)) { // already a property end node (;)
            stashPropertyEndNode(property->end(), endnode);
          }

          // now go on to parse further properties.
          //          if (m_datastore->isValidPropertyValue(block))
          //          parsePropertyWithValues(text, pos - block.length(), pos, block, endnode);
          return property->end();

        } else {
          property->addValue(
            block, PropertyNode::BadValue, (pos - block.length()) - start);
        }
      }
    }
  }

  return property->end();
}

void StylesheetEditPrivate::parseComment(const QString& text, int& pos)
{
  m_startComment = true;
  QTextCursor cursor = getCursorForNode(pos);
  Node* marker = new StartCommentNode(cursor, q_ptr);
  m_nodes->insert(cursor, marker);
  pos += 2;

  QChar c;
  CommentNode* comment = new CommentNode(getCursorForNode(pos), q_ptr);
  m_nodes->insert(cursor, comment);

  while (pos < text.length()) {
    c = text.at(pos++);

    if (c != '*') {
      comment->append(c);

    } else {
      if (pos < text.length() - 1) {
        c = text.at(pos++);

        if (c == '/') {
          cursor = getCursorForNode(pos - 2);
          EndCommentNode* endcomment = new EndCommentNode(cursor, q_ptr);
          m_nodes->insert(cursor, endcomment);
          break;

        } else {
          //          comment->append('*');
          comment->append(c);
        }
      }
    }
  }
}

void StylesheetEditPrivate::stashBadNode(int position,
    const QString& block,
    ParserState::Error error)
{
  QTextCursor cursor = getCursorForNode(position);
  Node* badblock = new BadBlockNode(block, cursor, error, q_ptr);
  m_nodes->insert(cursor, badblock);
}

void StylesheetEditPrivate::stashBadSubControlMarkerNode(
  int position,
  ParserState::Error error)
{
  QTextCursor cursor = getCursorForNode(position);
  Node* badblock = new BadSubControlMarkerNode(cursor, error, q_ptr);
  m_nodes->insert(cursor, badblock);
}

void StylesheetEditPrivate::stashBadPseudoStateMarkerNode(int position,
    ParserState::Error error)
{
  QTextCursor cursor = getCursorForNode(position);
  Node* badblock = new BadPseudoStateMarkerNode(cursor, error, q_ptr);
  m_nodes->insert(cursor, badblock);
}

void StylesheetEditPrivate::stashPseudoState(int position,
    const QString& block)
{
  QTextCursor cursor = getCursorForNode(position);
  Node* pseudostate = new PseudoStateNode(block, cursor, q_ptr);
  m_nodes->insert(cursor, pseudostate);
}

void StylesheetEditor::StylesheetEditPrivate::stashSubControl(int position,
    const QString& block)
{
  QTextCursor cursor = getCursorForNode(position);
  Node* subcontrol = new SubControlNode(block, cursor, q_ptr);
  m_nodes->insert(cursor, subcontrol);
}

void StylesheetEditor::StylesheetEditPrivate::stashEndBrace(int position)
{
  m_braceCount--;
  QTextCursor cursor = getCursorForNode(position);
  EndBraceNode* brace = new EndBraceNode(cursor, q_ptr);
  m_nodes->insert(cursor, brace);
  m_endbraces.append(brace);
}

void StylesheetEditor::StylesheetEditPrivate::stashStartBrace(int position)
{
  m_braceCount++;
  QTextCursor cursor = getCursorForNode(position);
  StartBraceNode* brace = new StartBraceNode(cursor, q_ptr);
  m_nodes->insert(cursor, brace);
  m_startbraces.append(brace);
}

void StylesheetEditor::StylesheetEditPrivate::stashPseudoStateMarker(int position)
{
  QTextCursor cursor = getCursorForNode(position);
  Node* marker = new PseudoStateMarkerNode(cursor, q_ptr);
  m_nodes->insert(cursor, marker);
}

void StylesheetEditor::StylesheetEditPrivate::stashSubControlMarker(int position)
{
  QTextCursor cursor = getCursorForNode(position);
  Node* marker = new SubControlMarkerNode(cursor, q_ptr);
  m_nodes->insert(cursor, marker);
}

void StylesheetEditor::StylesheetEditPrivate::stashWidget(int position,
    const QString& block)
{
  QTextCursor cursor = getCursorForNode(position);
  WidgetNode* widgetnode = new WidgetNode(block, cursor, q_ptr);
  m_nodes->insert(cursor, widgetnode);
}

//int StylesheetEditor::StylesheetEditPrivate::parseProperties(
//  const QString& text,
//  int start,
//  int& pos,
//  QString& block)
//{
//  Node* endnode = nullptr;
//  int end = parsePropertyWithValues(text, start, pos, block, &endnode);

//  // run out of text.
//  if (!endnode) {
//    QTextCursor cursor = getCursorForNode(end);
//    endnode = new PropertyEndNode(cursor, q_ptr);
//    m_nodes->insert(cursor, endnode);
//  }

//  return end;
//}

void StylesheetEditPrivate::parseInitialText(const QString& text, int pos)
{
  m_braceCount = 0;
  QString block;
  int start;

  while (true) {
    if ((block = findNext(text, pos)).isEmpty()) {
      break;
    }

    start = pos - block.length();
    QTextCursor cursor;

    if (m_datastore->containsWidget(block)) {
      stashWidget(start, block);

      while (!(block = findNext(text, pos)).isEmpty()) {
        start = pos - block.length();

        if (block == "/*") { // comment start
          parseComment(text, start);

        } else if (block == "::") { // subcontrol
          if (m_braceCount == 0) {
            stashSubControlMarker(start);

            if (!(block = findNext(text, pos)).isEmpty()) {
              if (m_datastore->containsSubControl(block)) {
                stashSubControl(pos - block.length(), block);

              } else {
                stashBadNode(pos - block.length(), block, ParserState::InvalidSubControl);
                break;
              }
            }
          }

        } else if (block == ":") { // pseudostate or property
          if (m_braceCount == 0) {
            stashPseudoStateMarker(start);

            if (!(block = findNext(text, pos)).isEmpty()) {
              if (m_datastore->containsPseudoState(block)) {
                stashPseudoState(pos - block.length(), block);

              } else {
                stashBadNode(pos - block.length(), block, ParserState::InvalidPseudoState);
                break;
              }
            }

          } else {
            parseProperties(text, start, pos, block);
          }

        } else if (block == "{") { // start of widget block
          stashStartBrace(pos - block.length());

        } else if (block == "}") { // end of widget block
          stashEndBrace(pos - block.length());
          break;

        } else if (m_datastore->containsProperty(block)) {
          Node* endnode = nullptr;
          cursor = getCursorForNode(start);
          PropertyNode* property = new PropertyNode(block, cursor, q_ptr);
          m_nodes->insert(cursor, property);
          parsePropertyWithValues(cursor, property, text, start, pos, block, &endnode);

        } else {
          if (m_braceCount == 1) {
            int oldPos = pos;
            QString oldBlock = block;

            if (!(block = findNext(text, pos)).isEmpty()) {
              if (block == ":") {
                Node* endnode = nullptr;
                pos = oldPos;
                PropertyNode* property = new PropertyNode(block, cursor, q_ptr);
                m_nodes->insert(cursor, property);
                int end = parsePropertyWithValues(cursor, property, text, start, pos, block, &endnode);

                // run out of text.
                if (!endnode) {
                  QTextCursor cursor = getCursorForNode(end);
                  endnode = new PropertyEndNode(cursor, q_ptr);
                  m_nodes->insert(cursor, endnode);
                }
              }

            } else {
              stashBadNode(pos - block.length(), block, ParserState::InvalidPropertyName);
            }
          }
        }
      }

    } else if (block == "/*") { // comment start
      parseComment(text, pos);

    } else if (block == "::") { // comment start
      int prevPos = pos;

      if (m_datastore->containsSubControl(block)) {
        stashSubControlMarker(prevPos);
        stashSubControl(pos - block.length(), block);

      } else {
        stashBadSubControlMarkerNode(prevPos - 1, ParserState::InvalidSubControlMarker);
        pos = prevPos;
      }

    } else if (block == ":") { // comment start
      int prevPos = pos;

      if (!(block = findNext(text, pos)).isEmpty()) {
        if (m_datastore->containsPseudoState(block)) {
          stashPseudoStateMarker(prevPos);
          stashPseudoState(pos - block.length(), block);

        } else {
          stashBadPseudoStateMarkerNode(prevPos - 1, ParserState::InvalidPseudoStateMarker);
          // revert to prev position and ignore last find.
          pos = prevPos;
        }
      }


    } else if (block == "{") {
      stashStartBrace(pos - block.length());

    } else if (block == "}") {
      stashEndBrace(pos - block.length());

    } else if (m_datastore->containsProperty(block)) {
      Node* endnode = nullptr;
      PropertyNode* property = new PropertyNode(block, cursor, q_ptr);
      m_nodes->insert(cursor, property);
      int end = parsePropertyWithValues(cursor, property, text, start, pos, block, &endnode);

      // run out of text.
      if (!endnode) {
        cursor = getCursorForNode(end);
        endnode = new PropertyEndNode(cursor, q_ptr);
        m_nodes->insert(cursor, endnode);
        break;
      }

    } else {
      if (!m_nodes->isEmpty()) {
        Node* lastnode = m_nodes->last();
        Node::Type lasttype = lastnode->type();

        if (lasttype == Node::BadSubControlMarkerType) {
          if (m_datastore->containsPseudoState(block)) {
            stashPseudoStateMarker(lastnode->start()); // correct last node
            stashPseudoState(start, block);

          } else if (m_datastore->containsSubControl(block)) {
            stashSubControlMarker(lastnode->start());
            stashSubControl(start, block);

          } else {
            stashBadNode(start, block, ParserState::AnomalousType);
          }

        } else {
          stashBadNode(start, block, ParserState::InvalidWidget);
        }

      } else {
        stashBadNode(start, block, ParserState::AnomalousType);
      }
    }
  }
}

void StylesheetEditPrivate::skipBlanks(const QString& text, int& pos)
{
  QChar c;

  for (; pos < text.length(); pos++) {
    c = text.at(pos);

    if (c.isSpace() /* || c == '\n' || c == '\r' || c == '\t'*/) {
      continue;

    } else {
      break;
    }
  }
}

QTextCursor StylesheetEditPrivate::getCursorForNode(int position)
{
  QTextCursor c(q_ptr->document());
  c.movePosition(QTextCursor::Start);
  c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, position);
  return c;
}

QString StylesheetEditPrivate::findNext(const QString& text, int& pos)
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

      } else if (c == '/') {
        if (pos < text.length() - 1) {
          if (text.at(pos + 1) == '*') {
            // a comment
            if (!block.isEmpty()) {
              pos--; // step back.
              return block;

            } else {
              return "/*";
            }
          }
        }
      }

      if (pos < text.length()) {
        c = text.at(pos);
      }
    }
  }

  return QString();
}

CursorData StylesheetEditPrivate::getNodeAtCursor(QTextCursor cursor)
{
  CursorData data;
  data.cursor = cursor;

  nodeAtCursorPosition(&data, cursor.anchor());

  return data;
}

CursorData StylesheetEditPrivate::getNodeAtCursor(int position)
{
  CursorData data;
  data.cursor = QTextCursor(q_ptr->document());

  nodeAtCursorPosition(&data, position);

  return data;
}

void StylesheetEditPrivate::nodeAtCursorPosition(CursorData* data, int position)
{
  Node* previous;

  for (auto key : m_nodes->keys()) {
    Node* node = m_nodes->value(key);

    if (!node) {
      return;
    }

    if (position > node->end()) {
      previous = node;
      continue;

    } else if (position >= node->start() && position <= node->end()) {
      data->node = node;
      data->cursor = node->cursor();
      data->prevNode = previous;
      break;
    }

    // TODO add property value hovers.
  }
}

StylesheetEditPrivate::LineNumberArea::LineNumberArea(StylesheetEdit* editor)
  : QWidget(editor)
  , m_codeEditor(editor)
  , m_fore(QColor("#B5B5B5"))
  , m_back(QColor("#E9E9E8"))
{}

QSize StylesheetEditPrivate::LineNumberArea::sizeHint() const
{
  return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
}

void StylesheetEditPrivate::LineNumberArea::paintEvent(QPaintEvent* event)
{
  m_codeEditor->lineNumberAreaPaintEvent(event);
}

QFont::Weight StylesheetEditPrivate::LineNumberArea::weight() const
{
  return m_weight;
}

void StylesheetEditPrivate::LineNumberArea::setWeight(const QFont::Weight& weight)
{
  m_weight = weight;
}

QColor StylesheetEditPrivate::LineNumberArea::back() const
{
  return m_back;
}

void StylesheetEditPrivate::LineNumberArea::setBack(const QColor& back)
{
  m_back = back;
}

QColor StylesheetEditPrivate::LineNumberArea::fore() const
{
  return m_fore;
}

void StylesheetEditPrivate::LineNumberArea::setFore(const QColor& fore)
{
  m_fore = fore;
}

StylesheetEditPrivate::HoverWidget::HoverWidget(QWidget* parent)
  : QWidget(parent)
{}

QSize StylesheetEditPrivate::HoverWidget::size()
{
  return QSize(m_width, m_height);
}

void StylesheetEditPrivate::HoverWidget::paintEvent(QPaintEvent* /*event*/)
{
  QPainter painter(this);
  painter.fillRect(0, 0, m_width, m_height, QColor("mistyrose"));
  painter.setPen(QColor(Qt::red));
  painter.drawText(5, m_height - 5, m_text);
}

QString StylesheetEditPrivate::HoverWidget::text() const
{
  return m_text;
}

void StylesheetEditPrivate::HoverWidget::setText(const QString& text)
{
  m_text = text;
  QFontMetrics fm = fontMetrics();
  QRect rect = fm.boundingRect(m_text);
  m_width = rect.width() + 10;
  m_height = rect.height() + 10;
}

int StylesheetEditPrivate::HoverWidget::width() const
{
  return m_width;
}

int StylesheetEditPrivate::HoverWidget::height() const
{
  return m_height;
}

} // end of StylesheetParser
