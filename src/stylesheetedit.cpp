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
#include "stylesheetedit_p.h"
#include <QtDebug>
#include <stylesheetparser/stylesheetedit.h>

namespace StylesheetEditor {

const QChar StylesheetEdit::m_arrow = QChar(0x2BC8);

//=== StylesheetEdit
//================================================================
StylesheetEdit::StylesheetEdit(QWidget* parent)
  : QPlainTextEdit(parent)
  , d_ptr(new StylesheetEditPrivate(this))
{
  initActions();
  initMenus();
  setMouseTracking(true);
  connect(this,
          &StylesheetEdit::blockCountChanged,
          this,
          &StylesheetEdit::updateLeftAreaWidth);
  connect(this,
          &StylesheetEdit::updateRequest,
          this,
          &StylesheetEdit::updateLeftArea);
  connect(this,
          &QPlainTextEdit::cursorPositionChanged,
          this,
          &StylesheetEdit::handleCursorPositionChanged);
  connect(this->document(),
          &QTextDocument::contentsChange,
          this,
          &StylesheetEdit::handleDocumentChanged);
  //  connect(this,
  //          &QPlainTextEdit::textChanged,
  //          this,
  //          &StylesheetEdit::handleTextChanged);
  //  updateLineNumberAreaWidth(0);
}

StylesheetEditPrivate::StylesheetEditPrivate(StylesheetEdit* parent)
  : q_ptr(parent)
  , m_bookmarkArea(new BookmarkArea(q_ptr))
  , m_lineNumberArea(new LineNumberArea(q_ptr))
  , m_datastore(new DataStore(q_ptr))
  , m_highlighter(new StylesheetHighlighter(q_ptr))
  , m_nodes(new QMap<QTextCursor, Node*>())
  , m_braceCount(0)
  , m_bracesMatched(true)
  , m_startComment(false)
  , m_hoverWidget(nullptr)
  , m_manualMove(false)
{
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

  m_highlighter->rehighlight();
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
  m_lineNumberArea->setForeSelected(color);
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

int StylesheetEdit::lineNumberAreaWidth()
{
  return d_ptr->lineNumberAreaWidth();
}

int StylesheetEditPrivate::lineNumberAreaWidth()
{
  return m_lineNumberArea->lineNumberAreaWidth();
}

int StylesheetEdit::bookmarkAreaWidth()
{
  return d_ptr->bookmarkAreaWidth();
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
  m_bookmarkArea->setGeometry(
    QRect(cr.left(), cr.top(), bookmarkAreaWidth(), cr.height()));
  m_lineNumberArea->setGeometry(
    QRect(cr.left() + bookmarkAreaWidth(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void StylesheetEdit::initActions()
{
  m_formatAct = new QAction(tr("&Format"), this);
  m_formatAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
  m_formatAct->setStatusTip(tr("Prettyfy the stylesheet"));
  connect(m_formatAct, &QAction::triggered, this, &StylesheetEdit::format);

  m_addBookmarkAct = new QAction(tr("Add Bookmark"), this);
  connect(m_addBookmarkAct, &QAction::triggered, this, &StylesheetEdit::handleAddBookmark);
  m_removeBookmarkAct = new QAction(tr("Remove Bookmark"), this);
  connect(m_removeBookmarkAct, &QAction::triggered, this, &StylesheetEdit::handleRemoveBookmark);
  m_clearBookmarksAct = new QAction(tr("Clear Bookmarks"), this);
  connect(m_clearBookmarksAct, &QAction::triggered, this, &StylesheetEdit::handleClearBookmarks);
  m_gotoBookmarkAct = new QAction(tr("Go To Bookmark"), this);
  connect(m_gotoBookmarkAct, &QAction::triggered, this, &StylesheetEdit::handleGotoBookmark);
  m_editBookmarkAct = new QAction(tr("Edit Bookmark"), this);
  connect(m_editBookmarkAct, &QAction::triggered, this, &StylesheetEdit::handleEditBookmark);
}

void StylesheetEdit::initMenus()
{
  m_contextMenu = createStandardContextMenu();
  m_contextMenu->addSeparator();
  m_contextMenu->addAction(m_formatAct);

  m_bookmarkMenu = new QMenu(this);
  m_bookmarkMenu->addAction(m_addBookmarkAct);
  m_bookmarkMenu->addAction(m_editBookmarkAct);
  m_bookmarkMenu->addAction(m_removeBookmarkAct);
  m_bookmarkMenu->addSeparator();
  m_bookmarkMenu->addAction(m_gotoBookmarkAct);
  m_bookmarkMenu->addSeparator();
  m_bookmarkMenu->addAction(m_clearBookmarksAct);
}

void StylesheetEdit::contextMenuEvent(QContextMenuEvent* event)
{
  m_contextMenu->exec(event->globalPos());
}

void StylesheetEdit::contextBookmarkMenuEvent(QContextMenuEvent* event)
{
  auto tc = cursorForPosition(event->pos());
  int lineNumber = d_ptr->calculateLineNumber(tc);
  d_ptr->setBookmarkLineNumber(lineNumber);

  if (d_ptr->hasBookmark(lineNumber)) {
    m_addBookmarkAct->setEnabled(false);
    m_editBookmarkAct->setEnabled(true);
    m_removeBookmarkAct->setEnabled(true);

  } else {
    m_addBookmarkAct->setEnabled(true);
    m_editBookmarkAct->setEnabled(false);
    m_removeBookmarkAct->setEnabled(false);
  }

  if (d_ptr->bookmarkCount() == 0) {
    m_clearBookmarksAct->setEnabled(false);
    m_gotoBookmarkAct->setEnabled(false);

  } else {
    m_clearBookmarksAct->setEnabled(true);
    m_gotoBookmarkAct->setEnabled(true);
  }

  m_bookmarkMenu->exec(event->globalPos());
}

int StylesheetEdit::calculateLineNumber(QTextCursor textCursor)
{
  return d_ptr->calculateLineNumber(textCursor);
}

void StylesheetEdit::drawHoverWidget(QPoint pos, QString text)
{
  d_ptr->drawHoverWidget(pos, text);
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
  auto tc = q_ptr->cursorForPosition(pos);
  auto node = getNodeAtCursor(tc).node;

  if (node) {
    switch (node->type()) {
    case Node::BadNodeType: {
      BadBlockNode* badNode = qobject_cast<BadBlockNode*>(node);

      if (badNode) {
        displayError(badNode, pos);
      }

      break;
    }

    case Node::PropertyType: {
      PropertyNode* property = qobject_cast<PropertyNode*>(node);

      if (property->checks().contains(PropertyNode::BadValue) ||
          property->checks().contains(PropertyNode::MissingPropertyEnd) ||
          !property->hasPropertyMarker()) {
        displayError(property, pos);
      }
    }
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

int StylesheetEditor::StylesheetEditPrivate::calculateWidth(QString name, int offset, QFontMetrics fm)
{
  auto o = offset - name.length();

  for (int i = 0; i < o; i++) {
    name += " ";
  }

  return fm.horizontalAdvance(name);
}

void StylesheetEditPrivate::displayError(PropertyNode* property, QPoint pos)
{
  auto rect = q_ptr->cursorRect(property->cursor());
  auto left = rect.x()/* + m_lineNumberArea->width()*/;
  auto top = rect.y();
  auto x = pos.x();
  auto y = pos.y();
  auto fm = q_ptr->fontMetrics();
  auto offsets = property->offsets();
  auto values = property->values();
  auto checks = property->checks();
  auto height = fm.height();
  auto bottom = top + height;
  int width;

  if (y > top && y < bottom) {
    auto name = property->name();

    // jump over name.
    if (property->count() > 0) {
      width = calculateWidth(name, offsets.first(), fm);

      if (!property->hasPropertyMarker()) {
        if (x > left && x < left + width) {
          drawHoverWidget(pos, q_ptr->tr("Missing property marker."));
          return;
        }
      }

      left += width;
    }

    if (x < left) {
      return;
    }

    for (int i = 0; i < offsets.count(); i++) {
      auto check = checks.at(i);
      width = calculateWidth(values.at(i), offsets.at(i), fm);
      auto right = left + width;

      if (check == PropertyNode::BadValue || check == PropertyNode::MissingPropertyEnd) {

        if (x > left && x < right) {
          if (check == PropertyNode::BadValue) {
            drawHoverWidget(pos, q_ptr->tr("Bad property value."));

          } else if (check == PropertyNode::MissingPropertyEnd) {
            drawHoverWidget(pos, q_ptr->tr("Missing property end marker (;)."));
          }
        }
      }

      left += width;
    }
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
    oldValue = qobject_cast<PropertyNode*>(data->node)->name();
    break;

  case Node::SubControlType:
  case Node::PseudoStateType:
    oldValue = qobject_cast<PseudoStateNode*>(data->node)->name();
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

//void StylesheetEdit::updateLineNumberAreaWidth(int /*newBlockCount*/)
//{
//  setViewportMargins(bookmarkAreaWidth() + lineNumberAreaWidth(), 0, 0, 0);
//}

void StylesheetEdit::updateLeftAreaWidth(int)
{
  setViewportMargins(bookmarkAreaWidth() + lineNumberAreaWidth(), 0, 0, 0);
}

void StylesheetEdit::updateLeftArea(const QRect& rect, int dy)
{
  d_ptr->updateLeftArea(rect, dy);
}

//void StylesheetEdit::updateLineNumberArea(const QRect& rect, int dy)
//{
//  d_ptr->updateLineNumberArea(rect, dy);
//}

void StylesheetEditPrivate::updateLeftArea(const QRect& rect, int dy)
{
  if (dy) {
    m_bookmarkArea->scroll(0, dy);
    m_lineNumberArea->scroll(0, dy);

  } else {
    m_bookmarkArea->setLeft(0);
    m_lineNumberArea->setLeft(m_bookmarkArea->bookmarkAreaWidth());
    m_bookmarkArea->update(/* 0, rect.y(), m_bookmarkArea->bookmarkAreaWidth(), rect.height()*/);
    m_lineNumberArea->update(/* m_bookmarkArea->bookmarkAreaWidth(), rect.y(), m_lineNumberArea->width(), rect.height()*/);
  }

  if (rect.contains(q_ptr->viewport()->rect())) {
    q_ptr->updateLeftAreaWidth(0);
  }
}

void StylesheetEdit::handleCursorPositionChanged()
{
  d_ptr->handleCursorPositionChanged(textCursor());
}

int StylesheetEditor::StylesheetEditPrivate::calculateLineNumber(QTextCursor textCursor)
{
  QTextCursor cursor(textCursor);
  cursor.movePosition(QTextCursor::StartOfLine);

  int lines = 1;

  while (cursor.positionInBlock() > 0) {
    cursor.movePosition(QTextCursor::Up);
    lines++;
  }

  QTextBlock block = cursor.block().previous();

  while (block.isValid()) {
    lines += block.lineCount();
    block = block.previous();
  }

  return lines;
}

void StylesheetEditPrivate::handleCursorPositionChanged(QTextCursor textCursor)
{
  if (m_manualMove) {
    return;
  }

  int lineNumber = calculateLineNumber(textCursor);
  setLineNumber(lineNumber);

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

void StylesheetEdit::handleDocumentChanged(int pos, int charsRemoved, int charsAdded)
{
  d_ptr->onDocumentChanged(pos, charsRemoved, charsAdded);
}

void StylesheetEdit::handleTextChanged()
{
  //  d_ptr->handleTextChanged();
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

      } else if (newValue == ":") {
        int start = node->start();
        int end = node->end();
        int anchor = cursor.anchor();

        if (anchor > start && anchor < end) {
          // is in property.
          int nameEnd = start + property->name().length();

          if (!property->hasPropertyMarker() && anchor >= nameEnd) {
            QTextCursor propCursor(q_ptr->document());
            propCursor.setPosition(pos);
            PropertyMarkerNode* marker = new PropertyMarkerNode(propCursor, q_ptr);
            m_nodes->insert(propCursor, marker);
            property->setPropertyMarkerExists(true);
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

    q_ptr->document()->markContentsDirty(0, q_ptr->document()->toPlainText().length());
    m_highlighter->rehighlight();
    //          emit q_ptr->viewport()->update();
    //          q_ptr->repaint();
  }
}

void StylesheetEditPrivate::handleTextChanged()
{
  //  //  m_highlighter->rehighlight();
  //  QTextCursor cursor = q_ptr->textCursor();
  //  qWarning();
  //  m_highlighter->rehighlight();
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
      stashPropertyEndMarkerNode(property->end(),
                                 endnode);
      break;

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
            stashPropertyEndNode(property->end(), endnode);

          }/* else { // already a property end node (;)

            stashPropertyEndNode(property->end(), endnode);
          }*/

          // now go on to parse further properties.
          //          if (m_datastore->isValidPropertyValue(block))
          //          parsePropertyWithValues(text, pos - block.length(), pos, block, endnode);
          pos -= block.length(); // skip back before block.
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
            Node* endnode = nullptr;
            cursor = getCursorForNode(start);
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
                cursor = getCursorForNode(start);
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
      cursor = getCursorForNode(start);
      PropertyNode* property = new PropertyNode(block, cursor, q_ptr);
      m_nodes->insert(cursor, property);
      int end = parsePropertyWithValues(cursor, property, text, start, pos, block, &endnode);

      // run out of text.
      if (!endnode) {
        cursor = getCursorForNode(end);
        endnode = new PropertyEndNode(cursor, q_ptr);
        m_nodes->insert(cursor, endnode);
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

StylesheetEditPrivate::BookmarkArea::BookmarkArea(StylesheetEdit* editor)
  : QWidget(editor)
  , m_codeEditor(editor)
  , m_foreSelected(QColor("#808080"))
  , m_foreUnselected(QColor("#C5C5C5"))
  , m_back(QColor("#EEEFEF"))
  , m_width(15)
  , m_left(0)
{}

QSize StylesheetEditPrivate::BookmarkArea::sizeHint() const
{
  return QSize(m_codeEditor->bookmarkAreaWidth(), 0);
}

QColor StylesheetEditPrivate::BookmarkArea::foreSelected() const
{
  return m_foreSelected;
}

void StylesheetEditPrivate::BookmarkArea::setForeSelected(const QColor& fore)
{
  m_foreSelected = fore;
}

QColor StylesheetEditPrivate::BookmarkArea::foreUnselected() const
{
  return m_foreUnselected;
}

void StylesheetEditPrivate::BookmarkArea::setForeUnselected(const QColor& fore)
{
  m_foreUnselected = fore;
}

QColor StylesheetEditPrivate::BookmarkArea::back() const
{
  return m_back;
}

void StylesheetEditPrivate::BookmarkArea::setBack(const QColor& back)
{
  m_back = back;
}

int StylesheetEditPrivate::BookmarkArea::bookmarkAreaWidth() const
{
  return m_width;
}

void StylesheetEditPrivate::BookmarkArea::setWidth(int width)
{
  m_width = width;
}

void StylesheetEditPrivate::BookmarkArea::paintEvent(QPaintEvent* event)
{
  QRect rect;
  rect.setLeft(m_left);
  rect.setRight(m_left + event->rect().width());
  rect.setTop(event->rect().top());
  rect.setBottom(event->rect().bottom());

  QTextBlock block = m_codeEditor->firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top =
    qRound(m_codeEditor->blockBoundingGeometry(block).translated(m_codeEditor->contentOffset()).top());
  int bottom = top + qRound(m_codeEditor->blockBoundingRect(block).height());
  int height = m_codeEditor->fontMetrics().height();
  double blockHeight = m_codeEditor->blockBoundingRect(block).height();

  QPainter painter(this);
  painter.fillRect(rect, back());

  while (block.isValid() && top <= rect.bottom()) {
    if (block.isVisible() && bottom >= rect.top()) {
      int number = blockNumber + 1;

      if (m_bookmarks.contains(number)) {
        painter.setPen(m_foreSelected);
        painter.drawText(
          0, top, width(), height, Qt::AlignRight, StylesheetEdit::m_arrow);
      }
    }

    block = block.next();
    top = bottom;
    bottom = top + qRound(blockHeight);
    ++blockNumber;
  }
}

void StylesheetEditPrivate::BookmarkArea::mousePressEvent(QMouseEvent* event)
{
  QWidget::mousePressEvent(event);
}

void StylesheetEditPrivate::BookmarkArea::mouseMoveEvent(QMouseEvent* event)
{
  auto tc = m_codeEditor->cursorForPosition(event->pos());
  int lineNumber = m_codeEditor->calculateLineNumber(tc);

  if (m_bookmarks.contains(lineNumber)) {
    QString text = bookmarkText(lineNumber);

    if (text.isEmpty()) {
      m_codeEditor->drawHoverWidget(event->pos(), tr("Bookmark"));

    } else {
      m_codeEditor->drawHoverWidget(event->pos(), text);
    }
  }
}

void StylesheetEditPrivate::BookmarkArea::mouseReleaseEvent(QMouseEvent* event)
{
  QWidget::mouseMoveEvent(event);
}

void StylesheetEditPrivate::BookmarkArea::contextMenuEvent(QContextMenuEvent* event)
{
  m_codeEditor->contextBookmarkMenuEvent(event);
}

int StylesheetEditPrivate::bookmarkLineNumber() const
{
  return m_bookmarkLineNumber;
}

void StylesheetEditPrivate::setBookmarkLineNumber(int bookmarkLineNumber)
{
  m_bookmarkLineNumber = bookmarkLineNumber;
}

int StylesheetEditPrivate::BookmarkArea::left() const
{
  return m_left;
}

void StylesheetEditPrivate::BookmarkArea::setLeft(int left)
{
  m_left = left;
}

QMap<int, QString> StylesheetEditPrivate::BookmarkArea::bookmarks()
{
  return m_bookmarks;
}

void StylesheetEditPrivate::BookmarkArea::setBookmarks(QMap<int, QString> bookmarks)
{
  m_bookmarks = bookmarks;
  update();
}

void StylesheetEditPrivate::BookmarkArea::insertBookmark(int bookmark, const QString& text)
{
  if (m_bookmarks.contains(bookmark) && hasBookmarkText(bookmark)) {
    m_oldBookmarks.insert(bookmark, bookmarkText(bookmark));

  } else {
    m_bookmarks.insert(bookmark, text);
  }

  update();
}

void StylesheetEditPrivate::BookmarkArea::toggleBookmark(int bookmark)
{
  if (m_bookmarks.contains(bookmark)) {
    removeBookmark(bookmark);

  } else {
    if (m_oldBookmarks.contains(bookmark)) {
      insertBookmark(bookmark, m_oldBookmarks.value(bookmark));

    } else {
      insertBookmark(bookmark);
    }
  }
}

void StylesheetEditPrivate::BookmarkArea::removeBookmark(int bookmark)
{
  if (m_bookmarks.contains(bookmark)) {
    m_oldBookmarks.insert(bookmark, m_bookmarks.value(bookmark));
    m_bookmarks.remove(bookmark);
    update();
  }
}

void StylesheetEditPrivate::BookmarkArea::clearBookmarks()
{
  m_bookmarks.clear();
  update();
}

bool StylesheetEditPrivate::BookmarkArea::hasBookmark(int bookmark)
{
  return m_bookmarks.contains(bookmark);
}

bool StylesheetEditPrivate::BookmarkArea::hasBookmarkText(int bookmark)
{
  return !m_bookmarks.value(bookmark).isEmpty();
}

QString StylesheetEditPrivate::BookmarkArea::bookmarkText(int bookmark)
{
  return m_bookmarks.value(bookmark);
}

int StylesheetEditPrivate::BookmarkArea::count()
{
  return m_bookmarks.size();
}

StylesheetEditPrivate::LineNumberArea::LineNumberArea(StylesheetEdit* editor)
  : QWidget(editor)
  , m_codeEditor(editor)
  , m_foreSelected(QColor("#808080"))
  , m_foreUnselected(QColor("#C5C5C5"))
  , m_back(QColor("#EEEFEF"))
  , m_currentLineNumber(1)
  , m_left(0)
{}

QSize StylesheetEditPrivate::LineNumberArea::sizeHint() const
{
  return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
}

void StylesheetEditPrivate::LineNumberArea::paintEvent(QPaintEvent* event)
{
  QRect rect;
  rect.setLeft(m_left);
  rect.setRight(m_left + event->rect().width());
  rect.setTop(event->rect().top());
  rect.setBottom(event->rect().bottom());
  QTextBlock block = m_codeEditor->firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top =
    qRound(m_codeEditor->blockBoundingGeometry(block).translated(m_codeEditor->contentOffset()).top());
  int bottom = top + qRound(m_codeEditor->blockBoundingRect(block).height());
  int height = m_codeEditor->fontMetrics().height();
  double blockHeight = m_codeEditor->blockBoundingRect(block).height();

  QPainter painter(this);
  painter.fillRect(rect, back());

  while (block.isValid() && top <= rect.bottom()) {
    if (block.isVisible() && bottom >= rect.top()) {
      int number = blockNumber + 1;
      m_lineCount = blockNumber;

      if (number == m_currentLineNumber) {
        painter.setPen(m_foreSelected);

      } else {
        painter.setPen(m_foreUnselected);
      }

      painter.drawText(
        0, top, width(), height, Qt::AlignRight, QString::number(number));
    }

    block = block.next();
    top = bottom;
    bottom = top + qRound(blockHeight);
    ++blockNumber;
  }
}

int StylesheetEditPrivate::LineNumberArea::lineCount() const
{
  return m_lineCount;
}

int StylesheetEditPrivate::LineNumberArea::left() const
{
  return m_left;
}

void StylesheetEditPrivate::LineNumberArea::setLeft(int left)
{
  m_left = left;
}

int StylesheetEditPrivate::LineNumberArea::lineNumber() const
{
  return m_currentLineNumber;
}

int StylesheetEditPrivate::bookmarkAreaWidth()
{
  return m_bookmarkArea->bookmarkAreaWidth();
}

int StylesheetEditPrivate::LineNumberArea::lineNumberAreaWidth()
{
  int digits = 1;
  int max = qMax(1, m_codeEditor->blockCount());

  while (max >= 10) {
    max /= 10;
    ++digits;
  }

  int space =
    3 + m_codeEditor->fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

  return space;

}

QMap<int, QString> StylesheetEdit::bookmarks()
{
  return d_ptr->bookmarks();
}

QMap<int, QString> StylesheetEditPrivate::bookmarks()
{
  return m_bookmarkArea->bookmarks();
}

void StylesheetEdit::setBookmarks(QMap<int, QString> bookmarks)
{
  d_ptr->setBookmarks(bookmarks);
}

void StylesheetEditPrivate::setBookmarks(QMap<int, QString> bookmarks)
{
  m_bookmarkArea->setBookmarks(bookmarks);
}

void StylesheetEdit::handleAddBookmark(bool)
{
  d_ptr->insertBookmark();
}

void StylesheetEdit::insertBookmark(int bookmark, const QString& text)
{
  d_ptr->insertBookmark(bookmark, text);
}

void StylesheetEditPrivate::insertBookmark(int bookmark, const QString& text)
{
  int bm = (bookmark >= 0 ? bookmark : m_bookmarkLineNumber);
  m_bookmarkArea->insertBookmark(bm, text);
}

void StylesheetEdit::toggleBookmark(int bookmark)
{
  d_ptr->toggleBookmark(bookmark);
}

void StylesheetEditPrivate::toggleBookmark(int bookmark)
{
  m_bookmarkArea->toggleBookmark(bookmark);
}

void StylesheetEdit::editBookmark(int bookmark)
{
  d_ptr->editBookmark(bookmark);
}

void StylesheetEdit::handleEditBookmark(bool)
{
  d_ptr->editBookmark();
}

void StylesheetEditPrivate::editBookmark(int bookmark)
{
  int lineNumber = (bookmark >= 0 ? bookmark : m_bookmarkLineNumber);
  QString text = bookmarkText(lineNumber);

  BookmarkEditDialog dlg(q_ptr);
  dlg.setText(text);
  dlg.setLineNumber(lineNumber);
  auto result = dlg.exec();

  if (result == QDialog::Accepted) {
    auto ln = dlg.lineNumber();
    removeBookmark(lineNumber);
    insertBookmark(ln, dlg.text());
  }

}

void StylesheetEdit::removeBookmark(int bookmark)
{
  d_ptr->removeBookmark(bookmark);
}

void StylesheetEdit::handleRemoveBookmark(bool)
{
  d_ptr->removeBookmark();
}

void StylesheetEditPrivate::removeBookmark(int bookmark)
{
  int lineNumber = (bookmark >= 0 ? bookmark : m_bookmarkLineNumber);
  m_bookmarkArea->removeBookmark(lineNumber);
}

void StylesheetEdit::clearBookmarks()
{
  d_ptr->clearBookmarks();
}

void StylesheetEdit::handleClearBookmarks(bool)
{
  auto btn = QMessageBox::warning(
               this,
               tr("Clearing Book marks"),
               tr("You are about to remove ALL bookmarks!\nAre you sure?"),
               QMessageBox::Cancel | QMessageBox::Ok,
               QMessageBox::Cancel);

  if (btn == QMessageBox::Ok) {
    d_ptr->clearBookmarks();
  }
}

void StylesheetEditPrivate::clearBookmarks()
{
  m_bookmarkArea->clearBookmarks();
}

bool StylesheetEdit::hasBookmark(int bookmark)
{
  return d_ptr->hasBookmark(bookmark);
}

bool StylesheetEditPrivate::hasBookmark(int linenumber)
{
  return m_bookmarkArea->hasBookmark(linenumber);
}

bool StylesheetEdit::hasBookmarkText(int bookmark)
{
  return d_ptr->hasBookmarkText(bookmark);
}

bool StylesheetEditPrivate::hasBookmarkText(int bookmark)
{
  return m_bookmarkArea->hasBookmarkText(bookmark);
}

QString StylesheetEdit::bookmarkText(int bookmark)
{
  return d_ptr->bookmarkText(bookmark);
}

QString StylesheetEditPrivate::bookmarkText(int bookmark)
{
  return m_bookmarkArea->bookmarkText(bookmark);
}

void StylesheetEdit::gotoBookmark(int bookmark)
{
  d_ptr->gotoBookmark(bookmark);
}

void StylesheetEdit::gotoBookmarkDialog(bool)
{
  handleGotoBookmark();
}

void StylesheetEdit::handleGotoBookmark()
{
  GoToBookmarkDialog* dlg = new GoToBookmarkDialog(bookmarks(), this);

  if (dlg->exec() == QDialog::Accepted) {
    int bookmark = dlg->bookmark();

    if (bookmark != -1) {
      d_ptr->setLineNumber(bookmark);
    }
  }
}

void StylesheetEditPrivate::gotoBookmark(int bookmark)
{
  if (hasBookmark(bookmark)) {
    setLineNumber(bookmark);
  }
}

int StylesheetEdit::lineNumber() const
{
  return d_ptr->lineNumber();
}

int StylesheetEditPrivate::lineNumber() const
{
  return  m_lineNumberArea->lineNumber();
}

void StylesheetEdit::setLineNumber(int lineNumber)
{
  d_ptr->setLineNumber(lineNumber);
}

void StylesheetEditPrivate::setLineNumber(int lineNumber)
{
  m_manualMove = true;
  QTextCursor cursor(q_ptr->document());
  cursor.movePosition(QTextCursor::Start);
  cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber - 1);
  q_ptr->setTextCursor(cursor);

  // this handles display of linenumber
  m_lineNumberArea->setLineNumber(lineNumber);
  m_manualMove = false;
}

void StylesheetEdit::up(int n)
{
  d_ptr->up(n);
}

void StylesheetEditPrivate::up(int n)
{
  m_manualMove = true;
  int ln = lineNumber() - 1;

  if (ln > 0) {
    QTextCursor cursor(q_ptr->document());
    cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, n);
    q_ptr->setTextCursor(cursor);

    // this handles display of linenumber
    m_lineNumberArea->setLineNumber(ln);
  }

  m_manualMove = false;
}

void StylesheetEdit::down(int n)
{
  d_ptr->down(n);
}

void StylesheetEditPrivate::down(int n)
{
  m_manualMove = true;
  int ln = lineNumber() + 1;

  if (ln < m_lineNumberArea->lineCount()) {
    QTextCursor cursor(q_ptr->document());
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, n);
    q_ptr->setTextCursor(cursor);

    // this handles display of linenumber
    m_lineNumberArea->setLineNumber(ln);
  }

  m_manualMove = false;
}

void StylesheetEdit::left(int n)
{
  d_ptr->left(n);
}

void StylesheetEditPrivate::left(int n)
{
  m_manualMove = true;
  QTextCursor cursor(q_ptr->document());
  cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, n);
  q_ptr->setTextCursor(cursor);

  // this handles display of linenumber
  m_lineNumberArea->setLineNumber(calculateLineNumber(cursor));
  m_manualMove = false;
}

void StylesheetEdit::right(int n)
{
  d_ptr->right(n);
}

void StylesheetEditPrivate::right(int n)
{
  m_manualMove = true;
  QTextCursor cursor(q_ptr->document());
  cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, n);
  q_ptr->setTextCursor(cursor);

  // this handles display of linenumber
  m_lineNumberArea->setLineNumber(calculateLineNumber(cursor));
  m_manualMove = false;
}

void StylesheetEdit::start()
{
  d_ptr->start();
}

void StylesheetEditPrivate::start()
{
  m_manualMove = true;
  QTextCursor cursor(q_ptr->document());
  cursor.movePosition(QTextCursor::Start);
  q_ptr->setTextCursor(cursor);

  // this handles display of linenumber
  m_lineNumberArea->setLineNumber(1);
  m_manualMove = false;
}

void StylesheetEdit::end()
{
  d_ptr->end();
}

void StylesheetEditPrivate::end()
{
  m_manualMove = true;
  QTextCursor cursor(q_ptr->document());
  cursor.movePosition(QTextCursor::End);
  q_ptr->setTextCursor(cursor);

  // this handles display of linenumber
  m_lineNumberArea->setLineNumber(m_lineNumberArea->lineCount());
  m_manualMove = false;
}

void StylesheetEdit::startOfLine()
{
  d_ptr->startOfLine();
}

void StylesheetEditPrivate::startOfLine()
{
  m_manualMove = true;
  QTextCursor cursor(q_ptr->document());
  cursor.movePosition(QTextCursor::StartOfLine);
  q_ptr->setTextCursor(cursor);
  m_manualMove = false;
}

void StylesheetEdit::endOfLine()
{
  d_ptr->endOfLine();
}

void StylesheetEdit::goToLine(int lineNumber)
{
  d_ptr->setLineNumber(lineNumber);
}

//void StylesheetEditPrivate::goToLine(int lineNumber)
//{
//  m_manualMove = true;
//  QTextCursor cursor(q_ptr->document());
//  cursor.movePosition(QTextCursor::Start);
//  cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber - 1);
//  q_ptr->setTextCursor(cursor);

//  // this handles display of linenumber
//  m_lineNumberArea->setLineNumber(lineNumber);
//  m_manualMove = false;
//}

void StylesheetEditPrivate::endOfLine()
{
  m_manualMove = true;
  QTextCursor cursor(q_ptr->document());
  cursor.movePosition(QTextCursor::EndOfLine);
  q_ptr->setTextCursor(cursor);
  m_manualMove = false;
}

int StylesheetEdit::bookmarkCount()
{
  return d_ptr->bookmarkCount();
}

int StylesheetEditPrivate::bookmarkCount()
{
  return m_bookmarkArea->count();
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

QColor StylesheetEditPrivate::LineNumberArea::foreSelected() const
{
  return m_foreSelected;
}

void StylesheetEditPrivate::LineNumberArea::setForeSelected(const QColor& fore)
{
  m_foreSelected = fore;
}

QColor StylesheetEditPrivate::LineNumberArea::foreUnselected() const
{
  return m_foreUnselected;
}

void StylesheetEditPrivate::LineNumberArea::setForeUnselected(const QColor& fore)
{
  m_foreUnselected = fore;
}

void StylesheetEditPrivate::LineNumberArea::setLineNumber(int lineNumber)
{
  m_currentLineNumber = lineNumber;
  update();
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

BookmarkEditDialog::BookmarkEditDialog(QWidget* parent)
  : QDialog(parent)
{
  QVBoxLayout* layout = new QVBoxLayout;
  setLayout(layout);

  m_textEdit = new LabelledLineEdit(tr("Note Text:"), this);
  layout->addWidget(m_textEdit);
  m_linenumberEdit = new LabelledSpinBox(tr("Line Number"), this);
  layout->addWidget(m_linenumberEdit);

  m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);
  layout->addWidget(m_buttonBox);

  connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void BookmarkEditDialog::setText(const QString& text)
{
  m_textEdit->setText(text);
}

QString BookmarkEditDialog::text()
{
  return m_textEdit->text();
}

void BookmarkEditDialog::setLineNumber(int linenumber)
{
  m_linenumberEdit->setValue(linenumber);
}

int BookmarkEditDialog::lineNumber()
{
  return m_linenumberEdit->value();
}

GoToBookmarkDialog::GoToBookmarkDialog(QMap<int, QString> bookmarks, QWidget* parent)
  : QDialog(parent)
  , m_bookmark(-1)
  , m_text(QString())
{
  QVBoxLayout* layout = new QVBoxLayout;
  setLayout(layout);

  m_group = new QGroupBox(tr("Bookmarks"), this);
//  m_group->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_group);

  QVBoxLayout* grpLayout = new QVBoxLayout;
  grpLayout->setContentsMargins(0, 0, 0, 0);
  m_group->setLayout(grpLayout);

  BookmarkModel* bookmarkModel = new BookmarkModel(bookmarks);
  m_bookmarkView = new QTableView(this);
  m_bookmarkView->setModel(bookmarkModel);
  m_bookmarkView->verticalHeader()->hide();
  grpLayout->addWidget(m_bookmarkView);
  connect(m_bookmarkView, &QTableView::clicked, this, &GoToBookmarkDialog::handleClicked);

  m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);
  layout->addWidget(m_buttonBox);

  connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

}

int GoToBookmarkDialog::bookmark()
{
  return m_bookmark;
}

QString GoToBookmarkDialog::text()
{
  return m_text;
}

//bool GoToBookmarkDialog::setBookmark(int bookmark)
//{
//  return dynamic_cast<BookmarkModel*>(m_bookmarkView->model())->setBookmark(bookmark);
//}

void GoToBookmarkDialog::handleClicked(const QModelIndex& index)
{
  QModelIndex i0 = m_bookmarkView->model()->index(index.row(), 0);
  QModelIndex i1 = m_bookmarkView->model()->index(index.row(), 1);

  if (i0.isValid()) {
    m_bookmark = m_bookmarkView->model()->data(i0).toInt();
  }

  if (i1.isValid()) {
    m_text = m_bookmarkView->model()->data(i1).toString();
  }
}

BookmarkModel::BookmarkModel(QMap<int, QString> bookmarks)
  : QAbstractTableModel()
  , m_bookmarks(bookmarks)
{}

int BookmarkModel::columnCount(const QModelIndex&) const
{
  return 2;
}

int BookmarkModel::rowCount(const QModelIndex&) const
{
  return m_bookmarks.size();
}

QVariant BookmarkModel::data(const QModelIndex& index, int role) const
{
  if (index.isValid() && role == Qt::DisplayRole) {
    switch (index.column()) {
    case 0:
      return m_bookmarks[index.row()];

    case 1:
      return m_bookmarks.value(index.row());
    }
  }

  return QVariant();
}

Qt::ItemFlags BookmarkModel::flags(const QModelIndex& index) const
{
  if (index.isValid()) {
    return (QAbstractTableModel::flags(index));
  }

  return Qt::NoItemFlags;
}

QVariant BookmarkModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal) {
    if (section == 0) {
      return tr("Bookmark");

    } else {
      return tr("Message");
    }
  }

  return QVariant();
}

//bool BookmarkModel::setBookmark(int bookmark)
//{
//  if (m_bookmarks.contains(bookmark)) {

//    return true;
//  }
//  return false;
//}


} // end of StylesheetParser
