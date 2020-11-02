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
#include "bookmarkarea.h"
#include "datastore.h"
#include "hoverwidget.h"
#include "parser.h"
#include "stylesheetedit_p.h"
#include "stylesheethighlighter.h"

#include <QtDebug>

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
  //  setContextMenuPolicy(Qt::CustomContextMenu);

  //  connect(this,
  //          SIGNAL(customContextMenuRequested(const QPoint&)),
  //          SLOT(handleContextMenuRequested(const QPoint&)));
}

StylesheetEditPrivate::StylesheetEditPrivate(StylesheetEdit* parent)
  : q_ptr(parent)
  , m_bookmarkArea(new BookmarkArea(q_ptr))
  , m_lineNumberArea(new LineNumberArea(q_ptr))
  , m_highlighter(new StylesheetHighlighter(q_ptr))
{
  QThread* thread = new QThread;
  m_parser = new Parser(q_ptr);
  m_parser->moveToThread(thread);
  //  q_ptr->connect(worker, SIGNAL(error(QString)), this,
  //  SLOT(errorString(QString))); q_ptr->connect(thread, SIGNAL(started()),
  //  m_parser, SLOT(process()));
  q_ptr->connect(m_parser,
                 &Parser::rehighlight,
                 q_ptr,
                 &StylesheetEdit::handleParseComplete);
  q_ptr->connect(m_parser, &Parser::finished, thread, &QThread::quit);
  q_ptr->connect(m_parser, &Parser::finished, m_parser, &Parser::deleteLater);
  q_ptr->connect(thread, &QThread::finished, thread, &QThread::deleteLater);
  thread->start();

  m_hoverWidget = new HoverWidget(q_ptr);
  m_hoverWidget->setVisible(
    true); // always showing just 0 size when not needed.
  m_hoverWidget->setPosition(QPoint(0, 0));
  m_hoverWidget->hideHover();
}

void
StylesheetEditPrivate::initActions()
{
  m_addBookmarkAct = new QAction(q_ptr->tr("Add Bookmark"), q_ptr);
  q_ptr->connect(m_addBookmarkAct,
                 &QAction::triggered,
                 q_ptr,
                 &StylesheetEdit::handleAddBookmark);
  m_removeBookmarkAct = new QAction(q_ptr->tr("Remove Bookmark"), q_ptr);
  q_ptr->connect(m_removeBookmarkAct,
                 &QAction::triggered,
                 q_ptr,
                 &StylesheetEdit::handleRemoveBookmark);
  m_clearBookmarksAct = new QAction(q_ptr->tr("Clear Bookmarks"), q_ptr);
  q_ptr->connect(m_clearBookmarksAct,
                 &QAction::triggered,
                 q_ptr,
                 &StylesheetEdit::handleClearBookmarks);
  m_gotoBookmarkAct = new QAction(q_ptr->tr("Go To Bookmark"), q_ptr);
  q_ptr->connect(m_gotoBookmarkAct,
                 &QAction::triggered,
                 q_ptr,
                 &StylesheetEdit::handleGotoBookmark);
  m_editBookmarkAct = new QAction(q_ptr->tr("Edit Bookmark"), q_ptr);
  q_ptr->connect(m_editBookmarkAct,
                 &QAction::triggered,
                 q_ptr,
                 &StylesheetEdit::handleEditBookmark);
}

void
StylesheetEditPrivate::initMenus()
{
  m_contextMenu = m_parser->contextMenu();
  createBookmarkMenu();
}

int
StylesheetEditPrivate::maxSuggestionCount() const
{
  return m_parser->maxSuggestionCount();
}

void
StylesheetEditPrivate::setMaxSuggestionCount(int maxSuggestionCount)
{
  m_parser->setMaxSuggestionCount(maxSuggestionCount);
}

int
StylesheetEditPrivate::getLineCount() const
{
  return m_lineCount;
}

void
StylesheetEdit::setPlainText(const QString& text)
{
  QPlainTextEdit::setPlainText(text);
  d_ptr->setPlainText(text);
}

void
StylesheetEditPrivate::setPlainText(const QString& text)
{
  m_parser->parseInitialText(text);
}

void
StylesheetEdit::handleParseComplete()
{
  d_ptr->handleParseComplete();
}

void
StylesheetEditPrivate::handleParseComplete()
{
  m_highlighter->rehighlight();
}

QMap<QTextCursor, Node*>
StylesheetEdit::nodes()
{
  return d_ptr->nodes();
}

QMap<QTextCursor, Node*>
StylesheetEditPrivate::nodes()
{
  return m_parser->nodes();
}

void
StylesheetEdit::showNewlineMarkers(bool show)
{
  d_ptr->showNewlineMarkers(show);
}

void
StylesheetEditPrivate::showNewlineMarkers(bool show)
{
  // TODO show/hide newline markers.
  // maybe detect position of line end and draw a [NL] box??
}

QString
StylesheetEdit::styleSheet() const
{
  return d_ptr->styleSheet();
}

QString
StylesheetEditPrivate::styleSheet() const
{
  return m_stylesheet;
}

bool
StylesheetEditPrivate::checkStylesheetColors(StylesheetData* data,
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

void
StylesheetEdit::setStyleSheet(const QString& stylesheet)
{
  d_ptr->setStyleSheet(stylesheet);
}

void
StylesheetEditPrivate::setStyleSheet(const QString& stylesheet)
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
          data = m_parser->getStylesheetProperty(sheet, pos);

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

          } /* else if (data->name == "bad") {

             m_highlighter->setBadValueFormat(
               color1, color2, fontWeight, true, underlineStyle, color3);
             continue;
           }*/
        }
      }
    }
  }

  m_highlighter->rehighlight();
}

void
StylesheetEdit::setValueFormat(QColor color, QColor back, QFont::Weight weight)
{
  d_ptr->setValueFormat(color, back, weight);
}

void
StylesheetEditPrivate::setValueFormat(QColor color,
                                      QColor back,
                                      QFont::Weight weight)
{
  m_highlighter->setValueFormat(color, back, weight);
}

void
StylesheetEdit::setWidgetFormat(QColor color, QColor back, QFont::Weight weight)
{
  d_ptr->setWidgetFormat(color, back, weight);
}

void
StylesheetEditPrivate::setWidgetFormat(QColor color,
                                       QColor back,
                                       QFont::Weight weight)
{
  m_highlighter->setWidgetFormat(color, back, weight);
}

void
StylesheetEdit::setPseudoStateFormat(QColor color,
                                     QColor back,
                                     QFont::Weight weight)
{
  d_ptr->setPseudoStateFormat(color, back, weight);
}

void
StylesheetEditPrivate::setPseudoStateFormat(QColor color,
                                            QColor back,
                                            QFont::Weight weight)
{
  m_highlighter->setPseudoStateFormat(color, back, weight);
}

void
StylesheetEdit::setPseudoStateMarkerFormat(QColor color,
                                           QColor back,
                                           QFont::Weight weight)
{
  d_ptr->setPseudoStateMarkerFormat(color, back, weight);
}

void
StylesheetEditPrivate::setPseudoStateMarkerFormat(QColor color,
                                                  QColor back,
                                                  QFont::Weight weight)
{
  m_highlighter->setPseudoStateMarkerFormat(color, back, weight);
}

void
StylesheetEdit::setSubControlFormat(QColor color,
                                    QColor back,
                                    QFont::Weight weight)
{
  d_ptr->setSubControlFormat(color, back, weight);
}
void
StylesheetEditPrivate::setSubControlFormat(QColor color,
                                           QColor back,
                                           QFont::Weight weight)
{
  m_highlighter->setSubControlFormat(color, back, weight);
}

void
StylesheetEdit::setSubControlMarkerFormat(QColor color,
                                          QColor back,
                                          QFont::Weight weight)
{
  d_ptr->setSubControlMarkerFormat(color, back, weight);
}

void
StylesheetEditPrivate::setSubControlMarkerFormat(QColor color,
                                                 QColor back,
                                                 QFont::Weight weight)
{
  m_highlighter->setSubControlMarkerFormat(color, back, weight);
}

void
StylesheetEdit::setPropertyFormat(QColor color,
                                  QColor back,
                                  QFont::Weight weight)
{
  d_ptr->setPropertyFormat(color, back, weight);
}

void
StylesheetEditPrivate::setPropertyFormat(QColor color,
                                         QColor back,
                                         QFont::Weight weight)
{
  m_highlighter->setPropertyFormat(color, back, weight);
}

void
StylesheetEdit::setPropertyMarkerFormat(QColor color,
                                        QColor back,
                                        QFont::Weight weight)
{
  d_ptr->setPropertyMarkerFormat(color, back, weight);
}

void
StylesheetEditPrivate::setPropertyMarkerFormat(QColor color,
                                               QColor back,
                                               QFont::Weight weight)
{
  m_highlighter->setPropertyMarkerFormat(color, back, weight);
}

void
StylesheetEditPrivate::setPropertyEndMarkerFormat(QColor color,
                                                  QColor back,
                                                  QFont::Weight weight)
{
  m_highlighter->setPropertyMarkerFormat(color, back, weight);
}

void
StylesheetEdit::setLineNumberFormat(QColor color,
                                    QColor back,
                                    QFont::Weight weight)
{
  d_ptr->setLineNumberFormat(color, back, weight);
}

void
StylesheetEditPrivate::setLineNumberFormat(QColor color,
                                           QColor back,
                                           QFont::Weight weight)
{
  m_lineNumberArea->setForeSelected(color);
  m_lineNumberArea->setBack(back);
  m_lineNumberArea->setWeight(weight);
}

void
StylesheetEdit::setStartBraceFormat(QColor color,
                                    QColor back,
                                    QFont::Weight weight)
{
  d_ptr->setStartBraceFormat(color, back, weight);
}

void
StylesheetEditPrivate::setStartBraceFormat(QColor color,
                                           QColor back,
                                           QFont::Weight weight)
{
  m_highlighter->setStartBraceFormat(color, back, weight);
}

void
StylesheetEdit::setEndBraceFormat(QColor color,
                                  QColor back,
                                  QFont::Weight weight)
{
  d_ptr->setEndBraceFormat(color, back, weight);
}

void
StylesheetEditPrivate::setEndBraceFormat(QColor color,
                                         QColor back,
                                         QFont::Weight weight)
{
  m_highlighter->setEndBraceFormat(color, back, weight);
}

void
StylesheetEdit::setBraceMatchFormat(QColor color,
                                    QColor back,
                                    QFont::Weight weight)
{
  d_ptr->setBraceMatchFormat(color, back, weight);
}

void
StylesheetEditPrivate::setBraceMatchFormat(QColor color,
                                           QColor back,
                                           QFont::Weight weight)
{
  m_highlighter->setBraceMatchFormat(color, back, weight);
}

void
StylesheetEdit::resizeEvent(QResizeEvent* event)
{
  QPlainTextEdit::resizeEvent(event);

  d_ptr->resizeEvent(contentsRect());
}

void
StylesheetEdit::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::RightButton) {
    // context menu.
    d_ptr->handleMousePress(event->pos());
  } else {
    QPlainTextEdit::mousePressEvent(event);
  }
}

void
StylesheetEditPrivate::handleMousePress(const QPoint& pos)
{
  m_parser->handleMouseClicked(pos);
}

void
StylesheetEdit::mouseMoveEvent(QMouseEvent* event)
{
  QPlainTextEdit::mouseMoveEvent(event);
}

// void
// StylesheetEditPrivate::handleMouseMove(const QPoint &pos)
//{}

void
StylesheetEdit::mouseReleaseEvent(QMouseEvent* event)
{
  QPlainTextEdit::mouseReleaseEvent(event);
}

// void
// StylesheetEditPrivate::handleMouseRelease(const QPoint &pos)
//{
//  m_parser->handleMouseClicked(pos);
//}

void
StylesheetEdit::mouseDoubleClickEvent(QMouseEvent* event)
{
  QPlainTextEdit::mouseDoubleClickEvent(event);
}

void
StylesheetEdit::leaveEvent(QEvent* /*event*/)
{
  d_ptr->handleLeaveEvent();
}

void
StylesheetEditPrivate::resizeEvent(QRect cr)
{
  m_bookmarkArea->setGeometry(
    QRect(cr.left(), cr.top(), bookmarkAreaWidth(), cr.height()));
  m_lineNumberArea->setGeometry(QRect(cr.left() + bookmarkAreaWidth(),
                                      cr.top(),
                                      lineNumberAreaWidth(),
                                      cr.height()));
}

void
StylesheetEdit::initActions()
{
  d_ptr->initActions();
}

void
StylesheetEditPrivate::createBookmarkMenu()
{
  m_bookmarkMenu = new QMenu(q_ptr);
  m_bookmarkMenu->addAction(m_addBookmarkAct);
  m_bookmarkMenu->addAction(m_editBookmarkAct);
  m_bookmarkMenu->addAction(m_removeBookmarkAct);
  m_bookmarkMenu->addSeparator();
  m_bookmarkMenu->addAction(m_gotoBookmarkAct);
  m_bookmarkMenu->addSeparator();
  m_bookmarkMenu->addAction(m_clearBookmarksAct);
}

void
StylesheetEdit::initMenus()
{
  d_ptr->initMenus();
}

void
StylesheetEdit::contextMenuEvent(QContextMenuEvent* event)
{
  d_ptr->handleContextMenuEvent(event->globalPos());
}

void
StylesheetEdit::contextBookmarkMenuEvent(QContextMenuEvent* event)
{
  d_ptr->handleBookmarkMenuEvent(event->globalPos());
}

int
StylesheetEdit::maxSuggestionCount() const
{
  return d_ptr->maxSuggestionCount();
}

void
StylesheetEdit::setMaxSuggestionCount(int maxSuggestionCount)
{
  d_ptr->setMaxSuggestionCount(maxSuggestionCount);
}

int
StylesheetEdit::calculateLineNumber(QTextCursor textCursor)
{
  return d_ptr->calculateLineNumber(textCursor);
}

void
StylesheetEdit::format()
{
  // TODO format code nicely.
}

void
StylesheetEditPrivate::createHover()
{
  if (!m_hoverWidget) {
    m_hoverWidget = new HoverWidget(q_ptr);
    m_hoverWidget->setVisible(
      true); // always showing just 0 size when not needed.
    m_hoverWidget->setPosition(QPoint(0, 0));
    m_hoverWidget->hideHover();
  }
}

void
StylesheetEditPrivate::handleLeaveEvent()
{
  if (m_hoverWidget && m_hoverWidget->isVisible()) {
    m_hoverWidget->hideHover();
  }
}

void
StylesheetEditPrivate::displayBookmark(BookmarkData* data, QPoint pos)
{
  if (m_hoverWidget && m_hoverWidget->isVisible()) {
    m_hoverWidget->hideHover();
  }

  m_hoverWidget->setHoverText(data->text);
  m_hoverWidget->setPosition(pos);
}

QString
StylesheetEditPrivate::getValueAtCursor(int pos, const QString& text)
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

//QString
//StylesheetEditPrivate::getOldNodeValue(CursorData* data)
//{
//  QString oldValue;

//  switch (data->node->type()) {
//    case NodeType::WidgetType:
//    case NodeType::PropertyType:
//      oldValue = qobject_cast<PropertyNode*>(data->node)->name();
//      break;

//    case NodeType::StartBraceType:
//      oldValue = "{";
//      break;

//    case NodeType::EndBraceType:
//      oldValue = "}";
//      break;

//      //  case Node::ValueType:

//    default:
//      break;
//  }

//  return oldValue;
//}

void
StylesheetEdit::updateLeftAreaWidth(int)
{
  setViewportMargins(bookmarkAreaWidth() + lineNumberAreaWidth(), 0, 0, 0);
}

void
StylesheetEdit::updateLeftArea(const QRect& rect, int dy)
{
  d_ptr->updateLeftArea(rect, dy);
}

void
StylesheetEditPrivate::updateLeftArea(const QRect& rect, int dy)
{
  if (dy) {
    m_bookmarkArea->scroll(0, dy);
    m_lineNumberArea->scroll(0, dy);

  } else {
    m_bookmarkArea->setLeft(0);
    m_lineNumberArea->setLeft(m_bookmarkArea->bookmarkAreaWidth());
    m_bookmarkArea->update(
      /* 0, rect.y(), m_bookmarkArea->bookmarkAreaWidth(), rect.height()*/);
    m_lineNumberArea->update(/* m_bookmarkArea->bookmarkAreaWidth(), rect.y(), m_lineNumberArea->width(), rect.height()*/);
  }

  if (rect.contains(q_ptr->viewport()->rect())) {
    q_ptr->updateLeftAreaWidth(0);
  }
}

/*
   Calculates current line number and total line count.
*/
int
StylesheetEditPrivate::calculateLineNumber(QTextCursor textCursor)
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

  int count = lines;

  block = cursor.block().next();

  while (block.isValid()) {
    count += block.lineCount();
    block = block.next();
  }

  m_lineCount = count;

  return lines;
}

/*
   Calculates the current text column.
*/
int
StylesheetEditPrivate::calculateColumn(QTextCursor textCursor)
{
  QTextCursor cursor(textCursor);
  cursor.movePosition(QTextCursor::StartOfLine);
  return textCursor.anchor() - cursor.anchor();
}

void
StylesheetEdit::handleCursorPositionChanged()
{
  d_ptr->handleCursorPositionChanged(textCursor());
}

void
StylesheetEditPrivate::handleCursorPositionChanged(QTextCursor textCursor)
{
  m_parser->handleCursorPositionChanged(textCursor);
}

void
StylesheetEditPrivate::handleSuggestion(QAction* act)
{
  m_parser->handleSuggestion(act);
}

void
StylesheetEdit::handleDocumentChanged(int pos, int charsRemoved, int charsAdded)
{
  d_ptr->onDocumentChanged(pos, charsRemoved, charsAdded);
}

void
StylesheetEdit::handleTextChanged()
{
  d_ptr->handleTextChanged();
}

void
StylesheetEditPrivate::onDocumentChanged(int pos,
                                         int charsRemoved,
                                         int charsAdded)
{
  m_parser->handleDocumentChanged(pos, charsRemoved, charsAdded);
}

void
StylesheetEditPrivate::handleTextChanged()
{
  qWarning();
  // TODO possibly remove this.
  //  //  m_highlighter->rehighlight();
  //  QTextCursor cursor = q_ptr->textCursor();
  //  qWarning();
  //  m_highlighter->rehighlight();
}

int
StylesheetEditPrivate::bookmarkLineNumber() const
{
  return m_bookmarkLineNumber;
}

void
StylesheetEditPrivate::setBookmarkLineNumber(int bookmarkLineNumber)
{
  m_bookmarkLineNumber = bookmarkLineNumber;
}

LineNumberArea::LineNumberArea(StylesheetEdit* editor)
  : QWidget(editor)
  , m_codeEditor(editor)
  , m_foreSelected(QColor("#808080"))
  , m_foreUnselected(QColor("#C5C5C5"))
  , m_back(QColor("#EEEFEF"))
  , m_currentLineNumber(1)
  , m_left(0)
{}

QSize
LineNumberArea::sizeHint() const
{
  return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
}

void
LineNumberArea::paintEvent(QPaintEvent* event)
{
  QRect rect;
  rect.setLeft(m_left);
  rect.setRight(m_left + event->rect().width());
  rect.setTop(event->rect().top());
  rect.setBottom(event->rect().bottom());
  QTextBlock block = m_codeEditor->firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = qRound(m_codeEditor->blockBoundingGeometry(block)
                     .translated(m_codeEditor->contentOffset())
                     .top());
  int bottom = top + qRound(m_codeEditor->blockBoundingRect(block).height());
  int height = m_codeEditor->fontMetrics().height();
  double blockHeight = m_codeEditor->blockBoundingRect(block).height();

  QPainter painter(this);
  painter.fillRect(rect, back());

  while (block.isValid()) {
    if (top <= rect.bottom()) {
      if (block.isVisible() && bottom >= rect.top()) {
        int number = blockNumber + 1;

        if (number == m_currentLineNumber) {
          painter.setPen(m_foreSelected);

        } else {
          painter.setPen(m_foreUnselected);
        }

        painter.drawText(
          0, top, width(), height, Qt::AlignRight, QString::number(number));
      }
    }

    block = block.next();
    top = bottom;
    bottom = top + qRound(blockHeight);
    ++blockNumber;
  }
}

int
LineNumberArea::left() const
{
  return m_left;
}

void
LineNumberArea::setLeft(int left)
{
  m_left = left;
}

int
LineNumberArea::currentLineNumber() const
{
  return m_currentLineNumber;
}

int
StylesheetEdit::bookmarkAreaWidth()
{
  return d_ptr->bookmarkAreaWidth();
}

int
StylesheetEditPrivate::bookmarkAreaWidth()
{
  return m_bookmarkArea->bookmarkAreaWidth();
}

int
StylesheetEdit::lineNumberAreaWidth()
{
  return d_ptr->lineNumberAreaWidth();
}

int
StylesheetEditPrivate::lineNumberAreaWidth()
{
  return m_lineNumberArea->lineNumberAreaWidth();
}

int
LineNumberArea::lineNumberAreaWidth()
{
  int digits = 1;
  int max = qMax(1, m_codeEditor->blockCount());

  while (max >= 10) {
    max /= 10;
    ++digits;
  }

  int space =
    3 +
    m_codeEditor->fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

  return space;
}

QMap<int, BookmarkData*>*
StylesheetEdit::bookmarks()
{
  return d_ptr->bookmarks();
}

QMap<int, BookmarkData*>*
StylesheetEditPrivate::bookmarks()
{
  return m_bookmarkArea->bookmarks();
}

void
StylesheetEdit::setBookmarks(QMap<int, BookmarkData*>* bookmarks)
{
  d_ptr->setBookmarks(bookmarks);
}

void
StylesheetEditPrivate::setBookmarks(QMap<int, BookmarkData*>* bookmarks)
{
  m_bookmarkArea->setBookmarks(bookmarks);
}

void
StylesheetEdit::handleAddBookmark(bool)
{
  d_ptr->insertBookmark();
}

void
StylesheetEdit::insertBookmark(int bookmark, const QString& text)
{
  d_ptr->insertBookmark(bookmark, text);
}

void
StylesheetEditPrivate::insertBookmark(int bookmark, const QString& text)
{
  int bm = (bookmark >= 0 ? bookmark : m_bookmarkLineNumber);
  m_bookmarkArea->insertBookmark(bm, text);
}

void
StylesheetEdit::toggleBookmark(int bookmark)
{
  d_ptr->toggleBookmark(bookmark);
}

void
StylesheetEditPrivate::toggleBookmark(int bookmark)
{
  m_bookmarkArea->toggleBookmark(bookmark);
}

void
StylesheetEdit::editBookmark(int bookmark)
{
  d_ptr->editBookmark(bookmark);
}

void
StylesheetEdit::handleEditBookmark(bool)
{
  d_ptr->editBookmark();
}

void
StylesheetEditPrivate::editBookmark(int bookmark)
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

void
StylesheetEdit::removeBookmark(int bookmark)
{
  d_ptr->removeBookmark(bookmark);
}

void
StylesheetEdit::handleRemoveBookmark(bool)
{
  d_ptr->removeBookmark();
}

void
StylesheetEditPrivate::removeBookmark(int bookmark)
{
  int lineNumber = (bookmark >= 0 ? bookmark : m_bookmarkLineNumber);
  m_bookmarkArea->removeBookmark(lineNumber);
}

void
StylesheetEdit::clearBookmarks()
{
  d_ptr->clearBookmarks();
}

void
StylesheetEdit::handleClearBookmarks(bool)
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

void
StylesheetEditPrivate::clearBookmarks()
{
  m_bookmarkArea->clearBookmarks();
}

bool
StylesheetEdit::hasBookmark(int bookmark)
{
  return d_ptr->hasBookmark(bookmark);
}

bool
StylesheetEditPrivate::hasBookmark(int linenumber)
{
  return m_bookmarkArea->hasBookmark(linenumber);
}

bool
StylesheetEdit::hasBookmarkText(int bookmark)
{
  return d_ptr->hasBookmarkText(bookmark);
}

bool
StylesheetEditPrivate::hasBookmarkText(int bookmark)
{
  return m_bookmarkArea->hasBookmarkText(bookmark);
}

QString
StylesheetEdit::bookmarkText(int bookmark)
{
  return d_ptr->bookmarkText(bookmark);
}

QString
StylesheetEditPrivate::bookmarkText(int bookmark)
{
  return m_bookmarkArea->bookmarkText(bookmark);
}

void
StylesheetEdit::gotoBookmark(int bookmark)
{
  d_ptr->gotoBookmark(bookmark);
}

void
StylesheetEdit::gotoBookmarkDialog(bool)
{
  handleGotoBookmark();
}

void
StylesheetEdit::handleGotoBookmark()
{
  GoToBookmarkDialog* dlg = new GoToBookmarkDialog(bookmarks(), this);

  if (dlg->exec() == QDialog::Accepted) {
    int bookmark = dlg->bookmark();

    if (bookmark != -1) {
      d_ptr->setLineNumber(bookmark);
    }
  }
}

void
StylesheetEditPrivate::gotoBookmark(int bookmark)
{
  if (hasBookmark(bookmark)) {
    setLineNumber(bookmark);
  }
}

void
StylesheetEditPrivate::handleContextMenuEvent(QPoint pos)
{
  m_contextMenu->exec(pos);
}

void
StylesheetEditPrivate::handleBookmarkMenuEvent(QPoint pos)
{
  auto tc = q_ptr->cursorForPosition(pos);
  int lineNumber = calculateLineNumber(tc);
  setBookmarkLineNumber(lineNumber);

  if (hasBookmark(lineNumber)) {
    m_addBookmarkAct->setEnabled(false);
    m_editBookmarkAct->setEnabled(true);
    m_removeBookmarkAct->setEnabled(true);

  } else {
    m_addBookmarkAct->setEnabled(true);
    m_editBookmarkAct->setEnabled(false);
    m_removeBookmarkAct->setEnabled(false);
  }

  if (bookmarkCount() == 0) {
    m_clearBookmarksAct->setEnabled(false);
    m_gotoBookmarkAct->setEnabled(false);

  } else {
    m_clearBookmarksAct->setEnabled(true);
    m_gotoBookmarkAct->setEnabled(true);
  }

  m_bookmarkMenu->exec(pos);
}

QTextCursor
StylesheetEditPrivate::currentCursor() const
{
  return m_parser->currentCursor();
}

void
StylesheetEditPrivate::setCurrentCursor(const QTextCursor& currentCursor)
{
  m_parser->setCurrentCursor(currentCursor);
  setLineData(currentCursor);
}

int
StylesheetEdit::currentLineNumber() const
{
  return d_ptr->currentLineNumber();
}

int
StylesheetEditPrivate::currentLineNumber() const
{
  return m_lineNumberArea->currentLineNumber();
}

int
StylesheetEditPrivate::currentLineCount() const
{
  return m_lineCount;
}

void
StylesheetEdit::setLineNumber(int lineNumber)
{
  d_ptr->setLineNumber(lineNumber);
}

void
StylesheetEdit::suggestion(bool)
{
  auto act = dynamic_cast<QAction*>(sender());

  if (act) {
    d_ptr->handleSuggestion(act);
  }
}

void
StylesheetEditPrivate::setLineNumber(int linenumber)
{
  m_parser->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Start);
  cursor.movePosition(
    QTextCursor::Down, QTextCursor::MoveAnchor, linenumber - 1);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_parser->setManualMove(false);
}

void
StylesheetEdit::up(bool)
{
  up(1);
}

void
StylesheetEdit::up(int n)
{
  d_ptr->up(n);
}

void
StylesheetEditPrivate::setLineData(QTextCursor cursor)
{
  // this handles display of linenumber, linecount and character column.
  int ln = calculateLineNumber(cursor);
  m_lineNumberArea->setLineNumber(ln);
  emit q_ptr->lineNumber(ln);
  emit q_ptr->column(calculateColumn(cursor));
  emit q_ptr->lineCount(m_lineCount);
}

void
StylesheetEditPrivate::up(int n)
{
  m_parser->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, n);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_parser->setManualMove(false);
}

void
StylesheetEdit::down(bool)
{
  down(1);
}

void
StylesheetEdit::down(int n)
{
  d_ptr->down(n);
}

void
StylesheetEditPrivate::down(int n)
{
  m_parser->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, n);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_parser->setManualMove(false);
}

void
StylesheetEdit::left(bool)
{
  left(1);
}

void
StylesheetEdit::left(int n)
{
  d_ptr->left(n);
}

void
StylesheetEditPrivate::left(int n)
{
  m_parser->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, n);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_parser->setManualMove(false);
}

void
StylesheetEdit::right(bool)
{
  right(1);
}

void
StylesheetEdit::right(int n)
{
  d_ptr->right(n);
}

void
StylesheetEditPrivate::right(int n)
{
  m_parser->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, n);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_parser->setManualMove(false);
}

void
StylesheetEdit::start()
{
  d_ptr->start();
}

void
StylesheetEditPrivate::start()
{
  m_parser->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Start);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_parser->setManualMove(false);
}

void
StylesheetEdit::end()
{
  d_ptr->end();
}

void
StylesheetEditPrivate::end()
{
  m_parser->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::End);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_parser->setManualMove(false);
}

void
StylesheetEdit::startOfLine()
{
  d_ptr->startOfLine();
}

void
StylesheetEditPrivate::startOfLine()
{
  m_parser->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::StartOfLine);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_parser->setManualMove(false);
}

void
StylesheetEdit::endOfLine()
{
  d_ptr->endOfLine();
}

void
StylesheetEditPrivate::endOfLine()
{
  m_parser->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::EndOfLine);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_parser->setManualMove(false);
}

void
StylesheetEdit::goToLine(int lineNumber)
{
  d_ptr->setLineNumber(lineNumber);
}

int
StylesheetEdit::bookmarkCount()
{
  return d_ptr->bookmarkCount();
}

int
StylesheetEditPrivate::bookmarkCount()
{
  return m_bookmarkArea->count();
}

QFont::Weight
LineNumberArea::weight() const
{
  return m_weight;
}

void
LineNumberArea::setWeight(const QFont::Weight& weight)
{
  m_weight = weight;
}

QColor
LineNumberArea::back() const
{
  return m_back;
}

void
LineNumberArea::setBack(const QColor& back)
{
  m_back = back;
}

QColor
LineNumberArea::foreSelected() const
{
  return m_foreSelected;
}

void
LineNumberArea::setForeSelected(const QColor& fore)
{
  m_foreSelected = fore;
}

QColor
LineNumberArea::foreUnselected() const
{
  return m_foreUnselected;
}

void
LineNumberArea::setForeUnselected(const QColor& fore)
{
  m_foreUnselected = fore;
}

void
LineNumberArea::setLineNumber(int lineNumber)
{
  m_currentLineNumber = lineNumber;
}
