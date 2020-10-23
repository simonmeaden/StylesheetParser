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
  //  , m_hoverWidget(nullptr)
  , m_manualMove(false)
  , m_maxSuggestionCount(5)
{
  m_hoverWidget = new HoverWidget(q_ptr);
  m_hoverWidget->setVisible(
    true); // always showing just 0 size when not needed.
  m_hoverWidget->setPosition(QPoint(0, 0));
  m_hoverWidget->hideHover();
}

void
StylesheetEditPrivate::initActions()
{
  m_formatAct = new QAction(q_ptr->tr("&Format"), q_ptr);
  m_formatAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
  m_formatAct->setStatusTip(q_ptr->tr("Prettyfy the stylesheet"));
  q_ptr->connect(
    m_formatAct, &QAction::triggered, q_ptr, &StylesheetEdit::format);

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
  m_contextMenu = createContextMenu();
  createBookmarkMenu();
}

int
StylesheetEditPrivate::maxSuggestionCount() const
{
  return m_maxSuggestionCount;
}

void
StylesheetEditPrivate::setMaxSuggestionCount(int maxSuggestionCount)
{
  m_maxSuggestionCount = maxSuggestionCount;
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
  parseInitialText(text);
  m_highlighter->rehighlight();
}

QMap<QTextCursor, Node*>*
StylesheetEdit::nodes()
{
  return d_ptr->nodes();
}

QMap<QTextCursor, Node*>*
StylesheetEditPrivate::nodes()
{
  return m_nodes;
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

StylesheetData*
StylesheetEditPrivate::getStylesheetProperty(const QString& sheet, int& pos)
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

// void StylesheetEdit::setBadValueFormat(
//  QColor color,
//  QColor back,
//  QFont::Weight weight,
//  bool underline,
//  QTextCharFormat::UnderlineStyle underlineStyle,
//  QColor underlineColor)
//{
//  d_ptr->setBadValueFormat(
//    color, back, weight, underline, underlineStyle, underlineColor);
//}

// void StylesheetEditPrivate::setBadValueFormat(
//  QColor color,
//  QColor back,
//  QFont::Weight weight,
//  bool underline,
//  QTextCharFormat::UnderlineStyle underlineStyle,
//  QColor underlineColor)
//{
//  m_highlighter->setBadValueFormat(
//    color, back, weight, underline, underlineStyle, underlineColor);
//}

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
  QPlainTextEdit::mouseMoveEvent(event);
}

void
StylesheetEdit::mouseMoveEvent(QMouseEvent* event)
{
  d_ptr->handleMouseMove(event->pos());
}

void
StylesheetEdit::mouseReleaseEvent(QMouseEvent* event)
{
  QPlainTextEdit::mouseReleaseEvent(event);
}

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

QMenu*
StylesheetEditPrivate::createContextMenu()
{
  QMenu* menu = q_ptr->createStandardContextMenu();
  menu->addSeparator();
  menu->addAction(m_formatAct);
  return menu;
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

QPair<NameNode::SectionType, int>
StylesheetEditPrivate::nodeForPoint(const QPoint& pos, NamedNode** nNode)
{
  QPair<NameNode::SectionType, int> isin =
    qMakePair<NameNode::SectionType, int>(NamedNode::None, -1);

  for (auto node : m_nodes->values()) {
    *nNode = qobject_cast<NamedNode*>(node);

    if (*nNode) {
      isin = (*nNode)->isIn(pos);

      if (isin.first != NamedNode::None) {
        return isin;
      }
    }
  }

  return isin;
}

void
StylesheetEditPrivate::hoverWidgetType(NamedNode* nNode,
                                       QPair<NameNode::SectionType, int> isin,
                                       QPoint pos)
{
  auto widget = qobject_cast<WidgetNode*>(nNode);

  if (isin.first == WidgetNode::Name) {
    if (!widget->isWidgetValid()) {
      // not a valid node
      if (widget == m_currentHover) {
        // already has a message just changing position
        m_hoverWidget->setPosition(pos);

      } else {
        // new node so restart message
        m_hoverWidget->setHoverText(
          q_ptr->tr("%1 is not a valid widget name").arg(widget->name()));
        m_hoverWidget->setPosition(pos);
        m_currentHover = widget;
      }
    }
  }
}

void
StylesheetEditPrivate::hoverPseudoStateType(
  NamedNode* nNode,
  QPair<NameNode::SectionType, int> isin,
  QPoint pos)
{
  auto pseudostate = qobject_cast<PseudoStateNode*>(nNode);

  if (isin.first == PseudoStateNode::Name) {
    if (!pseudostate->isStateValid()) {
      // not a valid node
      if (pseudostate == m_currentHover) {
        // already has a message just changing position
        m_hoverWidget->setPosition(pos);

      } else {
        // new node so restart message
        m_hoverWidget->setHoverText(
          q_ptr->tr("%1 is not a valid pseudo state").arg(pseudostate->name()));
        m_hoverWidget->setPosition(pos);
        m_currentHover = pseudostate;
      }
    }
  }
}

void
StylesheetEditPrivate::hoverSubControlType(
  NamedNode* nNode,
  QPair<NameNode::SectionType, int> isin,
  QPoint pos)
{
  auto subcontrol = qobject_cast<SubControlNode*>(nNode);

  if (isin.first == PseudoStateNode::Name) {
    if (!subcontrol->isStateValid()) {
      // not a valid node
      if (subcontrol == m_currentHover) {
        // already has a message just changing position
        m_hoverWidget->setPosition(pos);

      } else {
        // new node so restart message
        m_hoverWidget->setHoverText(
          q_ptr->tr("%1 is not a valid sub-control").arg(subcontrol->name()));
        m_hoverWidget->setPosition(pos);
        m_currentHover = subcontrol;
      }
    }
  }
}

void
StylesheetEditPrivate::hoverPropertyType(NamedNode* nNode,
                                         QPair<NameNode::SectionType, int> isin,
                                         QPoint pos)
{
  auto property = qobject_cast<PropertyNode*>(nNode);

  switch (isin.first) {
    case PropertyNode::None:
      m_hoverWidget->hideHover();
      break;

    case PropertyNode::Name: {
      auto propertyName = property->name();

      if (property->checks().contains(PropertyNode::MissingPropertyEnd) &&
          !property->hasPropertyMarker()) {
        m_hoverWidget->setHoverText(
          q_ptr
            ->tr("%1 is not a valid property name!\nMissing property "
                 "marker (:)!")
            .arg(propertyName));
        m_hoverWidget->setPosition(pos);

      } else if (property->checks().contains(
                   PropertyNode::MissingPropertyEnd)) {
        m_hoverWidget->setHoverText(
          q_ptr->tr("%1 is not a valid property name!").arg(propertyName));
        m_hoverWidget->setPosition(pos);

      } else if (!property->hasPropertyMarker()) {
        m_hoverWidget->setHoverText(q_ptr->tr("Missing property marker (:)!"));
        m_hoverWidget->setPosition(pos);

      } else if (!property->isValidProperty()) {
        m_hoverWidget->setHoverText(q_ptr->tr("Invalid property name!"));
        m_hoverWidget->setPosition(pos);
      }

      break;
    }

    case PropertyNode::Value: {
      /*if (property->checks().at(isin.second) ==
          PropertyNode::ValidPropertyType) {
        // This is a valid property but the property name is wrong.
        // TODO handle different attribute types here?
        m_hoverWidget->setHoverText(
          q_ptr->tr("%1 is a good property value.\nIncorrect property name!")
          .arg(property->values().at(isin.second)));
        m_hoverWidget->setPosition(pos);

        } else*/
      if (property->checks().at(isin.second) == PropertyNode::BadValue) {
        m_hoverWidget->setHoverText(q_ptr->tr("%1 is a bad property value.")
                                      .arg(property->values().at(isin.second)));
        m_hoverWidget->setPosition(pos);
      }

      break;
    }
  }
}

void
StylesheetEditPrivate::handleMouseMove(QPoint pos)
{
  //  NamedNode* nNode = nullptr;
  //  auto isin = nodeForPoint(pos, &nNode);

  //  if (!nNode || isin.first == NamedNode::None) {
  //    m_hoverWidget->hideHover();
  //    return;

  //  } else {
  //    switch (nNode->type()) {

  //      case Node::WidgetType: {
  //        hoverWidgetType(nNode, isin, pos);
  //        break;
  //      }

  //      case Node::PseudoStateType: {
  //        hoverPseudoStateType(nNode, isin, pos);
  //        break;
  //      }

  //      case Node::SubControlType: {
  //        hoverSubControlType(nNode, isin, pos);
  //        break;
  //      }

  //      case Node::PropertyType: {
  //        hoverPropertyType(nNode, isin, pos);
  //        break;
  //      }

  //      case Node::BadNodeType: {
  //        auto badNode = qobject_cast<BadBlockNode*>(nNode);

  //        //      if (badNode) {
  //        //        ParserState::Errors errors = badNode->errors();

  //        //        if (errors.testFlag(ParserState::InvalidWidget)) {
  //        //          m_hoverWidget->show(pos, q_ptr->tr("This is not a valid
  //        //          Widget"));

  //        //        } else if
  //        (errors.testFlag(ParserState::InvalidSubControl)) {
  //        //          m_hoverWidget->show(pos, q_ptr->tr("This is not a valid
  //        //          Sub-Control"));

  //        //        } else if
  //        (errors.testFlag(ParserState::InvalidPseudoState)) {
  //        //          m_hoverWidget->show(pos, q_ptr->tr("This is not a valid
  //        //          Pseudo-State"));

  //        //        } else if
  //        (errors.testFlag(ParserState::AnomalousMarkerType))
  //        //        {
  //        //          m_hoverWidget->show(
  //        //            pos,
  //        //            q_ptr->tr(
  //        //              "This could be either a Pseudo-State marker or a
  //        //              Sub-Control marker."));

  //        //        } else if (errors.testFlag(ParserState::AnomalousType)) {
  //        //          m_hoverWidget->show(pos, q_ptr->tr("The type of this is
  //        //          anomalous."));

  //        //        } else {
  //        //          hideHover();
  //        //        }

  //        //      }

  //        break;
  //      } // end case Node::BadNodeType

  //      default:
  //        m_hoverWidget->hideHover();
  //    }
  //  }
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

QString
StylesheetEditPrivate::getOldNodeValue(CursorData* data)
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

// void StylesheetEdit::updateLineNumberAreaWidth(int /*newBlockCount*/)
//{
//  setViewportMargins(bookmarkAreaWidth() + lineNumberAreaWidth(), 0, 0, 0);
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

// void StylesheetEdit::updateLineNumberArea(const QRect& rect, int dy)
//{
//  d_ptr->updateLineNumberArea(rect, dy);
//}

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

QList<int>
StylesheetEditPrivate::reverseLastNValues(QMap<int, QString> matches)
{

  QMap<int, QString> rMatches;
  auto keys = matches.keys();
  QList<int> reversed;
  QList<int>::reverse_iterator i;

  for (i = keys.rbegin(); i != keys.rend(); ++i) {
    if (reversed.size() > m_maxSuggestionCount) {
      break;
    }

    reversed << *i;
  }

  return reversed;
}

void
StylesheetEditPrivate::updateContextMenu(QMenu* contextMenu,
                                         QMap<int, QString> matches,
                                         NamedNode* nNode)
{
  auto first = contextMenu->actions().first();
  QString typeName;

  switch (nNode->type()) {
    case Node::WidgetType:
      typeName = "widget";
      break;

    case Node::PseudoStateType:
      typeName = "pseudo state";
      break;

    case Node::SubControlType:
      typeName = "sub control";
      break;

    case Node::PropertyType:
      typeName = "property";
      break;
  }

  QAction* act = new QAction(
    q_ptr->tr("%1 is not a valid %2 name").arg(nNode->name()).arg(typeName));
  contextMenu->insertAction(first, act);
  auto reversed = reverseLastNValues(matches);
  contextMenu->insertSeparator(first);

  for (auto key : reversed) {
    act = new QAction(matches.value(key));
    contextMenu->insertAction(first, act);
    QVariant v;
    v.setValue(nNode);
    act->setData(v);
    q_ptr->connect(
      act, &QAction::triggered, q_ptr, &StylesheetEdit::suggestion);
  }

  contextMenu->insertSeparator(first);
}

void
StylesheetEditPrivate::updatePropertyContextMenu(QMenu* contextMenu,
                                                 QMap<int, QString> matches,
                                                 NamedNode* nNode)
{
  auto first = contextMenu->actions().first();
  QAction* act = new QAction(
    q_ptr->tr("%1 is not a valid property value").arg(nNode->name()));
  contextMenu->insertAction(first, act);
  auto reversed = reverseLastNValues(matches);
  contextMenu->insertSeparator(first);

  for (auto key : reversed) {
    act = new QAction(matches.value(key));
    contextMenu->insertAction(first, act);
    QVariant v;
    v.setValue(nNode);
    act->setData(v);
    q_ptr->connect(
      act, &QAction::triggered, q_ptr, &StylesheetEdit::suggestion);
  }

  contextMenu->insertSeparator(first);
}

void
StylesheetEditPrivate::handleCursorPositionChanged(QTextCursor textCursor)
{
  if (m_manualMove) {
    return;
  }

  setCurrentCursor(textCursor);

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

    } else {
      NamedNode* nNode = qobject_cast<NamedNode*>(node);

      if (nNode) {
        switch (nNode->type()) {

          case Node::WidgetType: {
            auto widget = qobject_cast<WidgetNode*>(nNode);

            if (!widget->isWidgetValid()) {
              // not a valid node
              if (widget != m_currentWidget) {
                auto matches = m_datastore->fuzzySearchWidgets(nNode->name());
                auto contextMenu = createContextMenu();
                updateContextMenu(contextMenu, matches, nNode);
                m_contextMenu = contextMenu;
              }
            }

            break;
          } // end WidgetType

          case Node::PseudoStateType: {
            auto pseudostate = qobject_cast<PseudoStateNode*>(nNode);

            if (!pseudostate->isStateValid()) {
              // not a valid node
              if (pseudostate != m_currentWidget) {
                auto matches =
                  m_datastore->fuzzySearchPseudoStates(nNode->name());
                auto contextMenu = createContextMenu();
                updateContextMenu(contextMenu, matches, nNode);
                m_contextMenu = contextMenu;
              }
            }

            break;
          } // end PseudoStateType

          case Node::SubControlType: {
            auto subcontrol = qobject_cast<SubControlNode*>(nNode);

            if (!subcontrol->isStateValid()) {
              // not a valid node
              if (subcontrol != m_currentWidget) {
                auto matches =
                  m_datastore->fuzzySearchSubControl(nNode->name());
                auto contextMenu = createContextMenu();
                updateContextMenu(contextMenu, matches, nNode);
                m_contextMenu = contextMenu;
              }
            }

            break;
          } // end SubControlType

          case Node::PropertyType: {
            auto property = qobject_cast<PropertyNode*>(nNode);

            if (property != m_currentWidget) {
              auto offset = textCursor.anchor() - property->cursor().anchor();
              auto isProperty = property->isProperty(offset);

              if (isProperty.first) {
                auto matches =
                  m_datastore->fuzzySearchProperty(isProperty.second);
                auto contextMenu = createContextMenu();
                updateContextMenu(contextMenu, matches, nNode);
                m_contextMenu = contextMenu;

              } else {
                if (!isProperty.second.isEmpty()) {
                  if (property->isValidProperty()) {
                    // must have a valid property to check value types.
                    auto matches =
                      m_datastore->fuzzySearchPropertyValue(property->name(), isProperty.second);
                    auto contextMenu = createContextMenu();
                    updatePropertyContextMenu(contextMenu, matches, nNode);
                    m_contextMenu = contextMenu;
                  } else {
                    m_contextMenu = createContextMenu();
                  }
                }
              }
            }

            break;
          } // end

          case Node::BadNodeType: {
            //          auto badNode = qobject_cast<BadBlockNode*>(nNode);

            //          //      if (badNode) {
            //          //        ParserState::Errors errors =
            //          badNode->errors();

            //          //        if
            //          (errors.testFlag(ParserState::InvalidWidget)) {
            //          //          m_hoverWidget->show(pos, q_ptr->tr("This is
            //          not a valid
            //          //          Widget"));

            //          //        } else if
            //          (errors.testFlag(ParserState::InvalidSubControl)) {
            //            //          m_hoverWidget->show(pos, q_ptr->tr("This
            //            is not a valid
            //            //          Sub-Control"));

            //            //        } else if
            //            (errors.testFlag(ParserState::InvalidPseudoState)) {
            //              //          m_hoverWidget->show(pos, q_ptr->tr("This
            //              is not a valid
            //              //          Pseudo-State"));

            //              //        } else if
            //              (errors.testFlag(ParserState::AnomalousMarkerType))
            //              //        {
            //              //          m_hoverWidget->show(
            //              //            pos,
            //              //            q_ptr->tr(
            //              //              "This could be either a Pseudo-State
            //              marker or a
            //              //              Sub-Control marker."));

            //              //        } else if
            //              (errors.testFlag(ParserState::AnomalousType)) {
            //              //          m_hoverWidget->show(pos, q_ptr->tr("The
            //              type of this is
            //              //          anomalous."));

            //              //        } else {
            //              //          hideHover();
            //              //        }

            //              //      }

            //              break;
          } // end case Node::BadNodeType

          default:
            m_hoverWidget->hideHover();
        }
      }
    } // end end brace type
  }   // end if node

  if (modified) {
    m_highlighter->rehighlight();
  }
}

void
StylesheetEditPrivate::handleSuggestion(QAction* act)
{
  QVariant v = act->data();
  NamedNode* nNode = v.value<NamedNode*>();

  if (nNode) {
    nNode->setName(act->text());
    auto originalCursor = nNode->cursor().anchor();
    nNode->cursor().movePosition(
      QTextCursor::Right, QTextCursor::KeepAnchor, nNode->length());
    nNode->cursor().removeSelectedText();
    nNode->cursor().insertText(act->text());

    switch (nNode->type()) {
      case Node::WidgetType: {
        auto widget = qobject_cast<WidgetNode*>(nNode);

        if (widget) {
          widget->setWidgetValid(true);
          widget->setStart(originalCursor);
        }

        break;
      }

      case Node::PseudoStateType: {
        auto pseudostate = qobject_cast<PseudoStateNode*>(nNode);

        if (pseudostate) {
          pseudostate->setStateValid(true);
          pseudostate->setStart(originalCursor);
        }

        break;
      }

      case Node::SubControlType: {
        auto subcontrol = qobject_cast<PseudoStateNode*>(nNode);

        if (subcontrol) {
          subcontrol->setStateValid(true);
          subcontrol->setStart(originalCursor);
        }

        break;
      }

      case Node::PropertyType: {
        auto property = qobject_cast<PropertyNode*>(nNode);

        if (property) {
          property->setStart(originalCursor);
        }

        break;
      }
    }

    m_highlighter->rehighlight();
  }
}

void
StylesheetEdit::handleDocumentChanged(int pos, int charsRemoved, int charsAdded)
{
  d_ptr->onDocumentChanged(pos, charsRemoved, charsAdded);
}

void
StylesheetEdit::handleTextChanged()
{
  //  d_ptr->handleTextChanged();
}

Node*
StylesheetEditPrivate::nextNode(QTextCursor cursor)
{
  QList<QTextCursor> keys = m_nodes->keys();
  int index = keys.indexOf(cursor) + 1; // next index

  if (index < keys.size()) {
    QTextCursor cursor = keys.value(index);
    return m_nodes->value(cursor);
  }

  return nullptr;
}

Node*
StylesheetEditPrivate::previousNode(QTextCursor cursor)
{
  QList<QTextCursor> keys = m_nodes->keys();
  int index = keys.indexOf(cursor) - 1; // previous index

  if (index >= 0) {
    QTextCursor cursor = keys.value(index);
    return m_nodes->value(cursor);
  }

  return nullptr;
}

void
StylesheetEditPrivate::updatePropertyValues(int pos,
                                            PropertyNode* property,
                                            int charsAdded,
                                            int charsRemoved,
                                            const QString& newValue)
{
  bool updated = false;
  auto values = property->values();
  auto offsets = property->offsets();
  auto checks = property->checks();
  auto attributes = property->attributeTypes();

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

      if (m_datastore->isValidPropertyValueForProperty(property->name(),
                                                       newValue)) {
        checks.replace(i, PropertyNode::GoodValue);

      } else {
        DataStore::AttributeType attribute = DataStore::NoAttributeValue;

        if ((attribute = m_datastore->propertyValueAttribute(newValue)) !=
            DataStore::NoAttributeValue) {
          attributes[i] = attribute;
        }
      }

      updated = true;
    }
  }

  property->setValues(values);
  property->setChecks(checks);
  property->setOffsets(offsets);
  property->setAttributeTypes(attributes);
}

void
StylesheetEditPrivate::onDocumentChanged(int pos,
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
            PropertyEndMarkerNode* marker =
              new PropertyEndMarkerNode(next->cursor(), q_ptr);
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
              //            QTextCursor propCursor(q_ptr->document());
              //            propCursor.setPosition(pos);
              //            PropertyMarkerNode* marker =
              //              new PropertyMarkerNode(propCursor, q_ptr);
              //            m_nodes->insert(propCursor, marker);
              property->setPropertyMarkerExists(true);
              property->setPropertyMarkerOffset(pos);
            }
          }

        } else {
          updatePropertyValues(
            cursor.anchor(), property, charsAdded, charsRemoved, newValue);
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
                Node* subcontrolmarker =
                  new SubControlMarkerNode(next->cursor(), q_ptr);
                m_nodes->insert(next->cursor(), subcontrolmarker);
                next->deleteLater();

              } else if (next->type() == Node::BadPseudoStateMarkerType) {
                Node* pseudostatemarker =
                  new PseudoStateMarkerNode(next->cursor(), q_ptr);
                m_nodes->insert(next->cursor(), pseudostatemarker);
                next->deleteLater();
              }
            }

            badNode->deleteLater();

          } else if (m_datastore->containsSubControl(newValue)) {
            Node* subcontrol =
              new SubControlNode(newValue, badNode->cursor(), q_ptr);
            m_nodes->insert(badNode->cursor(), subcontrol);
            Node* prev = previousNode(badNode->cursor());

            if (prev) {
              if (prev->type() == Node::BadSubControlMarkerType) {
                Node* subcontrolmarker =
                  new SubControlMarkerNode(prev->cursor(), q_ptr);
                m_nodes->insert(prev->cursor(), subcontrolmarker);
                prev->deleteLater();
              }
            }

            badNode->deleteLater();

          } else if (m_datastore->containsPseudoState(newValue)) {
            Node* pseudostate =
              new PseudoStateNode(newValue, badNode->cursor(), q_ptr);
            m_nodes->insert(badNode->cursor(), pseudostate);
            Node* prev = previousNode(badNode->cursor());

            if (prev) {
              if (prev->type() == Node::BadPseudoStateMarkerType) {
                Node* pseudostatemarker =
                  new PseudoStateMarkerNode(prev->cursor(), q_ptr);
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

    q_ptr->document()->markContentsDirty(
      0, q_ptr->document()->toPlainText().length());
    m_highlighter->rehighlight();
    //          emit q_ptr->viewport()->update();
    //          q_ptr->repaint();
  }
}

void
StylesheetEditPrivate::handleTextChanged()
{
  // TODO possibly remove this.
  //  //  m_highlighter->rehighlight();
  //  QTextCursor cursor = q_ptr->textCursor();
  //  qWarning();
  //  m_highlighter->rehighlight();
}

void
StylesheetEditPrivate::stashPropertyEndNode(int position, Node** endnode)
{
  QTextCursor cursor = getCursorForNode(position);
  *endnode = new PropertyEndNode(cursor, q_ptr);
  m_nodes->insert(cursor, *endnode);
}

void
StylesheetEditPrivate::stashPropertyEndMarkerNode(int position, Node** endnode)
{
  QTextCursor cursor = getCursorForNode(position);
  *endnode = new PropertyEndMarkerNode(cursor, q_ptr);
  m_nodes->insert(cursor, *endnode);
}

int
StylesheetEditPrivate::parsePropertyWithValues(QTextCursor cursor,
                                               PropertyNode* property,
                                               const QString& text,
                                               int start,
                                               int& pos,
                                               QString& block,
                                               Node** endnode)
{
  QString propertyName = property->name();

  while (!(block = findNext(text, pos)).isEmpty()) {
    if (block == ":") {
      if (!property->hasPropertyMarker()) {
        property->setPropertyMarkerExists(true);
        property->setPropertyMarkerOffset(pos);
      }

    } else if (block == "/*") { // comment start
      parseComment(text, pos);

    } else if (block == ";") {
      stashPropertyEndMarkerNode(property->end(), endnode);
      break;

    } else if (block == "}") {
      if (!(*endnode)) { // already a property end node (;)
        stashPropertyEndNode(property->end(), endnode);
      }

      stashEndBrace(cursor.anchor());

      pos--; // step back from endbrace
      break;

    } else {
      bool validForProperty =
        m_datastore->isValidPropertyValueForProperty(propertyName, block);
      DataStore::AttributeType attributeType =
        m_datastore->propertyValueAttribute(block);

      if (validForProperty) {
        // valid property and valid value.
        property->addValue(block,
                           PropertyNode::GoodValue,
                           (pos - block.length()) - start,
                           attributeType);

      } else {

        if (attributeType == DataStore::NoAttributeValue) {
          // not a valid value for any property
          if (m_datastore->containsProperty(block)) {
            Node* lastnode = m_nodes->last();

            if (!(lastnode->type() == Node::PropertyEndType ||
                  lastnode->type() == Node::PropertyEndMarkerType)) {
              // the block is actually another property. Probably a missing ';'.
              // set the last check value to missing property end.
              property->setBadCheck(PropertyNode::MissingPropertyEnd);
              stashPropertyEndNode(property->end(), endnode);
            }

            pos -= block.length(); // skip back before block.
            return property->end();

          } else {
            property->addValue(block,
                               PropertyNode::BadValue,
                               (pos - block.length()) - start,
                               attributeType);
          }

        } else {
          // invalid property name but this is a valid property attribute
          // anyway.
          property->addValue(block,
                             PropertyNode::ValidPropertyType,
                             (pos - block.length()) - start,
                             attributeType);
        }
      }
    }
  }

  return property->end();
}

void
StylesheetEditPrivate::parseComment(const QString& text, int& pos)
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
          comment->append(c);
        }
      }
    }
  }
}

void
StylesheetEditPrivate::stashBadNode(int position,
                                    const QString& block,
                                    ParserState::Error error)
{
  auto cursor = getCursorForNode(position);
  auto badblock = new BadBlockNode(block, cursor, error, q_ptr);
  m_nodes->insert(cursor, badblock);
}

void
StylesheetEditPrivate::stashBadSubControlMarkerNode(int position,
                                                    ParserState::Error error)
{
  auto cursor = getCursorForNode(position);
  auto badblock = new BadSubControlMarkerNode(cursor, error, q_ptr);
  m_nodes->insert(cursor, badblock);
}

void
StylesheetEditPrivate::stashBadPseudoStateMarkerNode(int position,
                                                     ParserState::Error error)
{
  auto cursor = getCursorForNode(position);
  auto badblock = new BadPseudoStateMarkerNode(cursor, error, q_ptr);
  m_nodes->insert(cursor, badblock);
}

void
StylesheetEditPrivate::stashPseudoState(int position,
                                        const QString& block,
                                        bool valid)
{
  auto cursor = getCursorForNode(position);
  auto pseudostate = new PseudoStateNode(block, cursor, q_ptr);
  pseudostate->setStateValid(valid);
  m_nodes->insert(cursor, pseudostate);
}

void
StylesheetEditPrivate::stashSubControl(int position,
                                       const QString& block,
                                       bool valid)
{
  auto cursor = getCursorForNode(position);
  auto subcontrol = new SubControlNode(block, cursor, q_ptr);
  subcontrol->setStateValid(valid);
  m_nodes->insert(cursor, subcontrol);
}

void
StylesheetEditPrivate::stashEndBrace(int position)
{
  m_braceCount--;
  auto cursor = getCursorForNode(position);
  auto brace = new EndBraceNode(cursor, q_ptr);
  m_nodes->insert(cursor, brace);
  m_endbraces.append(brace);
}

void
StylesheetEditPrivate::stashStartBrace(int position)
{
  m_braceCount++;
  auto cursor = getCursorForNode(position);
  auto brace = new StartBraceNode(cursor, q_ptr);
  m_nodes->insert(cursor, brace);
  m_startbraces.append(brace);
}

void
StylesheetEditPrivate::stashPseudoStateMarker(int position)
{
  auto cursor = getCursorForNode(position);
  auto marker = new PseudoStateMarkerNode(cursor, q_ptr);
  m_nodes->insert(cursor, marker);
}

void
StylesheetEditPrivate::stashSubControlMarker(int position)
{
  auto cursor = getCursorForNode(position);
  auto marker = new SubControlMarkerNode(cursor, q_ptr);
  m_nodes->insert(cursor, marker);
}

void
StylesheetEditPrivate::stashWidget(int position,
                                   const QString& block,
                                   bool valid)
{
  auto cursor = getCursorForNode(position);
  auto widgetnode = new WidgetNode(block, cursor, q_ptr);
  widgetnode->setWidgetValid(valid);
  m_nodes->insert(cursor, widgetnode);
}

void
StylesheetEditPrivate::parseInitialText(const QString& text, int pos)
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
                stashSubControl(pos - block.length(), block, false);
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
                stashPseudoState(pos - block.length(), block, false);
                break;
              }
            }

          } else {
            Node* endnode = nullptr;
            cursor = getCursorForNode(start);
            PropertyNode* property = new PropertyNode(block, cursor, q_ptr);
            m_nodes->insert(cursor, property);
            int end = parsePropertyWithValues(
              cursor, property, text, start, pos, block, &endnode);

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
          parsePropertyWithValues(
            cursor, property, text, start, pos, block, &endnode);

        } else {
          if (m_braceCount == 1) {
            int oldPos = pos;
            QString oldBlock = block;

            if (!(block = findNext(text, pos)).isEmpty()) {
              if (block == ":") {
                Node* endnode = nullptr;
                pos = oldPos;
                cursor = getCursorForNode(start);
                PropertyNode* property =
                  new PropertyNode(oldBlock, cursor, q_ptr);
                property->setPropertyMarkerExists(true);
                property->setPropertyMarkerOffset(pos);
                m_nodes->insert(cursor, property);
                int end = parsePropertyWithValues(
                  cursor, property, text, start, pos, block, &endnode);

                // run out of text.
                if (!endnode) {
                  QTextCursor cursor = getCursorForNode(end);
                  endnode = new PropertyEndNode(cursor, q_ptr);
                  m_nodes->insert(cursor, endnode);
                }
              }

            } else {
              stashBadNode(
                pos - block.length(), block, ParserState::InvalidPropertyName);
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
        stashBadSubControlMarkerNode(prevPos - 1,
                                     ParserState::InvalidSubControlMarker);
        pos = prevPos;
      }

    } else if (block == ":") { // comment start
      int prevPos = pos;

      if (!(block = findNext(text, pos)).isEmpty()) {
        if (m_datastore->containsPseudoState(block)) {
          stashPseudoStateMarker(prevPos);
          stashPseudoState(pos - block.length(), block);

        } else {
          stashBadPseudoStateMarkerNode(prevPos - 1,
                                        ParserState::InvalidPseudoStateMarker);
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
      int end = parsePropertyWithValues(
        cursor, property, text, start, pos, block, &endnode);

      // run out of text.
      if (!endnode) {
        cursor = getCursorForNode(end);
        endnode = new PropertyEndNode(cursor, q_ptr);
        m_nodes->insert(cursor, endnode);
      }

    } else {
      QString nextBlock;

      if (!m_nodes->isEmpty()) {
        int oldPos = pos;
        nextBlock = findNext(text, pos);

        if (nextBlock == ":") {
          nextBlock = findNext(text, pos);

          if (m_datastore->containsPseudoState(nextBlock)) {
            stashWidget(start, block, false);
            stashPseudoStateMarker(oldPos); // correct last node
            stashPseudoState(pos - nextBlock.length(), nextBlock);
            continue;

          } else if (m_datastore->propertyValueAttribute(nextBlock) !=
                     DataStore::NoAttributeValue) {
            Node* endnode = nullptr;
            cursor = getCursorForNode(start);
            PropertyNode* property = new PropertyNode(block, cursor, q_ptr);
            property->setPropertyMarkerExists(true);
            property->setValidProperty(false);
            m_nodes->insert(cursor, property);
            pos -= block.length(); // step back
            int end = parsePropertyWithValues(
              cursor, property, text, start, pos, block, &endnode);

            // run out of text.
            if (!endnode) {
              cursor = getCursorForNode(end);
              endnode = new PropertyEndNode(cursor, q_ptr);
              m_nodes->insert(cursor, endnode);
            }

            continue;
          }

        } else if (nextBlock == "::") {
          nextBlock = findNext(text, pos);

          if (m_datastore->containsSubControl(nextBlock)) {
            stashWidget(start, block, false);
            stashSubControlMarker(oldPos);
            stashSubControl(pos - nextBlock.length(), block);
            continue;
          }
        }

        // step back
        pos = oldPos;
        Node* lastnode = m_nodes->last();
        Node::Type lasttype = lastnode->type();

        if (lasttype == Node::BadSubControlMarkerType) {
          if (m_datastore->containsSubControl(block)) {
            stashSubControlMarker(lastnode->start());
            stashSubControl(start, block);

          } else {
            stashSubControlMarker(lastnode->start());
            stashSubControl(start, block, false);
          }

        } else {
          stashWidget(start, block, false);
        }

      } else { // anomalous type - see what comes next.
        int oldPos = pos;
        nextBlock = findNext(text, pos);

        if (nextBlock == ":") {
          nextBlock = findNext(text, pos);

          if (m_datastore->containsPseudoState(nextBlock)) {
            stashWidget(start, block, false);
            stashPseudoStateMarker(oldPos); // correct last node
            stashPseudoState(pos - nextBlock.length(), nextBlock);
            continue;

          } else if (m_datastore->propertyValueAttribute(nextBlock) !=
                     DataStore::NoAttributeValue) {
            Node* endnode = nullptr;
            cursor = getCursorForNode(start);
            PropertyNode* property = new PropertyNode(block, cursor, q_ptr);
            property->setPropertyMarkerExists(true);
            property->setValidProperty(false);
            m_nodes->insert(cursor, property);
            pos -= block.length(); // step back
            int end = parsePropertyWithValues(
              cursor, property, text, start, pos, block, &endnode);

            // run out of text.
            if (!endnode) {
              cursor = getCursorForNode(end);
              endnode = new PropertyEndNode(cursor, q_ptr);
              m_nodes->insert(cursor, endnode);
            }

            continue;
          }

        } else if (nextBlock == "::") {
          nextBlock = findNext(text, pos);

          if (m_datastore->containsSubControl(nextBlock)) {
            stashWidget(start, block, false);
            stashSubControlMarker(oldPos);
            stashSubControl(pos - nextBlock.length(), block);
            continue;
          }
        }

        stashBadNode(start, block, ParserState::AnomalousType);
      }
    }
  }
}

void
StylesheetEditPrivate::skipBlanks(const QString& text, int& pos)
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

QTextCursor
StylesheetEditPrivate::getCursorForNode(int position)
{
  QTextCursor c(q_ptr->document());
  c.movePosition(QTextCursor::Start);
  c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, position);
  return c;
}

QString
StylesheetEditPrivate::findNext(const QString& text, int& pos)
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

CursorData
StylesheetEditPrivate::getNodeAtCursor(QTextCursor cursor)
{
  CursorData data;
  data.cursor = cursor;

  nodeAtCursorPosition(&data, cursor.anchor());

  return data;
}

CursorData
StylesheetEditPrivate::getNodeAtCursor(int position)
{
  CursorData data;
  data.cursor = QTextCursor(q_ptr->document());

  nodeAtCursorPosition(&data, position);

  return data;
}

void
StylesheetEditPrivate::nodeAtCursorPosition(CursorData* data, int position)
{
  Node* previous;

  for (auto key : m_nodes->keys()) {
    Node* node = m_nodes->value(key);

    if (!node) {
      return;
    }

    auto start = node->start();
    auto end = node->end();

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
  return m_currentCursor;
}

void
StylesheetEditPrivate::setCurrentCursor(const QTextCursor& currentCursor)
{
  m_currentCursor = currentCursor;
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
  m_manualMove = true;
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Start);
  cursor.movePosition(
    QTextCursor::Down, QTextCursor::MoveAnchor, linenumber - 1);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_manualMove = false;
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
  m_manualMove = true;
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, n);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_manualMove = false;
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
  m_manualMove = true;
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, n);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_manualMove = false;
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
  m_manualMove = true;
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, n);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_manualMove = false;
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
  m_manualMove = true;
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, n);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_manualMove = false;
}

void
StylesheetEdit::start()
{
  d_ptr->start();
}

void
StylesheetEditPrivate::start()
{
  m_manualMove = true;
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Start);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_manualMove = false;
}

void
StylesheetEdit::end()
{
  d_ptr->end();
}

void
StylesheetEditPrivate::end()
{
  m_manualMove = true;
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::End);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_manualMove = false;
}

void
StylesheetEdit::startOfLine()
{
  d_ptr->startOfLine();
}

void
StylesheetEditPrivate::startOfLine()
{
  m_manualMove = true;
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::StartOfLine);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_manualMove = false;
}

void
StylesheetEdit::endOfLine()
{
  d_ptr->endOfLine();
}

void
StylesheetEditPrivate::endOfLine()
{
  m_manualMove = true;
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::EndOfLine);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_manualMove = false;
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
