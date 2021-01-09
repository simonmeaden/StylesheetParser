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
#ifndef STYLESHEETHIGHLIGHTER_H
#define STYLESHEETHIGHLIGHTER_H

#include <QList>
#include <QSyntaxHighlighter>

#include "node.h"

class StylesheetEditor;
class DataStore;

class StylesheetHighlighter : public QSyntaxHighlighter
{
public:
  explicit StylesheetHighlighter(StylesheetEditor* editor,
                                 DataStore* datastore);

  void highlightBlock(const QString& text);

  void setWidgetFormat(QBrush color, QBrush back, QFont::Weight weight);
  void setPseudoStateFormat(QBrush color, QBrush back, QFont::Weight weight);
  void setPseudoStateMarkerFormat(QBrush color,
                                  QBrush back,
                                  QFont::Weight weight);
  void setSubControlFormat(QBrush color, QBrush back, QFont::Weight weight);
  void setSubControlMarkerFormat(QBrush color,
                                 QBrush back,
                                 QFont::Weight weight);
  void setValueFormat(QBrush color, QBrush back, QFont::Weight weight);
  //  void setBadValueFormat(QBrush color, QBrush back, QFont::Weight weight,
  //  bool underline,
  //                         QTextCharFormat::UnderlineStyle underlineStyle,
  //                         QColor underlineColor);
  void setPropertyFormat(QBrush color, QBrush back, QFont::Weight weight);
  void setPropertyMarkerFormat(QBrush color, QBrush back, QFont::Weight weight);
  void setPropertyEndMarkerFormat(QBrush color,
                                  QBrush back,
                                  QFont::Weight weight);
  void setStartBraceFormat(QBrush color, QBrush back, QFont::Weight weight);
  void setEndBraceFormat(QBrush color, QBrush back, QFont::Weight weight);
  void setBraceMatchFormat(QBrush color, QBrush back, QFont::Weight weight);
  void setCommentFormat(QBrush color, QBrush back, QFont::Weight weight);

  QBrush widget() const;
  QBrush pseudostate() const;
  QBrush pseudostatemarker() const;
  QBrush subcontrol() const;
  QBrush subcontrolmarker() const;
  QBrush value() const;
  QBrush badvalue() const;
  QBrush property() const;
  QBrush propertymarker() const;
  QBrush startbrace() const;
  QBrush endbrace() const;
  QBrush bracematch() const;
  QBrush bracematchBack() const;
  QBrush comment() const;
  QBrush back() const;
  QBrush underlineColor() const;

  int widgetWeight() const;
  int pseudostateWeight() const;
  int pseudostatemarkerWeight() const;
  int subcontrolWeight() const;
  int subcontrolmarkerWeight() const;
  int valueWeight() const;
  int commentWeight() const;
  int bracematchWeight() const;
  int endbraceWeight() const;
  int startbraceWeight() const;
  int propertymarkerWeight() const;
  int propertyWeight() const;
  int badvalueWeight() const;
  bool badUnderline() const;

  QTextCharFormat::UnderlineStyle underlinestyle() const;

private:
  StylesheetEditor* m_editor;
  DataStore* m_datastore;
  QTextCharFormat m_baseFormat;
  QTextCharFormat m_widgetFormat;
  QTextCharFormat m_badWidgetFormat;
  QTextCharFormat m_nameFormat;
  QTextCharFormat m_valueFormat;
  QTextCharFormat m_badValueFormat;
  QTextCharFormat m_pseudoStateFormat;
  QTextCharFormat m_badPseudoStateFormat;
  QTextCharFormat m_pseudoStateMarkerFormat;
  QTextCharFormat m_subControlFormat;
  QTextCharFormat m_badSubControlFormat;
  QTextCharFormat m_subControlMarkerFormat;
  QTextCharFormat m_propertyFormat;
  QTextCharFormat m_badPropertyFormat;
  QTextCharFormat m_propertyMarkerFormat;
  QTextCharFormat m_propertyEndMarkerFormat;
  QTextCharFormat m_startBraceFormat;
  QTextCharFormat m_badStartBraceFormat;
  QTextCharFormat m_endBraceFormat;
  QTextCharFormat m_badEndBraceFormat;
  QTextCharFormat m_braceMatchFormat;
  QTextCharFormat m_badBraceMatchFormat;
  QTextCharFormat m_commentFormat;
  bool m_badUnderline = true;
  QBrush m_back;

  int setNodeEnd(int nodeEnd, int blockEnd);
  int setNodeStart(int nodeStart, int blockStart);
  //  void formatProperty(PropertyNode* property);
  //  NodeIsIn nodeInBlock(const QTextBlock &block, Node *node, int&
  //  blockEnd,int &nodeStart);
  //  int getPositionRelToBlock(Node* node,
  //                            QTextBlock block,
  //                            int offset = 0);
  bool isInBlock(int position, int length, int blockStart, int blockEnd);
  void formatVisiblePart(int blockStart,
                         int blockEnd,
                         int position,
                         int length,
                         QTextCharFormat format);
  //  int getLengthRelToBlock(Node* node,
  //                          QTextBlock block,
  //                          int position = -1,
  //                          int length = -1);
  void formatPosition(int position,
                      int length,
                      int blockEnd,
                      QTextCharFormat format);
  void formatProperty(PropertyNode* property,
                      int blockStart,
                      int blockEnd, bool finalBlock=false);
  bool checkForEmpty(const QString & text);
};

#endif // STYLESHEETHIGHLIGHTER_H
