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
#include <QTableView>
#include <QResizeEvent>
#include <QTextCharFormat>
#include <QWidget>
#include <QThread>

#include "datastore.h"
#include "common.h"
#include "node.h"
#include "parserstate.h"
#include "stylesheethighlighter.h"
#include "stylesheetparser/labelledlineedit.h"
#include "stylesheetparser/labelledspinbox.h"

/// \cond DO_NOT_DOCUMENT

class StylesheetEdit;
class BookmarkData;
class BookmarkArea;
class HoverWidget;
class Parser;
class StylesheetData;



class LineNumberArea : public QWidget
{
public:
  LineNumberArea(StylesheetEdit* editor = nullptr);

  QSize sizeHint() const override;

  QColor foreSelected() const;
  void setForeSelected(const QColor& fore);
  QColor foreUnselected() const;
  void setForeUnselected(const QColor& fore);
  QColor back() const;
  void setBack(const QColor& back);

  void setLineNumber(int lineNumber);
  int currentLineNumber() const;
  int lineNumberAreaWidth();

  QFont::Weight weight() const;
  void setWeight(const QFont::Weight& weight);

  int left() const;
  void setLeft(int left);

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  StylesheetEdit* m_codeEditor;
  QColor m_foreSelected, m_foreUnselected, m_back;
  QFont::Weight m_weight;
  int m_currentLineNumber, m_left;
};


struct StylesheetEditPrivate
{
  Q_DECLARE_PUBLIC(StylesheetEdit)

  StylesheetEditPrivate(StylesheetEdit* parent);

  StylesheetEdit* q_ptr;
  BookmarkArea* m_bookmarkArea;
  LineNumberArea* m_lineNumberArea;
    Parser* m_parser;
  StylesheetHighlighter* m_highlighter;
  PropertyNode* m_propertynode = nullptr;
  QString m_stylesheet;
  HoverWidget* m_hoverWidget;
  Node* m_currentHover;
  int m_lineCount;

  QAction* m_addBookmarkAct, *m_removeBookmarkAct, *m_editBookmarkAct, *m_clearBookmarksAct, *m_gotoBookmarkAct;
  QMenu* m_contextMenu, *m_bookmarkMenu, *m_suggestionsMenu;
  void initActions();
  void initMenus();
  void createBookmarkMenu();

  void setPlainText(const QString& text);
  void handleParseComplete();

  QMap<QTextCursor, Node *> nodes();

  void showNewlineMarkers(bool show);

  QTextCursor currentCursor() const;
  void setCurrentCursor(const QTextCursor& currentCursor);

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
  void gotoBookmark(int bookmark = -1);
  void handleRemoveBookmark(bool);
  void handleEditBookmark(bool);
  void handleGotoBookmark(bool);
  void handleContextMenuEvent(QPoint pos);
  void handleBookmarkMenuEvent(QPoint pos);

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
  void updateLeftArea(const QRect& rect, int dy);

  void resizeEvent(QRect cr);
  void handleMousePress(const QPoint &pos);
  void handleLeaveEvent();
  void displayBookmark(BookmarkData*data, QPoint pos);

  void handleCursorPositionChanged(QTextCursor textCursor);
  void handleSuggestion(QAction *act);

  void onDocumentChanged(int pos, int charsRemoved, int charsAdded);
  void handleTextChanged();

  CursorData getNodeAtCursor(QTextCursor cursor);
  CursorData getNodeAtCursor(int position);
  void nodeAtCursorPosition(CursorData* data, int position);

  QString getValueAtCursor(int anchor, const QString& text);
  QString getOldNodeValue(CursorData* data);

  bool checkStylesheetColors(StylesheetData* data,
                             QColor& color1,
                             QColor& color2,
                             QColor& color3);


public:
  int getLineCount() const;

  int maxSuggestionCount() const;
  void setMaxSuggestionCount(int maxSuggestionCount);

private:
  void setLineData(QTextCursor cursor);
  void createHover();
  void hoverWidgetType(NamedNode *nNode, QPair<NodeSectionType, int> isin, QPoint pos);
  void hoverPseudoStateType(NamedNode *nNode, QPair<NodeSectionType, int> isin, QPoint pos);
  void hoverSubControlType(NamedNode *nNode, QPair<NodeSectionType, int> isin, QPoint pos);
  void hoverPropertyType(NamedNode *nNode, QPair<NodeSectionType, int> isin, QPoint pos);
  QList<int> reverseLastNValues(QMap<int, QString> matches);
};


/// \endcond DO_NOT_DOCUMENT

#endif // STYLESHEETEDIT_P_H
