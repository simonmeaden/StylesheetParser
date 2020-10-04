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

#include "datastore.h"
#include "stylesheethighlighter.h"

namespace StylesheetEditor {
class LineNumberArea;
class StylesheetEditPrivate;

class StylesheetEdit : public QPlainTextEdit
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(StylesheetEdit)
public:

  explicit StylesheetEdit(QWidget* parent = nullptr);

  //! Reimplemented from QPlainText::setPlainText()
  void setPlainText(const QString& text);

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
  //! Sets a new foreground/background/fontweight for the highlighter bad value format
  void setBadValueFormat(QColor color, QColor back, QFont::Weight weight = QFont::Light, bool underline = true,
                         QTextCharFormat::UnderlineStyle underlineStyle = QTextCharFormat::WaveUnderline,
                         QColor underlineColor = QColor("red"));
  //! Sets a new foreground/background/fontweight for the highlighter start curly brace ({) format
  void setStartBraceFormat(QColor color, QColor back, QFont::Weight weight);
  //! Sets a new foreground/background/fontweight for the highlighter end curly brace (}) format
  void setEndBraceFormat(QColor color, QColor back, QFont::Weight weight);
  //! Sets a new foreground/background/fontweight for the highlighter brace match format
  void setBraceMatchFormat(QColor color, QColor back, QFont::Weight weight);
  QMap<QTextCursor, Node *> *nodes();


  QList<int> bookmarks();
  void setBookmarks(QList<int> bookmarks);
  void addBookmark(int bookmark);
  void removeBookmark(int bookmark);
  void clearBookmarks();

protected:
  void resizeEvent(QResizeEvent* event) override;
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event);
  void mouseDoubleClickEvent(QMouseEvent *event);
  void contextMenuEvent(QContextMenuEvent *event);

private:
  StylesheetEditPrivate* d_ptr;
  QAction *m_formatAct;
  Node* m_hoverNode;

  void initActions();
  void format();
  void updateLeftArea(const QRect& rect, int dy);
//  void updateLineNumberArea(const QRect& rect, int dy);
  int bookmarkAreaWidth();
  int lineNumberAreaWidth();
  void onCursorPositionChanged();
  void onDocumentChanged(int pos, int charsRemoved, int charsAdded);
  void handleTextChanged();
//  void updateLineNumberAreaWidth(int);
  void updateLeftAreaWidth(int);

};


} // end of StylesheetParser

#endif // STYLESHEETEDIT_H
