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

#include "datastore.h"
#include "stylesheethighlighter.h"

namespace StylesheetEditor {
class LineNumberArea;
class StylesheetEditPrivate;

class StylesheetEdit : public QPlainTextEdit
{
  Q_OBJECT
public:

  explicit StylesheetEdit(QWidget* parent = nullptr);

   void setPlainText(const QString& text);

//  QMap<int, Node*>* nodes();

  void showNewlineMarkers(bool show);

  QString styleSheet() const;
  void setStyleSheet(const QString& stylesheet);
  StylesheetData *getStylesheetProperty(const QString& sheet, int& pos);

  //  //! Sets a new color/fontweight pair for the highlighter base format
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
  void setLineNumberFormat(QColor foreground, QColor back, QFont::Weight weight = QFont::Light);
  void setBadValueFormat(QColor color, QColor back, QFont::Weight weight = QFont::Light, bool underline = true,
                         QTextCharFormat::UnderlineStyle underlineStyle = QTextCharFormat::WaveUnderline,
                         QColor underlineColor = QColor("red"));
  void setStartBraceFormat(QColor color, QColor back, QFont::Weight weight);
  void setEndBraceFormat(QColor color, QColor back, QFont::Weight weight);
  void setBraceMatchFormat(QColor color, QColor back, QFont::Weight weight);

  void lineNumberAreaPaintEvent(QPaintEvent* event);
  int lineNumberAreaWidth();

  void onCursorPositionChanged();
  void onDocumentChanged(int pos, int charsRemoved, int charsAdded);

  void updateLineNumberAreaWidth(int);

  QMap<int, Node*>* nodes();


protected:
  void resizeEvent(QResizeEvent* event) override;
  bool event(QEvent* event);

private:
  StylesheetEditPrivate* d_ptr;

  void highlightCurrentLine();
  void updateLineNumberArea(const QRect& rect, int dy);

};


} // end of StylesheetParser

#endif // STYLESHEETEDIT_H
