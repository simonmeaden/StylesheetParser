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
#include "sm_widgets/labelledlineedit.h"
#include "sm_widgets/labelledspinbox.h"
#include "stylesheethighlighter.h"

/// \cond DO_NOT_DOCUMENT

class BookmarkData;
class BookmarkArea;
class HoverWidget;
class Parser;
class StylesheetData;
class LineNumberArea;

namespace YAML {
class Emitter;
class Node;
}

class StylesheetEditor : public QPlainTextEdit
{
  Q_OBJECT
public:
  explicit StylesheetEditor(QWidget* parent = nullptr);

  StylesheetHighlighter* highlighter();
  void setup(BookmarkArea* bookmarkArea, LineNumberArea* linenumberArea);
  void saveConfig(const QString& filename = QString());
  void loadConfig(const QString& filename = QString());

  void setPlainText(const QString& text);

  // TODO
  void setShowNewlineMarkers(bool show);
  bool showNewlineMarkers();

  QString styleSheet() const;
  void setStyleSheet(const QString& stylesheet);
  bool checkStylesheetColors(StylesheetData* data,
                             QColor& color1,
                             QColor& color2,
                             QColor& color3);

  void setWidgetFormat(
    QBrush color,
    QBrush back,
    QFont weight,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setBadWidgetFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underlineColor = QBrush(Qt::red),
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

  void setPseudoStateFormat(
    QBrush color,
    QBrush back,
    QFont weight,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setBadPseudoStateFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setPseudoStateMarkerFormat(
    QBrush color,
    QBrush back,
    QFont weight,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setBadPseudoStateMarkerFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setSubControlFormat(
    QBrush color,
    QBrush back,
    QFont weight,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setBadSubControlFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setSubControlMarkerFormat(
    QBrush color,
    QBrush back,
    QFont weight,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setBadSubControlMarkerFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setValueFormat(
    QBrush color,
    QBrush back,
    QFont weight,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setBadValueFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setPropertyFormat(
    QBrush color,
    QBrush back,
    QFont weight,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setBadPropertyFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setPropertyMarkerFormat(
    QBrush color,
    QBrush back,
    QFont weight,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setPropertyEndMarkerFormat(
    QBrush color,
    QBrush back,
    QFont weight,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setLineNumberFormat(QBrush foreground, QBrush back, QFont weight);
  void setStartBraceFormat(
    QBrush color,
    QBrush back,
    QFont weight,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setBadStartBraceFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setEndBraceFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setBadEndBraceFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);

  void setBraceMatchFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);

  QMap<int, BookmarkData*>* bookmarks();
  void setBookmarks(QMap<int, BookmarkData*>* bookmarks);
  void insertBookmark(int bookmark = -1, const QString& text = QString());
  void toggleBookmark(int bookmark);
  void editBookmark(int bookmark = -1);
  void removeBookmark(int bookmark = -1);
  void clearBookmarks();
  bool hasBookmark(int linenumber);
  bool hasBookmarkText(int bookmark);
  QString bookmarkText(int bookmark);
  int bookmarkCount();
  void gotoBookmarkDialog();
  int bookmarkLineNumber() const;
  void setBookmarkLineNumber(int bookmarkLineNumber);

  void format();
  void options();

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
  int calculateColumn(QTextCursor textCursor);

  int maxSuggestionCount() const;
  void setMaxSuggestionCount(int maxSuggestionCount);

  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void contextMenuEvent(QContextMenuEvent* event);

  bool checkForEmpty(const QString& text);
  void setHoverBadPropertyMarker(QString& hover);
  void setHoverFuzzyWidgetName(QString& hover, const QString& name);
  void setHoverFuzzySubControl(QString& hover, const QString& name);
  void setHoverBadSubControl(QString& hover,
                             const QString& widget,
                             const QString& name);
  void setHoverFuzzyPseudoState(QString& hover, const QString& name);
  void setHoverBadPropertyEndMarker(QString& hover, int end);

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
  int lineCount();

  bool addCustomWidget(const QString& name, const QString& parent);
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

private:
  void setLineNumber(int lineNumber);
  void suggestionMade(bool);
  void bookmarkMenuRequested(QPoint pos);
  void linenumberMenuRequested(QPoint pos);
  QString getValueAtCursor(int anchor, const QString& text);
  BookmarkArea* m_bookmarkArea;
  LineNumberArea* m_lineNumberArea;
  DataStore* m_datastore;
  Parser* m_parser;
  StylesheetHighlighter* m_highlighter;
  PropertyNode* m_propertynode = nullptr;
  QString m_stylesheet;
  bool m_parseComplete;
  int m_bookmarkLineNumber;
  QString m_configDir;
  QString m_configFile;

  QMenu *m_contextMenu, *m_suggestionsMenu;
  NodeSection* m_oldSection;

  //  void initActions();
  //  void initMenus();
  void handleParseComplete();
  void handleRehighlight();
  void handleRehighlightBlock(const QTextBlock& block);
  void handleAddBookmark(bool);
  void handleRemoveBookmark(bool);
  void handleEditBookmark(bool);
  void handleGotoBookmark();
  void handleClearBookmarks(bool);
  void cursorPositionHasChanged();
  void documentChanged(int pos, int charsRemoved, int charsAdded);
  void handleTextChanged();
  void handleCustomMenuRequested(QPoint pos);

  //  QByteArray formatToByteArray(QTextCharFormat format);

  //  void updateLeftArea(const QRect& rect, int dy);
  void updateLineNumberArea();
  QTextCursor currentCursor() const;
  void setCurrentCursor(const QTextCursor& currentCursor);
  void setLineData(QTextCursor cursor);

  void writeFormat(YAML::Emitter* emitter, QTextCharFormat format);
  QBrush readBrush(YAML::Node* node);
  QColor readColor(YAML::Node* node) const;
  QFont readFont(YAML::Node* node);
  void readFormat(YAML::Node *subnode1, QTextCharFormat **format);

  //  QByteArray formatToByteArray(QTextCharFormat format);
  //  QByteArray byteArrayToFormat(QByteArray array);

  //  void setFormatValues(QSettings *settings, QTextCharFormat format, QString
  //  dataset);

  //  QTextCharFormat loadFormatValues(QSettings *settings,  QString dataset);

  static const QString STYLESHEETEDIT;
  static const QString GOOD;
  static const QString BAD;
  static const QString RED;
  static const QString GREEN;
  static const QString BLUE;
  static const QString ALPHA;
  static const QString FORE;
  static const QString BACK;
  static const QString FONT;
  static const QString COLOR;
  static const QString STYLE;
  static const QString WIDGET;


public:
  static const QChar m_arrow;

  friend class Parser;
};

/// \endcond DO_NOT_DOCUMENT

#endif // STYLESHEETEDIT_P_H
