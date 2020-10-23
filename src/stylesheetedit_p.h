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

#include "datastore.h"
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

class StylesheetData
{
public:
  QString name;
  QList<QColor> colors;
  QList<QFont::Weight> weights;
  QList<QTextCharFormat::UnderlineStyle> underline;
};

struct CursorData
{
  QTextCursor cursor;
  Node* node = nullptr;
  Node* prevNode = nullptr;
  //  int start;
};


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
  DataStore* m_datastore;
  //  Parser* m_parser;
  StylesheetHighlighter* m_highlighter;
  //  QList<Node*>* m_nodes;
  QMap<QTextCursor, Node*>* m_nodes;
  QList<StartBraceNode*> m_startbraces;
  QList<EndBraceNode*> m_endbraces;
  int m_braceCount;
  bool m_bracesMatched;
  PropertyNode* m_propertynode = nullptr;
  QString m_stylesheet;
  StylesheetData m_stylesheetData;
  bool m_startComment;
  HoverWidget* m_hoverWidget;
  Node* m_currentHover;
  NamedNode* m_currentWidget;
  bool m_manualMove;
  QTextCursor m_currentCursor;
  int m_lineCount;

  QAction* m_formatAct;
  QAction* m_addBookmarkAct, *m_removeBookmarkAct, *m_editBookmarkAct, *m_clearBookmarksAct, *m_gotoBookmarkAct;
  QMenu* m_contextMenu, *m_bookmarkMenu;
  void initActions();
  void initMenus();
  QMenu *createContextMenu();
  void createBookmarkMenu();


  // TODO possibly to steup?
  int m_maxSuggestionCount;

  void setPlainText(const QString& text);

  QMap<QTextCursor, Node*>* nodes();

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
  //  void goToLine(int lineNumber);

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
  StylesheetData* getStylesheetProperty(const QString& sheet, int& pos);

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
//  void setBadValueFormat(QColor color,
//                         QColor back,
//                         QFont::Weight weight = QFont::Light,
//                         bool underline = true,
//                         QTextCharFormat::UnderlineStyle underlineStyle =
//                           QTextCharFormat::WaveUnderline,
//                         QColor underlineColor = QColor("red"));
  void setStartBraceFormat(QColor color, QColor back, QFont::Weight weight);
  void setEndBraceFormat(QColor color, QColor back, QFont::Weight weight);
  void setBraceMatchFormat(QColor color, QColor back, QFont::Weight weight);

  int bookmarkAreaWidth();
  int lineNumberAreaWidth();
  int calculateLineNumber(QTextCursor textCursor);
  int calculateColumn(QTextCursor textCursor);
  void updateLeftArea(const QRect& rect, int dy);

  void resizeEvent(QRect cr);
  //  bool event(QEvent* event);
//  void drawHoverWidget(QPoint pos, QString text);
//  void hideHoverWidget();
  void handleMouseMove(QPoint pos);
  void handleLeaveEvent();
  void displayBookmark(BookmarkData*data, QPoint pos);
//  void displayError(QString text, QPoint pos);
//  void displayError(WidgetNode *widget, QPoint pos);
//  void displayError(BadBlockNode* badNode, QPoint pos);
//  void displayError(PropertyNode* property, QPoint pos);

  void handleCursorPositionChanged(QTextCursor textCursor);
  void handleSuggestion(QAction *act);

  void onDocumentChanged(int pos, int charsRemoved, int charsAdded);
  void handleTextChanged();
  Node* nextNode(QTextCursor cursor);
  Node* previousNode(QTextCursor cursor);

  void parseInitialText(const QString& text, int pos = 0);
  int parsePropertyWithValues(QTextCursor cursor,
                              PropertyNode* property,
                              const QString& text,
                              int start,
                              int& pos,
                              QString& block,
                              Node** endnode);
  void parseComment(const QString& text, int& pos);
  void stashWidget(int position, const QString& block, bool valid=true);
  void stashBadNode(int position,
                    const QString& block,
                    ParserState::Error error);
  void stashBadSubControlMarkerNode(int position, ParserState::Error error);
  void stashBadPseudoStateMarkerNode(int position, ParserState::Error error);
  void stashPseudoState(int position, const QString& block, bool valid=true);
  void stashSubControl(int position, const QString& block, bool valid=true);
  void stashEndBrace(int position);
  void stashStartBrace(int position);
  void stashPseudoStateMarker(int position);
  void stashSubControlMarker(int position);
  void stashPropertyEndNode(int position, Node** endnode);
  void stashPropertyEndMarkerNode(int position, Node** endnode);
  void updatePropertyValues(int pos,
                            PropertyNode* property,
                            int charsAdded,
                            int charsRemoved,
                            const QString& newValue);
//  int calculateWidth(QString name, int offset, QFontMetrics fm);

  // Skips blank characters (inc \n\t etc.) and returns the first non-blank
  // character.
  void skipBlanks(const QString& text, int& pos);
  QTextCursor getCursorForNode(int position);
  QString findNext(const QString& text, int& pos);

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
  QPair<NameNode::SectionType, int> nodeForPoint(const QPoint &pos, NamedNode **nNode);
  void hoverWidgetType(NamedNode *nNode, QPair<NameNode::SectionType, int> isin, QPoint pos);
  void hoverPseudoStateType(NamedNode *nNode, QPair<NameNode::SectionType, int> isin, QPoint pos);
  void hoverSubControlType(NamedNode *nNode, QPair<NameNode::SectionType, int> isin, QPoint pos);
  void hoverPropertyType(NamedNode *nNode, QPair<NameNode::SectionType, int> isin, QPoint pos);
  QList<int> reverseLastNValues(QMap<int, QString> matches);


  void updateContextMenu(QMenu *contextMenu, QMap<int, QString> matches, NamedNode *nNode);
  void updatePropertyContextMenu(QMenu *contextMenu, QMap<int, QString> matches, NamedNode *nNode);
};


/// \endcond DO_NOT_DOCUMENT

#endif // STYLESHEETEDIT_P_H
