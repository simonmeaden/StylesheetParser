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
#ifndef STYLESHEETEDIT_H
#define STYLESHEETEDIT_H

#include <QObject>
#include <QHeaderView>
#include <QInputDialog>
#include <QKeySequence>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextCursor>
#include <QToolTip>
#include <QTextStream>
#include <QSettings>

#include "common.h"

class LineNumberArea;
class StylesheetEditorPrivate;
class BookmarkArea;
class BookmarkData;
class StylesheetEditor;


class StylesheetEdit : public QWidget
{
  Q_OBJECT
public:
  explicit StylesheetEdit(QWidget* parent = nullptr);

  //! Reimplemented from QPlainText::setPlainText()
  void setPlainText(const QString& text);

  // TODO
  void setShowNewlineMarkers(bool show);
  bool showNewlineMarkers();

  //! Reimplemented from QPlainText::stylesheet()
  QString styleSheet() const;
  //! Reimplemented from QPlainText::setStylesheet()
  void setStyleSheet(const QString& stylesheet);

  //! Sets a new foreground/background/fontweight for the highlighter widget
  //! format
  void setWidgetFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  //! Sets a new foreground/background/fontweight for the highlighter widget
  //! format
  void setBadWidgetFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setSeperatorFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setIdSelectorFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setBadIdSelectorFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setIDSelectorMarkerFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setBadIDSelectorMarkerFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  //! Sets a new foreground/background/fontweight for the highlighter
  //! pseudo-state format
  void setPseudoStateFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setBadPseudoStateFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  //! Sets a new foreground/background/fontweight for the highlighter
  //! pseude-state marker (::) format
  void setPseudoStateMarkerFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setBadPseudoStateMarkerFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  //! Sets a new foreground/background/fontweight for the highlighter subcontrol
  //! format
  void setSubControlFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setBadSubControlFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  //! Sets a new foreground/background/fontweight for the highlighter subcontrol
  //! marker (:) format
  void setSubControlMarkerFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setBadSubControlMarkerFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  //! Sets a new foreground/background/fontweight for the highlighter value
  //! format
  void setValueFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setBadValueFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  //! Sets a new foreground/background/fontweight for the highlighter property
  //! name format
  void setPropertyFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setBadPropertyFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  //! Sets a new foreground/background/fontweight for the highlighter property
  //! marker (:) format
  void setPropertyMarkerFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setPropertyEndMarkerFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  //! Sets a new foreground/background/fontweight for the highlighter line
  //! numbers format
  void setLineNumberFormat(QBrush color, QBrush back, QFont font);
  //! Sets a new foreground/background/fontweight for the highlighter start
  //! curly brace ({) format
  void setStartBraceFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setBadStartBraceFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  //! Sets a new foreground/background/fontweight for the highlighter end curly
  //! brace (}) format
  void setEndBraceFormat(QBrush color, QBrush back, QFont font,
                         QBrush underline = QBrush(),
                         QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setBadEndBraceFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  //! Sets a new foreground/background/fontweight for the highlighter brace
  //! match format
  void setBraceMatchFormat(QBrush color, QBrush back, QFont font,
                           QBrush underline = QBrush(),
                           QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);

  //! Returns the bookmarks with associated text, if any.
  //!
  //! The bookmarks are stored in the key list of a map.
  QMap<int, BookmarkData*>* bookmarks();
  //! Sets the bookmarks with associated text, if any.
  //! \note
  //! Bookmarks will only be added for line numbers that actually exist.
  //!
  void setBookmarks(QMap<int, BookmarkData*>* bookmarks);
  //! Inserts a new bookmark, with an optional text component. The current text
  //! is stored until the text is replaced again.
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
  //! Returns true if the supplied line number has an existing bookmark,
  //! otherwise false.
  bool hasBookmark(int linenumber);
  //! Returns true if the supplied line number has any text associated with it,
  //! otherwise false.
  //!
  //! Note that it also returns false if the bookmark does not exist.
  bool hasBookmarkText(int bookmark);
  //! Returns the text for the supplied bookmark, or an empty string if no text
  //! exists.
  QString bookmarkText(int bookmark);
  //! Returns the number of bookmarks stored.
  int bookmarkCount();
  //! Moves the current line number to the supplied number.
  void gotoBookmark(int bookmark);
  void gotoBookmarkDialog();

  //! Pretty print the text.
  void format();

  //! Displays the editor options dialog.
  void options();

  //! Returns the current line number
  int currentLineNumber() const;
  //! Moves n lines up, defaults to 1 line.
  void up(int n = 1);
  //! Moves n lines down defults to 1 line.
  void down(int n = 1);
  //! Moves n characters left, defaults to 1 line.
  void left(int n = 1);
  //! Moves n characters right, defaults to 1 character.
  void right(int n = 1);
  //! Moves to the start of the document.
  void start();
  //! Moves to the end of the document.
  void end();
  //! Moves to the start of the current line.
  void startOfLine();
  //! Moves to the start of the current line, or the start
  //! of the document if this is reached first.
  void endOfLine();
  //! Moves to the start of the suggested line number, or the end
  //! of the document if this is reached first.
  void goToLine(int lineNumber);

  int maxSuggestionCount() const;
  void setMaxSuggestionCount(int maxSuggestionCount);

  //! add a custom widget to the widget tree.
  //!
  //! This will allow a user to add any of their own widgets to
  //! the list of supported widget names.
  bool addCustomWidget(const QString& name, const QString& parent);

  //! Returns a list of the widget names that are supported.
  //!
  //! This will include all the standard QWidget subclasses plus any added
  //! widgets.
  QStringList widgets();

  bool addCustomWidgetPseudoStates(const QString& name,
                                   const QStringList& states);
  bool addCustomWidgetSubControls(const QString& name,
                                  const QStringList& controls);
  bool addCustomWidgetProperties(const QString& name,
                                 const QStringList& properties);
  bool addCustomWidgetPropertyValue(const QString& widget,
                                    const QString& property,
                                    const QString& value);
  bool addCustomWidgetPropertyValues(const QString& widget,
                                     const QString& property,
                                     QStringList values);

signals:
  void lineNumber(int);
  void lineCount(int);
  void column(int);
  void handleMouseClicked(const QPoint&);
  void handleCursorPositionChanged(QTextCursor);
  void handleSuggestion(QAction*);
  void handleDocumentChanged(int, int, int);

protected:
  StylesheetEditor* editor() { return m_editor; }

private:
  StylesheetEditor* m_editor;
  LineNumberArea* m_linenumberArea;
  BookmarkArea* m_bookmarkArea;

  friend class StylesheetEditDialog;
};

#endif // STYLESHEETEDIT_H
