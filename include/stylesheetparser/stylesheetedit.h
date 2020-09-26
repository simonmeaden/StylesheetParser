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

//#include <QTextEdit>
#include <QPlainTextEdit>
#include <QPainter>
#include <QToolTip>

#include "datastore.h"
#include "stylesheethighlighter.h"

namespace StylesheetEditor {

class StylesheetEdit;
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

class StylesheetEdit : public QPlainTextEdit
{

  struct Data
  {
    QTextCursor cursor;
    Node* node = nullptr;
    //    Node* nextNode = nullptr;
    Node* prevNode = nullptr;
    int start;
    //    int valueIndex = -1;
    //    int nextValueIndex = -1;
  };

public:
  explicit StylesheetEdit(QWidget* parent = nullptr);

  //  void setText(const QString& text);
  void setPlainText(const QString& text);

  QMap<int, Node *> *nodes();

  void showNewlineMarkers(bool show);
  //  void showLineNumbers(bool show);

  QString styleSheet() const;
  void setStyleSheet(const QString& stylesheet);

  //! Sets a new color/fontweight pair for the highlighter base format
  void setNormalFormat(QColor color, QColor back, QFont::Weight weight = QFont::Normal);
  //! Sets a new color/fontweight pair for the highlighter value format
  void setValueFormat(QColor color, QColor back, QFont::Weight weight = QFont::Normal);
  //! Sets a new color/fontweight pair for the highlighter widget name format
  void setWidgetFormat(QColor color, QColor back, QFont::Weight weight = QFont::Normal);
  //! Sets a new color/fontweight pair for the highlighter pseudo state format
  void setPseudoStateFormat(QColor color, QColor back, QFont::Weight weight = QFont::Normal);
  //! Sets a new color/fontweight pair for the highlighter pseudo state marker (:) format
  void setPseudoStateMarkerFormat(QColor color, QColor back, QFont::Weight weight = QFont::Normal);
  //! Sets a new color/fontweight pair for the highlighter sub control format
  void setSubControlFormat(QColor color, QColor back, QFont::Weight weight = QFont::Normal);
  //! Sets a new color/fontweight pair for the highlighter sub control marker (::) format
  void setSubControlMarkerFormat(QColor color, QColor back, QFont::Weight weight = QFont::Normal);
  void setPropertyFormat(QColor color, QColor back, QFont::Weight weight = QFont::Normal);
  void setPropertyMarkerFormat(QColor color, QColor back, QFont::Weight weight = QFont::Normal);
  void setLineNumberFormat(QColor foreground, QColor back, QColor background, QFont::Weight weight = QFont::Light);
  void setBadValueFormat(QColor color, QColor back, QFont::Weight weight = QFont::Light, bool underline = true,
                         QTextCharFormat::UnderlineStyle underlineStyle = QTextCharFormat::WaveUnderline,
                         QColor underlineColor = QColor("red"));
  void setStartBraceFormat(QColor color, QColor back, QFont::Weight weight);
  void setEndBraceFormat(QColor color, QColor back, QFont::Weight weight);
  void setBraceMatchFormat(QColor color, QColor back, QFont::Weight weight);

  void lineNumberAreaPaintEvent(QPaintEvent* event);
  int lineNumberAreaWidth();

protected:
  void resizeEvent(QResizeEvent* event) override;

private:
  LineNumberArea* m_lineNumberArea;
  DataStore* m_datastore;
  //  Parser* m_parser;
  StylesheetHighlighter* m_highlighter;
  //  QList<Node*>* m_nodes;
  QMap<int, Node*>* m_nodes;
  QList<StartBraceNode*> m_startbraces;
  QList<EndBraceNode*> m_endbraces;
  int m_braceCount;
  bool m_bracesMatched;
  PropertyNode* m_propertynode = nullptr;
  QString m_stylesheet;
  bool m_startComment;


  bool event(QEvent* event);

  void onCursorPositionChanged();
  void onDocumentChanged(int pos, int charsRemoved, int charsAdded);

  void parseInitialText(const QString& text, int pos = 0);
  int parseProperty(const QString& text, int start, int& pos, QString& block, Node** endnode);
  void parseComment(const QString &text, int &pos);
  // Skips blank characters (inc \n\t etc.) and returns the first non-blank character.
  void skipBlanks(const QString& text, int& pos);
  QTextCursor getCursorForNode(int position);
  QString findNext(const QString& text, int& pos);


  Data getNodeAtCursor(QTextCursor cursor);
  Data getNodeAtCursor(int position);
  void nodeAtCursorPosition(Data* data, int position);

  QString getValueAtCursor(int anchor, const QString& text);
  QString getOldNodeValue(Data* data);

  void updateLineNumberAreaWidth(int);
  void highlightCurrentLine();
  void updateLineNumberArea(const QRect& rect, int dy);

};


} // end of StylesheetParser

#endif // STYLESHEETEDIT_H
