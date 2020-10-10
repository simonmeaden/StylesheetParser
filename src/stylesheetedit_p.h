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

namespace StylesheetEditor {

class StylesheetEdit;

class BookmarkEditDialog : public QDialog
{
  Q_OBJECT
public:
  explicit BookmarkEditDialog(QWidget* parent = nullptr);

  void setText(const QString& text);
  QString text();
  void setLineNumber(int linenumber);
  int lineNumber();

private:
  LabelledSpinBox* m_linenumberEdit;
  LabelledLineEdit* m_textEdit;
  QDialogButtonBox* m_buttonBox;
};

class BookmarkModel : public QAbstractTableModel
{
public:
  explicit BookmarkModel(QMap<int, QString> bookmarks);
  int columnCount(const QModelIndex& = QModelIndex()) const;
  int rowCount(const QModelIndex& = QModelIndex()) const;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
//  bool setBookmark(int bookmark);

private:
  QMap<int, QString> m_bookmarks;
};

class GoToBookmarkDialog : public QDialog
{
  Q_OBJECT
public:
  explicit GoToBookmarkDialog(QMap<int, QString> bookmarks,
                              QWidget* parent = nullptr);

  int bookmark();
  QString text();
//  bool setBookmark(int bookmark);

private:
  QGroupBox* m_group;
  QTableView* m_bookmarkView;
  QDialogButtonBox* m_buttonBox;
  int m_bookmark;
  QString m_text;

  void handleClicked(const QModelIndex& index);
};

struct StylesheetData
{
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

struct StylesheetEditPrivate
{
  Q_DECLARE_PUBLIC(StylesheetEdit)

  class BookmarkArea : public QWidget
  {
  public:
    BookmarkArea(StylesheetEdit* parent = nullptr);

    QSize sizeHint() const override;

    QColor foreSelected() const;
    void setForeSelected(const QColor& fore);
    QColor foreUnselected() const;
    void setForeUnselected(const QColor& fore);
    QColor back() const;
    void setBack(const QColor& back);

    int bookmarkAreaWidth() const;
    void setWidth(int width);

    int left() const;
    void setLeft(int left);

    QMap<int, QString> bookmarks();
    void setBookmarks(QMap<int, QString> bookmarks);
    void insertBookmark(int bookmark, const QString& text = QString());
    void toggleBookmark(int bookmark);
    void removeBookmark(int bookmark);
    void editBookmark(int bookmark);
    void clearBookmarks();
    bool hasBookmark(int bookmark);
    bool hasBookmarkText(int bookmark);
    QString bookmarkText(int bookmark);
    int count();

  protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event);
    void contextMenuEvent(QContextMenuEvent* event);

  private:
    StylesheetEdit* m_codeEditor;
    QColor m_foreSelected, m_foreUnselected, m_back;
    QFont::Weight m_weight;
    int m_width, m_left;
    QMap<int, QString> m_bookmarks;
    QMap<int, QString> m_oldBookmarks;
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
    int lineNumber() const;
    int lineNumberAreaWidth();

    QFont::Weight weight() const;
    void setWeight(const QFont::Weight& weight);

    int left() const;
    void setLeft(int left);

    int lineCount() const;

  protected:
    void paintEvent(QPaintEvent* event) override;

  private:
    StylesheetEdit* m_codeEditor;
    QColor m_foreSelected, m_foreUnselected, m_back;
    QFont::Weight m_weight;
    int m_currentLineNumber, m_left, m_lineCount;
  };

  class HoverWidget : public QWidget
  {
  public:
    HoverWidget(QWidget* parent = nullptr);

    QSize size();
    int height() const;
    int width() const;

    QString text() const;
    void setText(const QString& text);

  protected:
    void paintEvent(QPaintEvent*) override;

  private:
    QString m_text;
    int m_height, m_width;
  };

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
  bool m_manualMove;

  void setPlainText(const QString& text);

  QMap<QTextCursor, Node*>* nodes();

  void showNewlineMarkers(bool show);

  int lineNumber() const;
  void setLineNumber(int lineNumber);
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
  QMap<int, QString> bookmarks();
  void setBookmarks(QMap<int, QString> bookmarks);
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
  void setBadValueFormat(QColor color,
                         QColor back,
                         QFont::Weight weight = QFont::Light,
                         bool underline = true,
                         QTextCharFormat::UnderlineStyle underlineStyle =
                           QTextCharFormat::WaveUnderline,
                         QColor underlineColor = QColor("red"));
  void setStartBraceFormat(QColor color, QColor back, QFont::Weight weight);
  void setEndBraceFormat(QColor color, QColor back, QFont::Weight weight);
  void setBraceMatchFormat(QColor color, QColor back, QFont::Weight weight);

  int bookmarkAreaWidth();
  int lineNumberAreaWidth();
  int calculateLineNumber(QTextCursor textCursor);
  void updateLeftArea(const QRect& rect, int dy);

  void resizeEvent(QRect cr);
  //  bool event(QEvent* event);
  void drawHoverWidget(QPoint pos, QString text);
  void hideHoverWidget();
  void handleMouseMove(QPoint pos);
  void displayError(BadBlockNode* badNode, QPoint pos);
  void displayError(PropertyNode* property, QPoint pos);

  void handleCursorPositionChanged(QTextCursor textCursor);
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
  void stashWidget(int position, const QString& block);
  void stashBadNode(int position,
                    const QString& block,
                    ParserState::Error error);
  void stashBadSubControlMarkerNode(int position, ParserState::Error error);
  void stashBadPseudoStateMarkerNode(int position, ParserState::Error error);
  void stashPseudoState(int position, const QString& block);
  void stashSubControl(int position, const QString& block);
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
  int calculateWidth(QString name, int offset, QFontMetrics fm);

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

private:
};

} // end of StylesheetParser

#endif // STYLESHEETEDIT_P_H
