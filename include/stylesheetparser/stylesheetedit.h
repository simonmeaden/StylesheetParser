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
#ifndef STYLESHEETEDIT_H
#define STYLESHEETEDIT_H

#include <QPlainTextEdit>
#include <QPainter>
#include <QToolTip>
#include <QMenu>
#include <QKeySequence>
#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>
#include <QTextBlock>
#include <QTextCursor>

class LineNumberArea;
class StylesheetEditPrivate;
class BookmarkArea;
class BookmarkData;
class Node;

class StylesheetEdit : public QPlainTextEdit
{
  Q_OBJECT Q_DECLARE_PRIVATE(StylesheetEdit)
public:

  explicit StylesheetEdit(QWidget* parent = nullptr);

  //! Reimplemented from QPlainText::setPlainText()
  void setPlainText(const QString& text);

  // TODO
  void showNewlineMarkers(bool show);

  //! Reimplemented from QPlainText::stylesheet()
  QString styleSheet() const;
  //! Reimplemented from QPlainText::setStylesheet()
  void setStyleSheet(const QString& stylesheet);

  //! Sets a new foreground/background/fontweight for the highlighter value format
  void setValueFormat(QColor color, QColor back, QFont::Weight weight = QFont::Normal);
  //! Sets a new foreground/background/fontweight for the highlighter widget format
  void setWidgetFormat(QColor color, QColor back, QFont::Weight weight = QFont::Normal);
  //! Sets a new foreground/background/fontweight for the highlighter pseudo-state format
  void setPseudoStateFormat(QColor color, QColor back, QFont::Weight weight = QFont::Normal);
  //! Sets a new foreground/background/fontweight for the highlighter pseude-state marker (::) format
  void setPseudoStateMarkerFormat(QColor color, QColor back, QFont::Weight weight = QFont::Normal);
  //! Sets a new foreground/background/fontweight for the highlighter subcontrol format
  void setSubControlFormat(QColor color, QColor back, QFont::Weight weight = QFont::Normal);
  //! Sets a new foreground/background/fontweight for the highlighter subcontrol marker (:) format
  void setSubControlMarkerFormat(QColor color, QColor back, QFont::Weight weight = QFont::Normal);
  //! Sets a new foreground/background/fontweight for the highlighter property name format
  void setPropertyFormat(QColor color, QColor back, QFont::Weight weight = QFont::Normal);
  //! Sets a new foreground/background/fontweight for the highlighter property marker (:) format
  void setPropertyMarkerFormat(QColor color, QColor back, QFont::Weight weight = QFont::Normal);
  //! Sets a new foreground/background/fontweight for the highlighter line numbers format
  void setLineNumberFormat(QColor foreground, QColor back, QFont::Weight weight = QFont::Light);
//  //! Sets a new foreground/background/fontweight for the highlighter bad value format
//  void setBadValueFormat(QColor color, QColor back, QFont::Weight weight = QFont::Light, bool underline = true,
//                         QTextCharFormat::UnderlineStyle underlineStyle = QTextCharFormat::WaveUnderline,
//                         QColor underlineColor = QColor("red"));
  //! Sets a new foreground/background/fontweight for the highlighter start curly brace ({) format
  void setStartBraceFormat(QColor color, QColor back, QFont::Weight weight);
  //! Sets a new foreground/background/fontweight for the highlighter end curly brace (}) format
  void setEndBraceFormat(QColor color, QColor back, QFont::Weight weight);
  //! Sets a new foreground/background/fontweight for the highlighter brace match format
  void setBraceMatchFormat(QColor color, QColor back, QFont::Weight weight);

  //! Returns the list of nodes.
  QMap<QTextCursor, Node*>* nodes();

  //! Returns the bookmarks with associated text, if any.
  //!
  //! The bookmarks are stored in the key list of a map.
  QMap<int, BookmarkData *> *bookmarks();
  //! Sets the bookmarks with associated text, if any.
  void setBookmarks(QMap<int, BookmarkData *> *bookmarks);
  //! Inserts a new bookmark, with an optional text component. The current text is stored
  //! until the text is replaced again.
  void insertBookmark(int bookmark, const QString& text = QString());
  //! Toggles the bookmark.
  //!
  //! Note that if you remove it the last set of text will be stored.
  void toggleBookmark(int bookmark);
  //! Edit the bookmark text, and optionally the line number.
  void editBookmark(int bookmark = -1);
  //! Remove the bookmark.
  void removeBookmark(int bookmark);
  //! Remove ALL bookmarks.
  void clearBookmarks();
  //! Returns true if the supplied line number has an existing bookmark, otherwise false.
  bool hasBookmark(int linenumber);
  //! Returns true if the supplied line number has any text associated with it, otherwise false.
  //!
  //! Note that it also returns false if the bookmark does not exist.
  bool hasBookmarkText(int bookmark);
  //! Returns the text for the supplied bookmark, or an empty string if no text exists.
  QString bookmarkText(int bookmark);
  //! Returns the number of bookmarks stored.
  int bookmarkCount();
  //! Moves the current line number to the supplied number.
  void gotoBookmark(int bookmark);
  void gotoBookmarkDialog(bool);

  //! Pretty print the text.
  void format();

  //! Returns the current line number
  int currentLineNumber() const;
  //! Moves 1 line up
  //!
  //! This is a convenience function to catch signals that have a boolean
  //! parameter which defaults to 0.
  //!
  //! To use it it the modern form you will probably need to use qOverload<bool>
  //! or QOverload<bool> in order to access it properly.
  //! \code
  //!   QAction* act = new QAction(upIcon, tr("&Up"), this);
  //!   connect(act, &QAction::triggered, m_editor, qOverload<bool>(&StylesheetEdit::up));
  //! \endcode

  void up(bool);
  //! Moves n lines up
  void up(int n = 1);
  //! Moves 1 line down
  //!
  //! This is a convenience function to catch signals that have a boolean
  //! parameter which defaults to 0.
  //!
  //! To use it it the modern form you will probably need to use qOverload<bool>
  //! or QOverload<bool> in order to access it properly.
  //! \code
  //!   QAction* act = new QAction(upIcon, tr("&Down"), this);
  //!   connect(act, &QAction::triggered, m_editor, qOverload<bool>(&StylesheetEdit::down));
  //! \endcode
  void down(bool);
  //! Moves n lines down
  void down(int n = 1);
  //! Moves 1 line left
  //!
  //! This is a convenience function to catch signals that have a boolean
  //! parameter which defaults to 0.
  //!
  //! To use it it the modern form you will probably need to use qOverload<bool>
  //! or QOverload<bool> in order to access it properly.
  //! \code
  //!   QAction* act = new QAction(icon, tr("&Left"), this);
  //!   connect(act, &QAction::triggered, m_editor, qOverload<bool>(&StylesheetEdit::left));
  //! \endcode
  void left(bool);
  //! Moves n characters left.
  void left(int n = 1);
  //! Moves 1 line right
  //!
  //! This is a convenience function to catch signals that have a boolean
  //! parameter which defaults to 0.
  //!
  //! To use it it the modern form you will probably need to use qOverload<bool>
  //! or QOverload<bool> in order to access it properly.
  //! \code
  //!   QAction* act = new QAction(icon, tr("&Right"), this);
  //!   connect(act, &QAction::triggered, m_editor, qOverload<bool>(&StylesheetEdit::right));
  //! \endcode
  void right(bool);
  //! Moves n characters right.
  void right(int n = 1);
  //! Moves to the start of the document.
  void start();
  //! Moves to the end of the document.
  void end();
  //! Moves to the start of the current line.
  void startOfLine();
  //! Moves to the start of the current line.
  void endOfLine();
  //! Moves to the start of the suggested line number.
  void goToLine(int lineNumber);

  int lineNumberAreaWidth();
  int bookmarkAreaWidth();
  int calculateLineNumber(QTextCursor textCursor);
  void contextBookmarkMenuEvent(QContextMenuEvent* event);
//  void drawHoverWidget(QPoint pos, QString text);

  int maxSuggestionCount() const;
  void setMaxSuggestionCount(int maxSuggestionCount);


  /// \cond DO_NOT_DOCUMENT
  // These should not be documented as they are only removing protected status.
  QTextBlock firstVisibleBlock() {
    return QPlainTextEdit::firstVisibleBlock();
  }
  QRectF blockBoundingGeometry(QTextBlock block) {
    return QPlainTextEdit::blockBoundingGeometry(block);
  }
  QRectF blockBoundingRect(QTextBlock block) {
    return QPlainTextEdit::blockBoundingRect(block);
  }
  QPointF contentOffset() {
    return QPlainTextEdit::contentOffset();
  }
  /// \endcond DO_NOT_DOCUMENT

signals:
  void lineNumber(int);
  void lineCount(int);
  void column(int);

protected:
  void contextMenuEvent(QContextMenuEvent* event);

  void resizeEvent(QResizeEvent* event) override;
  void mousePressEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event);
  void mouseDoubleClickEvent(QMouseEvent* event);
  void leaveEvent(QEvent *event) override;

  void setLineNumber(int lineNumber);
  void suggestion(bool);


private:
  StylesheetEditPrivate* d_ptr;
  Node* m_hoverNode;

  void initActions();
  void initMenus();
  void handleAddBookmark(bool);
  void handleRemoveBookmark(bool);
  void handleEditBookmark(bool);
  void handleGotoBookmark();
  void handleClearBookmarks(bool);
  void handleCursorPositionChanged();
  void handleDocumentChanged(int pos, int charsRemoved, int charsAdded);
  void handleTextChanged();

  void updateLeftArea(const QRect& rect, int dy);
  void updateLeftAreaWidth(int);

  
public:
  static const QChar m_arrow;

};


#endif // STYLESHEETEDIT_H
