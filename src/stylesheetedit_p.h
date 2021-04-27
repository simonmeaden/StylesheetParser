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
#include "stylesheetedit/labelledlineedit.h"
#include "stylesheetedit/labelledspinbox.h"
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
  explicit StylesheetEditor(DataStore* datastore, QWidget* parent = nullptr);

  StylesheetHighlighter* highlighter();
  void setup(BookmarkArea* bookmarkArea, LineNumberArea* linenumberArea);
  void saveYamlConfig(const QString& filename = QString());
  void loadYamlConfig(const QString& filename = QString());
  bool saveXmlConfig(const QString& filename = QString())
  {
    return m_datastore->saveXmlScheme(filename);
  }
  bool loadXmlConfig(const QString& filename = QString())
  {
    return m_datastore->loadXmlTheme(filename);
  }

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
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setWidgetFormat(QTextCharFormat format);
  void setBadWidgetFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setBadWidgetFormat(QTextCharFormat format);
  void setSeperatorFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setSeperatorFormat(QTextCharFormat format);
  void setIdSelectorFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setIdSelectorFormat(QTextCharFormat format);
  void setBadIdSelectorFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setBadIdSelectorFormat(QTextCharFormat format);
  void setIdSelectorMarkerFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setIdSelectorMarkerFormat(QTextCharFormat format);
  void setBadIdSelectorMarkerFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setBadIdSelectorMarkerFormat(QTextCharFormat format);
  void setPseudoStateFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setPseudoStateFormat(QTextCharFormat format);
  void setBadPseudoStateFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setBadPseudoStateFormat(QTextCharFormat format);
  void setPseudoStateMarkerFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setPseudoStateMarkerFormat(QTextCharFormat format);
  void setBadPseudoStateMarkerFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setBadPseudoStateMarkerFormat(QTextCharFormat format);
  void setSubControlFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setSubControlFormat(QTextCharFormat format);
  void setBadSubControlFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setBadSubControlFormat(QTextCharFormat format);
  void setSubControlMarkerFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setSubControlMarkerFormat(QTextCharFormat format);
  void setBadSubControlMarkerFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setBadSubControlMarkerFormat(QTextCharFormat format);
  void setPropertyValueFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setPropertyValueFormat(QTextCharFormat format);
  void setBadPropertyValueFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setBadPropertyValueFormat(QTextCharFormat format);
  void setPropertyFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setPropertyFormat(QTextCharFormat format);
  void setBadPropertyFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setBadPropertyFormat(QTextCharFormat format);
  void setPropertyMarkerFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setPropertyMarkerFormat(QTextCharFormat format);
  void setPropertyEndMarkerFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setPropertyEndMarkerFormat(QTextCharFormat format);
  void setStartBraceFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setStartBraceFormat(QTextCharFormat format);
  void setBadStartBraceFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setBadStartBraceFormat(QTextCharFormat format);
  void setEndBraceFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setEndBraceFormat(QTextCharFormat format);
  void setBadEndBraceFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setBadEndBraceFormat(QTextCharFormat format);
  void setBraceMatchFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setBraceMatchFormat(QTextCharFormat format);
  void setBadBraceMatchFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(Qt::red),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setBadBraceMatchFormat(QTextCharFormat format);
  void setCommentFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setCommentFormat(QTextCharFormat format);
  void setLineNumberFormat(QBrush color, QBrush back, QFont font);

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

  void handleRehighlight();
  void handleRehighlightBlock(const QTextBlock& block);

  DataStore* datastore();

signals:
  void lineNumber(int);
  void lineCount(int);
  void column(int);

protected:
private:
  BookmarkArea* m_bookmarkArea;
  LineNumberArea* m_lineNumberArea;
  DataStore* m_datastore;
  Parser* m_parser;
  StylesheetHighlighter* m_highlighter;
  PropertyNode* m_propertynode = nullptr;
  QString m_stylesheet;
  bool m_parseComplete;
  int m_bookmarkLineNumber;
  //  QString m_configDir;
  //  QString m_configFile;

  QMenu *m_contextMenu, *m_suggestionsMenu;
  NodeSection* m_oldSection;

  //  void setupConfiguration();

  void setLineNumber(int lineNumber);
  void suggestionMade(bool);
  void bookmarkMenuRequested(QPoint pos);
  void linenumberMenuRequested(QPoint pos);
  QString getValueAtCursor(int anchor, const QString& text);

  void handleParseComplete();
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
  //  QBrush readBrush(YAML::Node* node);
  //  QColor readColor(YAML::Node* node) const;
  //  QFont readFont(YAML::Node* node);
  void readFormat(YAML::Node* subnode1, QTextCharFormat** format);

  void errorMsg(const char* text);
  static const QString MARKER;
  static const QString ENDMARKER;
  static const QString GOOD;
  static const QString BAD;
  static const QString FORE;
  static const QString BACK;
  static const QString FONT;
  static const QString COLOR;
  static const QString STYLE;
  static const QString WIDGET;
  static const QString SEPERATOR;
  static const QString IDSELECTOR;
  static const QString PSEUDOSTATE;
  static const QString SUBCONTROL;
  static const QString PROPERTY;
  static const QString VALUE;
  static const QString BRACE;
  static const QString START;
  static const QString END;
  static const QString MATCH;
  static const QString COMMENT;

public:
  static const QChar m_arrow;

  friend class Parser;
};

/// \endcond DO_NOT_DOCUMENT

#endif // STYLESHEETEDIT_P_H
