/*
  Copyright 2020 Simon Meaden

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#ifndef STYLESHEETEDIT_P_H
#define STYLESHEETEDIT_P_H

#include <QAbstractTableModel>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QList>
#include <QPlainTextEdit>
#include <QResizeEvent>
#include <QTextCharFormat>
#include <QThread>
#include <QWidget>

#include "common.h"
#include "datastore.h"
#include "node.h"
#include "parserstate.h"
#include "stylesheethighlighter.h"
#include "stylesheetparser/labelledlineedit.h"
#include "stylesheetparser/labelledspinbox.h"

/// \cond DO_NOT_DOCUMENT

class StylesheetEditor;
class BookmarkData;
class BookmarkArea;
class HoverWidget;
class Parser;
class StylesheetData;
class LineNumberArea;

struct StylesheetEditorPrivate
{
  Q_DECLARE_PUBLIC(StylesheetEditor)

  StylesheetEditorPrivate(StylesheetEditor* parent);

  StylesheetEditor* q_ptr;
  BookmarkArea* m_bookmarkArea;
  LineNumberArea* m_lineNumberArea;
  DataStore* m_datastore;
  Parser* m_parser;
  StylesheetHighlighter* m_highlighter;
  PropertyNode* m_propertynode = nullptr;
  QString m_stylesheet;
//  HoverWidget* m_hoverWidget;
//  WidgetNode* m_currentHover;
  bool m_parseComplete;

  QMenu *m_contextMenu, *m_suggestionsMenu;

  void setup(BookmarkArea* bookmarkArea, LineNumberArea* linenumberArea);
  void initActions();
  void initMenus();
//  void createBookmarkMenu();
  void handleCustomMenuRequested(QPoint pos);

  void setPlainText(const QString& text);
  void handleRehighlight();
  void handleParseComplete();

  void format();

  //  QMap<QTextCursor, Node*> nodes();

  void setShowNewlineMarkers(bool show);
  bool showLineMarkers();

  QTextCursor currentCursor() const;
  void setCurrentCursor(const QTextCursor& currentCursor);

  void setCurrentLineNumber(const int number);
  int currentLineNumber() const;
  int currentLineCount() const;
  void setLineNumber(int linenumber);
  void up(int n = 1);
  void down(int n = 1);
  void left(int n = 1);
  void right(int n = 1);
  void start();
  void end();
  void startOfLine();
  void endOfLine();

  int m_bookmarkLineNumber;
  int bookmarkLineNumber() const;
  void setBookmarkLineNumber(int bookmarkLineNumber);
  QMap<int, BookmarkData*>* bookmarks();
  void setBookmarks(QMap<int, BookmarkData*>* bookmarks);
  void insertBookmark(int bookmark = -1, const QString& text = QString());
  void toggleBookmark(int bookmark);
  void removeBookmark(int bookmark = -1);
  void clearBookmarks();
  bool hasBookmark(int linenumber);
  bool hasBookmarkText(int bookmark);
  void editBookmark(int bookmark = -1);
  QString bookmarkText(int bookmark);
  int bookmarkCount();
//  void gotoBookmark(int bookmark = -1);
  void handleRemoveBookmark(bool);
  void handleEditBookmark(bool);
  void handleGotoBookmark(bool);
  void handleContextMenuEvent(QPoint pos);
//  void handleBookmarkMenuEvent(QPoint pos);

  QString styleSheet() const;
  void setStyleSheet(const QString& stylesheet);

  //  //! Sets a new color/fontweight pair for the highlighter base format
  //! Sets a new color/fontweight pair for the highlighter value format
  void setValueFormat(QColor color,
                      QColor back,
                      QFont::Weight weight = QFont::Normal);
  //! Sets a new color/fontweight pair for the highlighter widget name format
  void setWidgetFormat(QColor color,
                       QColor back,
                       QFont::Weight weight = QFont::Normal);
  //! Sets a new color/fontweight pair for the highlighter pseudo state format
  void setPseudoStateFormat(QColor color,
                            QColor back,
                            QFont::Weight weight = QFont::Normal);
  //! Sets a new color/fontweight pair for the highlighter pseudo state marker
  //! (:) format
  void setPseudoStateMarkerFormat(QColor color,
                                  QColor back,
                                  QFont::Weight weight = QFont::Normal);
  //! Sets a new color/fontweight pair for the highlighter sub control format
  void setSubControlFormat(QColor color,
                           QColor back,
                           QFont::Weight weight = QFont::Normal);
  //! Sets a new color/fontweight pair for the highlighter sub control marker
  //! (::) format
  void setSubControlMarkerFormat(QColor color,
                                 QColor back,
                                 QFont::Weight weight = QFont::Normal);
  void setPropertyFormat(QColor color,
                         QColor back,
                         QFont::Weight weight = QFont::Normal);
  void setPropertyMarkerFormat(QColor color,
                               QColor back,
                               QFont::Weight weight = QFont::Normal);
  void setPropertyEndMarkerFormat(QColor color,
                                  QColor back,
                                  QFont::Weight weight = QFont::Normal);
  void setLineNumberFormat(QColor foreground,
                           QColor back,
                           QFont::Weight weight = QFont::Light);
  void setStartBraceFormat(QColor color, QColor back, QFont::Weight weight);
  void setEndBraceFormat(QColor color, QColor back, QFont::Weight weight);
  void setBraceMatchFormat(QColor color, QColor back, QFont::Weight weight);

  int bookmarkAreaWidth();
  int lineNumberAreaWidth();
  int calculateLineNumber(QTextCursor textCursor);
  int calculateColumn(QTextCursor textCursor);
  void updateLineNumberArea(int linenumber);

//  void resizeEvent(QRect cr);
//  void handleMouseClicked(QMouseEvent* event);
//  void handleLeaveEvent();
//  void displayBookmark(BookmarkData* data, QPoint pos);

  void cursorPositionChanged(QTextCursor textCursor);
  void suggestion(QAction* act);

  void onDocumentChanged(int pos, int charsRemoved, int charsAdded);
  void handleTextChanged();

//  CursorData getNodeAtCursor(QTextCursor cursor);
//  CursorData getNodeAtCursor(int position);
  void nodeAtCursorPosition(CursorData* data, int position);

  QString getValueAtCursor(int anchor, const QString& text);
  //  QString getOldNodeValue(CursorData* data);

  bool checkStylesheetColors(StylesheetData* data,
                             QColor& color1,
                             QColor& color2,
                             QColor& color3);
  int getLineCount() const;

  int maxSuggestionCount() const;
  void setMaxSuggestionCount(int maxSuggestionCount);
  void setLineData(QTextCursor cursor);
//  void createHover();
  QList<int> reverseLastNValues(QMap<int, QString> matches);
};

class StylesheetEditor : public QPlainTextEdit
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(StylesheetEditor)
public:
  explicit StylesheetEditor(QWidget* parent = nullptr);

  void setup(BookmarkArea* bookmarkArea, LineNumberArea* linenumberArea);

  void setPlainText(const QString& text);

  // TODO
  void setShowNewlineMarkers(bool show);
  bool showNewlineMarkers();

  QString styleSheet() const;
  void setStyleSheet(const QString& stylesheet);

  void setValueFormat(QColor color,
                      QColor back,
                      QFont::Weight weight = QFont::Normal);
  void setWidgetFormat(QColor color,
                       QColor back,
                       QFont::Weight weight = QFont::Normal);
  void setPseudoStateFormat(QColor color,
                            QColor back,
                            QFont::Weight weight = QFont::Normal);
  void setPseudoStateMarkerFormat(QColor color,
                                  QColor back,
                                  QFont::Weight weight = QFont::Normal);
  void setSubControlFormat(QColor color,
                           QColor back,
                           QFont::Weight weight = QFont::Normal);
  void setSubControlMarkerFormat(QColor color,
                                 QColor back,
                                 QFont::Weight weight = QFont::Normal);
  void setPropertyFormat(QColor color,
                         QColor back,
                         QFont::Weight weight = QFont::Normal);
  void setPropertyMarkerFormat(QColor color,
                               QColor back,
                               QFont::Weight weight = QFont::Normal);
  void setLineNumberFormat(QColor foreground,
                           QColor back,
                           QFont::Weight weight = QFont::Light);
  void setStartBraceFormat(QColor color, QColor back, QFont::Weight weight);
  void setEndBraceFormat(QColor color, QColor back, QFont::Weight weight);
  void setBraceMatchFormat(QColor color, QColor back, QFont::Weight weight);

  QMap<int, BookmarkData*>* bookmarks();
  void setBookmarks(QMap<int, BookmarkData*>* bookmarks);
  void insertBookmark(int bookmark, const QString& text = QString());
  void toggleBookmark(int bookmark);
  void editBookmark(int bookmark = -1);
  void removeBookmark(int bookmark);
  void clearBookmarks();
  bool hasBookmark(int linenumber);
  bool hasBookmarkText(int bookmark);
  QString bookmarkText(int bookmark);
  int bookmarkCount();
//  void gotoBookmark(int bookmark);
  void gotoBookmarkDialog();

  void format();

  void up(int n = 1);
  void down(int n = 1);
  void left(int n = 1);
  void right(int n = 1);
  void start();
  void end();
  void startOfLine();
  void endOfLine();
  void goToLine(int lineNumber);

  int lineNumberAreaWidth();
  int bookmarkAreaWidth();
  int calculateLineNumber(QTextCursor textCursor);
//  void contextBookmarkMenuEvent(QContextMenuEvent* event);
  //  void drawHoverWidget(QPoint pos, QString text);

  int maxSuggestionCount() const;
  void setMaxSuggestionCount(int maxSuggestionCount);

  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;

//  void handleMouseClicked(QMouseEvent* event);
//  void handleMousePressEvent(QMouseEvent* event);
//  void handleMouseMoveEvent(QMouseEvent* event);
//  void handleMouseReleaseEvent(QMouseEvent* event);
//  void handleMouseDoubleClickEvent(QMouseEvent* event);

  // These should not be documented as they are only removing protected status.
  QTextBlock firstVisibleBlock() { return QPlainTextEdit::firstVisibleBlock(); }
  QRectF blockBoundingGeometry(QTextBlock block)
  {
    return QPlainTextEdit::blockBoundingGeometry(block);
  }
  QRectF blockBoundingRect(QTextBlock block)
  {
    return QPlainTextEdit::blockBoundingRect(block);
  }
  QPointF contentOffset() { return QPlainTextEdit::contentOffset(); }

  int currentLineNumber() const;
  int getLineCount();

signals:
  void lineNumber(int);
  void lineCount(int);
  void column(int);
  void parseInitialText(const QString&);
  void handleCursorPositionChanged(QTextCursor);
  void handleSuggestion(QAction*);
  void handleDocumentChanged(int, int, int);

protected:
  //  void contextMenuEvent(QContextMenuEvent* event);

//  void resizeEvent(QResizeEvent* event) override;
  //  void mousePressEvent(QMouseEvent* event) override;
  //  void mouseMoveEvent(QMouseEvent* event) override;
  //  void mouseReleaseEvent(QMouseEvent* event) override;
  //  void mouseDoubleClickEvent(QMouseEvent* event) override;
//  void leaveEvent(QEvent* event) override;

  void setLineNumber(int lineNumber);
  void suggestion(bool);
  //  void setContextMenu(QMenu* menu);
  void customMenuRequested(QPoint pos);
  void bookmarkMenuRequested(QPoint pos);
  void linenumberMenuRequested(QPoint pos);

private:
  StylesheetEditorPrivate* d_ptr;

  void initActions();
  void initMenus();
  void handleParseComplete();
  void handleRehighlight();
  void handleAddBookmark(bool);
  void handleRemoveBookmark(bool);
  void handleEditBookmark(bool);
  void handleGotoBookmark();
  void handleClearBookmarks(bool);
  void cursorPositionHasChanged();
  void documentChanged(int pos, int charsRemoved, int charsAdded);
  void handleTextChanged();

  void updateLeftArea(const QRect& rect, int dy);
  void updateLineNumberArea();

public:
  static const QChar m_arrow;

  friend class Parser;
};

/// \endcond DO_NOT_DOCUMENT

#endif // STYLESHEETEDIT_P_H
