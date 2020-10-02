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

#include <QLabel>
#include <QList>
#include <QResizeEvent>
#include <QTextCharFormat>
#include <QWidget>

#include "datastore.h"
#include "node.h"
#include "parserstate.h"
#include "stylesheethighlighter.h"

namespace StylesheetEditor {

class StylesheetEdit;

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

  class LineNumberArea : public QWidget
  {
  public:
    LineNumberArea(StylesheetEdit* editor);

    QSize sizeHint() const override;

    QColor fore() const;
    void setFore(const QColor& fore);

    QColor back() const;
    void setBack(const QColor& back);

    QFont::Weight weight() const;
    void setWeight(const QFont::Weight& weight);

  protected:
    void paintEvent(QPaintEvent* event) override;

  private:
    StylesheetEdit* m_codeEditor;
    QColor m_fore, m_back;
    QFont::Weight m_weight;
  };

  class HoverWidget : public QWidget
  {
  public:
    HoverWidget(QWidget* parent = nullptr);

    QSize size();
    int height() const;
    int width() const;

    QString text() const;
    void setText(const QString &text);

  protected:
    void paintEvent(QPaintEvent*) override;

  private:
    QString m_text;
    int m_height, m_width;
  };

  StylesheetEditPrivate(StylesheetEdit* parent);

  StylesheetEdit* q_ptr;
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

  void setPlainText(const QString& text);

  QMap<QTextCursor, Node *> *nodes();

  void showNewlineMarkers(bool show);

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

  void lineNumberAreaPaintEvent(QTextBlock block,
                                int blockNumber,
                                int top,
                                int bottom,
                                int height,
                                double blockHeight,
                                QRect rect);
  int lineNumberAreaWidth();

  void resizeEvent(QRect cr);
//  bool event(QEvent* event);
  void drawHoverWidget(QPoint pos, QString text);
  void hideHoverWidget();
  void handleMouseMove(QPoint pos);
  void displayError(BadBlockNode *badNode, QPoint pos);

  void onCursorPositionChanged(QTextCursor textCursor);
  void onDocumentChanged(int pos, int charsRemoved, int charsAdded);

  void parseInitialText(const QString& text, int pos = 0);
  int parseProperty(const QString& text,
                    int start,
                    int& pos,
                    QString& block,
                    Node** endnode);
  void parseComment(const QString& text, int& pos);
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

  void highlightCurrentLine();
  void updateLineNumberArea(const QRect& rect, int dy);

  bool checkStylesheetColors(StylesheetData* data,
                             QColor& color1,
                             QColor& color2,
                             QColor& color3);
};

} // end of StylesheetParser

#endif // STYLESHEETEDIT_P_H
