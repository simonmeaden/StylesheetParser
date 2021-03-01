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

class StylesheetEditor;
class DataStore;
class PseudoState;
class SubControl;
class IDSelector;
class ControlBase;
class PropertyNode;

class StylesheetHighlighter : public QSyntaxHighlighter
{
public:
  explicit StylesheetHighlighter(StylesheetEditor* editor,
                                 DataStore* datastore);

  void highlightBlock(const QString& text);

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
  void setIDSelectorMarkerFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline);
  void setIDSelectorMarkerFormat(QTextCharFormat format);
  void setBadIDSelectorMarkerFormat(
    QBrush color,
    QBrush back,
    QFont font,
    QBrush underline = QBrush(),
    QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline);
  void setBadIDSelectorMarkerFormat(QTextCharFormat format);
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

  QTextCharFormat::UnderlineStyle underlinestyle() const;

  QTextCharFormat widgetFormat() const;
  QTextCharFormat badWidgetFormat() const;
  QTextCharFormat seperatorFormat() const;
  QTextCharFormat idSelectorFormat() const;
  QTextCharFormat badIdSelectorFormat() const;
  QTextCharFormat idSelectorMarkerFormat() const;
  QTextCharFormat badIdSelectorMarkerFormat() const;
  QTextCharFormat valueFormat() const;
  QTextCharFormat badValueFormat() const;
//  QTextCharFormat idSelectorFormat() const;
//  QTextCharFormat idSelectorMarkerFormat() const;
  QTextCharFormat pseudoStateFormat() const;
  QTextCharFormat badPseudoStateFormat() const;
  QTextCharFormat pseudoStateMarkerFormat() const;
  QTextCharFormat badPseudoStateMarkerFormat() const;
  QTextCharFormat subControlFormat() const;
  QTextCharFormat badSubControlFormat() const;
  QTextCharFormat subControlMarkerFormat() const;
  QTextCharFormat badSubControlMarkerFormat() const;
  QTextCharFormat propertyFormat() const;
  QTextCharFormat badPropertyFormat() const;
  QTextCharFormat propertyMarkerFormat() const;
  QTextCharFormat startBraceFormat() const;
  QTextCharFormat badStartBraceFormat() const;
  QTextCharFormat endBraceFormat() const;
  QTextCharFormat badEndBraceFormat() const;
  QTextCharFormat braceMatchFormat() const;
  QTextCharFormat badBraceMatchFormat() const;
  QTextCharFormat commentFormat() const;

  QTextCharFormat propertyEndMarkerFormat() const;

private:
  StylesheetEditor* m_editor;
  DataStore* m_datastore;
  QTextCharFormat m_baseFormat;
  QTextCharFormat m_widgetFormat;
  QTextCharFormat m_badWidgetFormat;
  QTextCharFormat m_seperatorFormat;
  QTextCharFormat m_valueFormat;
  QTextCharFormat m_badValueFormat;
  QTextCharFormat m_idSelectorFormat;
  QTextCharFormat m_badIdSelectorFormat;
  QTextCharFormat m_idSelectorMarkerFormat;
  QTextCharFormat m_badIdSelectorMarkerFormat;
  QTextCharFormat m_pseudoStateFormat;
  QTextCharFormat m_badPseudoStateFormat;
  QTextCharFormat m_pseudoStateMarkerFormat;
  QTextCharFormat m_badPseudoStateMarkerFormat;
  QTextCharFormat m_subControlFormat;
  QTextCharFormat m_badSubControlFormat;
  QTextCharFormat m_subControlMarkerFormat;
  QTextCharFormat m_badSubControlMarkerFormat;
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
  bool isInBlock(int position, int length, int blockStart, int blockEnd);
  void formatVisiblePart(int blockStart,
                         int blockEnd,
                         int position,
                         int length,
                         QTextCharFormat format);
  void formatPosition(int position,
                      int length,
                      int blockEnd,
                      QTextCharFormat format);
  void formatPseudoState(PseudoState* state, int blockStart, int blockEnd);
  void formatSubControl(SubControl* subcontrol, int blockStart, int blockEnd);
  void formatIdSelector(IDSelector* selector, int blockStart, int blockEnd);
  void formatProperty(PropertyNode* property,
                      int blockStart,
                      int blockEnd,
                      bool finalBlock = false);
  bool checkForEmpty(const QString& text);
  void formatControlBase(ControlBase* control,
                         int blockStart,
                         int blockEnd,
                         QTextCharFormat goodFormat,
                         QTextCharFormat badFormat,
                         QTextCharFormat goodMarkerFormat,
                         QTextCharFormat badMarkerFormat);
};

#endif // STYLESHEETHIGHLIGHTER_H
