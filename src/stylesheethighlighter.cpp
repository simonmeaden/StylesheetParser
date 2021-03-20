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
#include "stylesheethighlighter.h"
#include "node.h"
#include "stylesheetedit_p.h"

StylesheetHighlighter::StylesheetHighlighter(StylesheetEditor* editor,
                                             DataStore* datastore)
  : QSyntaxHighlighter(editor->document())
  , m_editor(editor)
  , m_datastore(datastore)
{
  m_back = editor->palette().brush(QPalette::Base);
  setWidgetFormat(QColor(0x80, 0, 0x80), m_back, DataStore::LIGHTFONT);
  setBadWidgetFormat(QColor(0x80, 0, 0x80), m_back, DataStore::LIGHTFONT);
  setSeperatorFormat(Qt::black, m_back, DataStore::LIGHTFONT);
  setPseudoStateFormat(QColor(0x80, 0x80, 0), m_back, DataStore::LIGHTFONT);
  setBadPseudoStateFormat(QColor(0x80, 0x80, 0), m_back, DataStore::LIGHTFONT);
  setPseudoStateMarkerFormat(QColor(Qt::black), m_back, DataStore::LIGHTFONT);
  setBadPseudoStateMarkerFormat(
    QColor(Qt::black), m_back, DataStore::LIGHTFONT);
  setIdSelectorFormat(QColor("darkviolet"), m_back, DataStore::LIGHTFONT);
  setBadIdSelectorFormat(QColor("darkviolet"), m_back, DataStore::LIGHTFONT);
  setIdSelectorMarkerFormat(Qt::black, m_back, DataStore::LIGHTFONT);
  setBadIdSelectorMarkerFormat(Qt::black, m_back, DataStore::LIGHTFONT);
  setSubControlFormat(QColor(0xCE, 0x5C, 0), m_back, DataStore::LIGHTFONT);
  setBadSubControlFormat(QColor(0xCE, 0x5C, 0), m_back, DataStore::LIGHTFONT);
  setSubControlMarkerFormat(QColor(Qt::black), m_back, DataStore::LIGHTFONT);
  setBadSubControlMarkerFormat(QColor(Qt::black), m_back, DataStore::LIGHTFONT);
  setPropertyFormat(QColor("darkblue"), m_back, DataStore::LIGHTFONT);
  setBadPropertyFormat(QColor("darkblue"), m_back, DataStore::LIGHTFONT);
  setPropertyMarkerFormat(QColor(Qt::black), m_back, DataStore::LIGHTFONT);
  setPropertyEndMarkerFormat(QColor(Qt::black), m_back, DataStore::LIGHTFONT);
  setPropertyValueFormat(QColor("firebrick"), m_back, DataStore::LIGHTFONT);
  setBadPropertyValueFormat(QColor("firebrick"), m_back, DataStore::LIGHTFONT);
  setStartBraceFormat(QColor("blue"), m_back, DataStore::LIGHTFONT);
  setBadStartBraceFormat(QColor("blue"), m_back, DataStore::LIGHTFONT);
  setEndBraceFormat(QColor("lightblue"), m_back, DataStore::LIGHTFONT);
  setBadEndBraceFormat(QColor("lightblue"), m_back, DataStore::LIGHTFONT);
  setBraceMatchFormat(
    QColor(Qt::red), QColor("lightgreen"), DataStore::NORMALFONT);
  setBadBraceMatchFormat(
    QColor(Qt::red), QColor("lightgreen"), DataStore::NORMALFONT);
  setCommentFormat(QColor("darkmagenta"), m_back, DataStore::LIGHTFONT);
  //  setWidgetFormat(QColor(0x80, 0, 0x80), m_back, DataStore::LIGHTFONT);
  //  setBadWidgetFormat(QColor(0x80, 0, 0x80), m_back, DataStore::LIGHTFONT);
  //  setSeperatorFormat(Qt::black, m_back, DataStore::LIGHTFONT);
  //  setPseudoStateFormat(QColor(0x80, 0x80, 0), m_back, DataStore::LIGHTFONT);
  //  setBadPseudoStateFormat(QColor(0x80, 0x80, 0), m_back,
  //  DataStore::LIGHTFONT); setPseudoStateMarkerFormat(QColor(Qt::black),
  //  m_back, DataStore::LIGHTFONT); setBadPseudoStateMarkerFormat(
  //    QColor(Qt::black), m_back, DataStore::LIGHTFONT);
  //  setIdSelectorFormat(QColor("darkviolet"), m_back, DataStore::LIGHTFONT);
  //  setBadIdSelectorFormat(QColor("darkviolet"), m_back,
  //  DataStore::LIGHTFONT); setIdSelectorMarkerFormat(Qt::black, m_back,
  //  DataStore::LIGHTFONT); setBadIdSelectorMarkerFormat(Qt::black, m_back,
  //  DataStore::LIGHTFONT); setSubControlFormat(QColor(0xCE, 0x5C, 0), m_back,
  //  DataStore::LIGHTFONT); setBadSubControlFormat(QColor(0xCE, 0x5C, 0),
  //  m_back, DataStore::LIGHTFONT);
  //  setSubControlMarkerFormat(QColor(Qt::black), m_back,
  //  DataStore::LIGHTFONT); setBadSubControlMarkerFormat(QColor(Qt::black),
  //  m_back, DataStore::LIGHTFONT); setPropertyFormat(QColor("darkblue"),
  //  m_back, DataStore::LIGHTFONT); setBadPropertyFormat(QColor("darkblue"),
  //  m_back, DataStore::LIGHTFONT); setPropertyMarkerFormat(QColor(Qt::black),
  //  m_back, DataStore::LIGHTFONT);
  //  setPropertyEndMarkerFormat(QColor(Qt::black), m_back,
  //  DataStore::LIGHTFONT); setPropertyValueFormat(QColor("firebrick"), m_back,
  //  DataStore::LIGHTFONT); setBadPropertyValueFormat(QColor("firebrick"),
  //  m_back, DataStore::LIGHTFONT); setStartBraceFormat(QColor("blue"), m_back,
  //  DataStore::LIGHTFONT); setBadStartBraceFormat(QColor("blue"), m_back,
  //  DataStore::LIGHTFONT); setEndBraceFormat(QColor("lightblue"), m_back,
  //  DataStore::LIGHTFONT); setBadEndBraceFormat(QColor("lightblue"), m_back,
  //  DataStore::LIGHTFONT); setBraceMatchFormat(
  //    QColor(Qt::red), QColor("lightgreen"), DataStore::NORMALFONT);
  //  setBadBraceMatchFormat(
  //    QColor(Qt::red), QColor("lightgreen"), DataStore::NORMALFONT);
  //  setCommentFormat(QColor("darkmagenta"), m_back, DataStore::LIGHTFONT);
}

int
StylesheetHighlighter::setNodeEnd(int nodeEnd, int blockEnd)
{
  if (blockEnd < nodeEnd) {
    return blockEnd;
  }

  return nodeEnd;
}

int
StylesheetHighlighter::setNodeStart(int nodeStart, int blockStart)
{
  if (blockStart > nodeStart) {
    nodeStart = blockStart;
  }

  return nodeStart;
}

bool
StylesheetHighlighter::isInBlock(int position,
                                 int length,
                                 int blockStart,
                                 int blockEnd)
{
  int end = position + length;

  if (position >= blockStart && position < blockEnd)
    return true;
  if (end >= blockStart && end < blockEnd)
    return true;
  if (position < blockStart && end > blockEnd)
    return true;
  return false;
}

void
StylesheetHighlighter::formatVisiblePart(int blockStart,
                                         int blockEnd,
                                         int position,
                                         int length,
                                         QTextCharFormat format)
{
  auto nodeEnd = position + length;
  int p = 0, l = 0;

  if (position == blockStart && nodeEnd == blockEnd) {
    p = position;
    l = length;
  } else if (position < blockStart && nodeEnd < blockEnd) {
    p = 0;
    l = length - (blockStart - position);
  } else if (position >= blockStart && nodeEnd < blockEnd) {
    p = position - blockStart;
    l = length;
  } else if (position >= blockStart && nodeEnd >= blockEnd) {
    p = position - blockStart;
    l = length - (nodeEnd - blockEnd);
  } else if (position < blockStart && nodeEnd >= blockEnd) {
    p = 0;
    l = blockEnd - blockStart;
  }

  if (l > 0)
    setFormat(p, l, format);
}

void
StylesheetHighlighter::formatPosition(int position,
                                      int length,
                                      int blockEnd,
                                      QTextCharFormat format)
{

  if (position < blockEnd) {
    setFormat(position, length, format);
  }
}

void
StylesheetHighlighter::formatPseudoState(PseudoState* state,
                                         int blockStart,
                                         int blockEnd)
{
  int position, length;
  if (state->hasMarker()) {
    position = state->position();
    if (state->isValid()) {
      formatVisiblePart(
        blockStart, blockEnd, position, 1, m_pseudoStateMarkerFormat);
    } else {
      formatVisiblePart(
        blockStart, blockEnd, position, 1, m_badPseudoStateMarkerFormat);
    }

    position = state->namePosition();
    length = state->name().length();
    if (isInBlock(position, length, blockStart, blockEnd)) {
      if (m_datastore->containsPseudoState(state->name())) {
        formatVisiblePart(
          blockStart, blockEnd, position, length, m_pseudoStateFormat);
      } else {
        formatVisiblePart(
          blockStart, blockEnd, position, length, m_badPseudoStateFormat);
      }
    }
  }
}

void
StylesheetHighlighter::formatControlBase(ControlBase* control,
                                         int blockStart,
                                         int blockEnd,
                                         QTextCharFormat goodFormat,
                                         QTextCharFormat badFormat,
                                         QTextCharFormat goodMarkerFormat,
                                         QTextCharFormat badMarkerFormat)
{
  auto position = control->position();
  auto length = 2;
  if (isInBlock(position, length, blockStart, blockEnd)) {
    if (control->hasMarker()) {
      formatVisiblePart(blockStart, blockEnd, position, 2, goodMarkerFormat);
    } else {
      formatVisiblePart(blockStart, blockEnd, position, 2, badMarkerFormat);
    }
  }

  position = control->namePosition();
  length = control->name().length();
  if (isInBlock(position, length, blockStart, blockEnd)) {
    if (control->isValid()) {
      formatVisiblePart(blockStart, blockEnd, position, length, goodFormat);
    } else {
      formatVisiblePart(blockStart, blockEnd, position, length, badFormat);
    }
  }

  if (control->hasPseudoStates()) {
    for (auto& state : *(control->pseudoStates())) {
      formatPseudoState(state, blockStart, blockEnd);
    }
  }
}

void
StylesheetHighlighter::formatSubControl(SubControl* subcontrol,
                                        int blockStart,
                                        int blockEnd)
{
  formatControlBase(subcontrol,
                    blockStart,
                    blockEnd,
                    m_subControlFormat,
                    m_badSubControlFormat,
                    m_subControlMarkerFormat,
                    m_badSubControlMarkerFormat);
}

void
StylesheetHighlighter::formatIdSelector(IDSelector* selector,
                                        int blockStart,
                                        int blockEnd)
{
  formatControlBase(selector,
                    blockStart,
                    blockEnd,
                    m_idSelectorFormat,
                    m_badIdSelectorFormat,
                    m_idSelectorMarkerFormat,
                    m_badIdSelectorMarkerFormat);
}

void
StylesheetHighlighter::formatProperty(PropertyNode* property,
                                      int blockStart,
                                      int blockEnd,
                                      bool finalBlock)
{
  auto length = property->name().length();
  auto position = property->position();
  if (isInBlock(position, length, blockStart, blockEnd)) {
    if (property->isValid(finalBlock)) {
      auto partialTypes = property->sectionIfIn(blockStart, blockEnd);
      for (auto pt : partialTypes) {
        switch (pt.type) {
          case PartialType::Name:
            if (property->isValidPropertyName()) {
              formatVisiblePart(
                blockStart, blockEnd, position, length, m_propertyFormat);
            } else {
              formatVisiblePart(
                blockStart, blockEnd, position, length, m_badPropertyFormat);
            }
            break;
          case PartialType::Marker:
            formatVisiblePart(blockStart,
                              blockEnd,
                              property->propertyMarkerPosition(),
                              1,
                              m_propertyMarkerFormat);
            break;
        }
      }
    } else {
      formatVisiblePart(
        blockStart, blockEnd, position, length, m_badPropertyFormat);
    }
  }

  NodeState check;
  PropertyStatus* status;
  QString value;

  for (int i = 0; i < property->count(); i++) {
    value = property->value(i);
    check = property->check(i);
    status = property->valueStatus(i);
    position = property->valuePosition(i);
    length = value.length();
    if (position >= blockEnd)
      break;

    if (isInBlock(position, length, blockStart, blockEnd)) {
      PropertyStatus::PropertyValueState state = status->state;
      switch (state) {
        case PropertyStatus::FuzzyGradientName: {
          formatVisiblePart(position - blockStart,
                            position - blockStart + status->name.length(),
                            position - blockStart,
                            status->name.length(),
                            m_badValueFormat);
          break;
        }
        case PropertyStatus::FuzzyColorValue: {
          formatVisiblePart(status->offset,
                            position + status->name.length(),
                            position,
                            length,
                            m_badValueFormat);
          break;
        }
          //      if (check == ValidPropertyValueState) {
          //        formatVisiblePart(
          //          blockStart, blockEnd, position, length, m_valueFormat);
          //      } else {
          //        formatVisiblePart(
          //          blockStart, blockEnd, position, length, m_badValueFormat);
          //      }
      }
    }

    if (property->hasPropertyEndMarker()) {
      position = property->propertyEndMarkerPosition();
      if (isInBlock(position, 1, blockStart, blockEnd)) {
        formatVisiblePart(
          blockStart, blockEnd, position, 1, m_propertyEndMarkerFormat);
      }
    }
  }
}

bool
StylesheetHighlighter::checkForEmpty(const QString& text)
{
  if (text.isEmpty() || text.trimmed().isEmpty()) {
    return true;
  }
  return false;
}

void
StylesheetHighlighter::highlightBlock(const QString& text)
{
  auto nodes = m_datastore->nodes();

  if (text.isEmpty() || nodes.isEmpty()) {
    return;
  }

  auto block = currentBlock();
  auto blockStart = block.position();
  auto blockEnd = blockStart + block.text().length();

  auto keys = nodes.keys();
  for (auto& key : keys) {
    auto node = nodes.value(key);
    auto type = node->type();
    int nodeStart = node->position();
    auto length = node->length();
    auto nodeEnd = nodeStart + length;
    int position;

    if (nodeEnd < blockStart) {
      continue;

    } else if (nodeStart >= blockEnd) {
      break;
    }

    switch (type) {
      case NodeType::NewlineType:
        break;

        //      case NodeType::FuzzyWidgetType:
      case WidgetsType: {
        auto widgets = qobject_cast<WidgetNodes*>(node);
        if (widgets) {
          for (auto& widget : widgets->widgets()) {
            position = widget->position();
            length = widget->name().length();
            if (isInBlock(position, length, blockStart, blockEnd)) {
              if (widget->isNameValid()) {
                formatVisiblePart(
                  blockStart, blockEnd, position, length, m_widgetFormat);
              } else {
                if (widget->position() < blockEnd) {
                  formatVisiblePart(
                    blockStart, blockEnd, position, length, m_badWidgetFormat);
                }
              }
            }

            if (widget->hasIdSelector()) {
              auto selector = widget->idSelector();
              formatIdSelector(selector, blockStart, blockEnd);
              if (selector->hasPseudoStates()) {
                for (auto& state : *(selector->pseudoStates())) {
                  formatPseudoState(state, blockStart, blockEnd);
                }
              }
            }

            if (widget->hasSubControl()) {
              for (auto& subcontrol : *(widget->subControls())) {
                formatSubControl(subcontrol, blockStart, blockEnd);
              }
            }
          }

          for (auto& seperator : widgets->seperators()) {
            position = seperator.anchor();
            length = 1;
            if (isInBlock(position, length, blockStart, blockEnd)) {
              formatVisiblePart(
                blockStart, blockEnd, position, length, m_seperatorFormat);
            }
          }

          bool hasstart = widgets->hasStartBrace();
          bool hasend = widgets->hasEndBrace();
          if (hasstart) {
            if (hasend) {
              formatVisiblePart(blockStart,
                                blockEnd,
                                widgets->startBracePosition(),
                                1,
                                m_startBraceFormat);
            } else {
              formatVisiblePart(blockStart,
                                blockEnd,
                                widgets->startBracePosition(),
                                1,
                                m_badStartBraceFormat);
            }
          }

          for (int i = 0; i < widgets->propertyCount(); i++) {
            auto property = widgets->property(i);
            if (property) {
              formatProperty(property, blockStart, blockEnd);
            }
          }

          if (hasend) {
            position = widgets->endBracePosition();
            if (isInBlock(position, 1, blockStart, blockEnd)) {
              if (hasstart) {
                formatVisiblePart(
                  blockStart, blockEnd, position, 1, m_endBraceFormat);
              } else {
                formatVisiblePart(
                  blockStart, blockEnd, position, 1, m_badEndBraceFormat);
              }
            }
          }
        }

        break;
      }

      case NodeType::CommentType: {
        formatVisiblePart(blockStart,
                          blockEnd,
                          node->position(),
                          node->length(),
                          m_commentFormat);
        break;
      }

      case NodeType::PropertyType: {
        PropertyNode* property = qobject_cast<PropertyNode*>(node);
        auto t = m_editor->toPlainText();
        bool finalBlock = checkForEmpty(t.mid(property->end()));
        formatProperty(property, blockStart, blockEnd, finalBlock);
        break;
      }

        //      case NodeType::StartBraceType: {
        //        StartBraceNode* startbrace =
        //        qobject_cast<StartBraceNode*>(node); if
        //        (startbrace->isBraceAtCursor()) {
        //          if (startbrace->hasEndBrace()) {
        //            setFormat(nodeStart, node->length(),
        //            m_braceMatchFormat);
        //          } else {
        //            setFormat(nodeStart, node->length(),
        //            m_badBraceMatchFormat);
        //          }
        //        } else {
        //          if (startbrace->hasEndBrace()) {
        //            setFormat(nodeStart, node->length(),
        //            m_startBraceFormat);
        //          } else {
        //            setFormat(nodeStart, node->length(),
        //            m_badStartBraceFormat);
        //          }
        //        }
        //        break;
        //      }

        //      case NodeType::EndBraceType: {
        //        EndBraceNode* endbrace = qobject_cast<EndBraceNode*>(node);
        //        if (endbrace->isBraceAtCursor()) {
        //          if (endbrace->hasStartBrace()) {
        //            setFormat(nodeStart, node->length(),
        //            m_braceMatchFormat);
        //          } else {
        //            setFormat(nodeStart, node->length(),
        //            m_badBraceMatchFormat);
        //          }
        //        } else {
        //          if (endbrace->hasStartBrace()) {
        //            setFormat(nodeStart, node->length(), m_endBraceFormat);
        //          } else {
        //            setFormat(nodeStart, node->length(),
        //            m_badEndBraceFormat);
        //          }
        //        }
        //        break;
        //      }

      default:
        break;
    }
  }
}

void
StylesheetHighlighter::setWidgetFormat(QBrush color,
                                       QBrush back,
                                       QFont font,
                                       QBrush underline,
                                       QTextCharFormat::UnderlineStyle style)
{
  m_widgetFormat.setFont(font);
  m_widgetFormat.setForeground(color.color());
  m_widgetFormat.setBackground(back.color());
  m_widgetFormat.setUnderlineColor(underline.color());
  m_widgetFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setWidgetFormat(QTextCharFormat format)
{
  m_widgetFormat = format;
}

void
StylesheetHighlighter::setBadWidgetFormat(QBrush color,
                                          QBrush back,
                                          QFont font,
                                          QBrush underline,
                                          QTextCharFormat::UnderlineStyle style)
{
  m_badWidgetFormat.setFont(font);
  m_badWidgetFormat.setForeground(color.color());
  m_badWidgetFormat.setBackground(back.color());
  m_badWidgetFormat.setUnderlineColor(underline.color());
  m_badWidgetFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setBadWidgetFormat(QTextCharFormat format)
{
  m_badWidgetFormat = format;
}

void
StylesheetHighlighter::setSeperatorFormat(QBrush color,
                                          QBrush back,
                                          QFont font,
                                          QBrush underline,
                                          QTextCharFormat::UnderlineStyle style)
{
  m_seperatorFormat.setFont(font);
  m_seperatorFormat.setForeground(color);
  m_seperatorFormat.setBackground(back);
  m_seperatorFormat.setUnderlineColor(underline.color());
  m_seperatorFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setSeperatorFormat(QTextCharFormat format)
{
  m_seperatorFormat = format;
}

void
StylesheetHighlighter::setIdSelectorFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  if (underline.style() == Qt::NoBrush)
    underline = m_back;
  m_idSelectorFormat.setFont(font);
  m_idSelectorFormat.setForeground(color);
  m_idSelectorFormat.setBackground(back);
  m_idSelectorFormat.setUnderlineColor(underline.color());
  m_idSelectorFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setIdSelectorFormat(QTextCharFormat format)
{
  m_idSelectorFormat = format;
}

void
StylesheetHighlighter::setBadIdSelectorFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_badIdSelectorFormat.setFont(font);
  m_badIdSelectorFormat.setForeground(color);
  m_badIdSelectorFormat.setBackground(back);
  m_badIdSelectorFormat.setUnderlineColor(underline.color());
  m_badIdSelectorFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setBadIdSelectorFormat(QTextCharFormat format)
{
  m_badIdSelectorFormat = format;
}

void
StylesheetHighlighter::setIdSelectorMarkerFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  if (underline.style() == Qt::NoBrush)
    underline = m_back;
  m_idSelectorMarkerFormat.setFont(font);
  m_idSelectorMarkerFormat.setForeground(color);
  m_idSelectorMarkerFormat.setBackground(back);
  m_idSelectorMarkerFormat.setUnderlineColor(underline.color());
  m_idSelectorMarkerFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setIdSelectorMarkerFormat(QTextCharFormat format)
{
  m_idSelectorMarkerFormat = format;
}

void
StylesheetHighlighter::setBadIdSelectorMarkerFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_badIdSelectorMarkerFormat.setFont(font);
  m_badIdSelectorMarkerFormat.setForeground(color);
  m_badIdSelectorMarkerFormat.setBackground(back);
  m_badIdSelectorMarkerFormat.setUnderlineColor(underline.color());
  m_badIdSelectorMarkerFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setBadIdSelectorMarkerFormat(QTextCharFormat format)
{
  m_badIdSelectorMarkerFormat = format;
}

void
StylesheetHighlighter::setPseudoStateFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_pseudoStateFormat.setFont(font);
  m_pseudoStateFormat.setForeground(color);
  m_pseudoStateFormat.setBackground(back);
  m_pseudoStateFormat.setUnderlineColor(underline.color());
  m_pseudoStateFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setPseudoStateFormat(QTextCharFormat format)
{
  m_pseudoStateFormat = format;
}

void
StylesheetHighlighter::setBadPseudoStateFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_badPseudoStateFormat.setFont(font);
  m_badPseudoStateFormat.setForeground(color);
  m_badPseudoStateFormat.setBackground(back);
  m_badPseudoStateFormat.setUnderlineColor(underline.color());
  m_badPseudoStateFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setBadPseudoStateFormat(QTextCharFormat format)
{
  m_badPseudoStateFormat = format;
}

void
StylesheetHighlighter::setPseudoStateMarkerFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_pseudoStateMarkerFormat.setFont(font);
  m_pseudoStateMarkerFormat.setForeground(color);
  m_pseudoStateMarkerFormat.setBackground(back);
  m_pseudoStateMarkerFormat.setUnderlineColor(underline.color());
  m_pseudoStateMarkerFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setPseudoStateMarkerFormat(QTextCharFormat format)
{
  m_pseudoStateMarkerFormat = format;
}

void
StylesheetHighlighter::setBadPseudoStateMarkerFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_badPseudoStateMarkerFormat.setFont(font);
  m_badPseudoStateMarkerFormat.setForeground(color);
  m_badPseudoStateMarkerFormat.setBackground(back);
  m_badPseudoStateMarkerFormat.setUnderlineColor(underline.color());
  m_badPseudoStateMarkerFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setBadPseudoStateMarkerFormat(QTextCharFormat format)
{
  m_badPseudoStateMarkerFormat = format;
}

void
StylesheetHighlighter::setSubControlFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_subControlFormat.setFont(font);
  m_subControlFormat.setForeground(color);
  m_subControlFormat.setBackground(back);
  m_subControlFormat.setUnderlineColor(underline.color());
  m_subControlFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setSubControlFormat(QTextCharFormat format)
{
  m_subControlFormat = format;
}

void
StylesheetHighlighter::setBadSubControlFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_badSubControlFormat.setFont(font);
  m_badSubControlFormat.setForeground(color);
  m_badSubControlFormat.setBackground(back);
  m_badSubControlFormat.setUnderlineColor(underline.color());
  m_badSubControlFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setBadSubControlFormat(QTextCharFormat format)
{
  m_badSubControlFormat = format;
}

void
StylesheetHighlighter::setSubControlMarkerFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_subControlMarkerFormat.setFont(font);
  m_subControlMarkerFormat.setForeground(color);
  m_subControlMarkerFormat.setBackground(back);
  m_subControlMarkerFormat.setUnderlineColor(underline.color());
  m_subControlMarkerFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setSubControlMarkerFormat(QTextCharFormat format)
{
  m_subControlMarkerFormat = format;
}

void
StylesheetHighlighter::setBadSubControlMarkerFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_badSubControlMarkerFormat.setFont(font);
  m_badSubControlMarkerFormat.setForeground(color);
  m_badSubControlMarkerFormat.setBackground(back);
  m_badSubControlMarkerFormat.setUnderlineColor(underline.color());
  m_badSubControlMarkerFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setBadSubControlMarkerFormat(QTextCharFormat format)
{
  m_badSubControlMarkerFormat = format;
}

void
StylesheetHighlighter::setPropertyValueFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_valueFormat.setFont(font);
  m_valueFormat.setForeground(color);
  m_valueFormat.setBackground(back);
  m_valueFormat.setUnderlineColor(underline.color());
  m_valueFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setPropertyValueFormat(QTextCharFormat format)
{
  m_valueFormat = format;
}

void
StylesheetHighlighter::setBadPropertyValueFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_badValueFormat.setFont(font);
  m_badValueFormat.setForeground(color);
  m_badValueFormat.setBackground(back);
  m_badValueFormat.setUnderlineColor(underline.color());
  m_badValueFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setBadPropertyValueFormat(QTextCharFormat format)
{
  m_badValueFormat = format;
}

void
StylesheetHighlighter::setPropertyFormat(QBrush color,
                                         QBrush back,
                                         QFont font,
                                         QBrush underline,
                                         QTextCharFormat::UnderlineStyle style)
{
  m_propertyFormat.setFont(font);
  m_propertyFormat.setForeground(color);
  m_propertyFormat.setBackground(back);
  m_propertyFormat.setUnderlineColor(underline.color());
  m_propertyFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setPropertyFormat(QTextCharFormat format)
{
  m_propertyFormat = format;
}

void
StylesheetHighlighter::setBadPropertyFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_badPropertyFormat.setFont(font);
  m_badPropertyFormat.setForeground(color);
  m_badPropertyFormat.setBackground(back);
  m_badPropertyFormat.setUnderlineColor(underline.color());
  m_badPropertyFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setBadPropertyFormat(QTextCharFormat format)
{
  m_badPropertyFormat = format;
}

void
StylesheetHighlighter::setPropertyMarkerFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_propertyMarkerFormat.setFont(font);
  m_propertyMarkerFormat.setForeground(color);
  m_propertyMarkerFormat.setBackground(back);
  m_propertyMarkerFormat.setUnderlineColor(underline.color());
  m_propertyMarkerFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setPropertyMarkerFormat(QTextCharFormat format)
{
  m_propertyMarkerFormat = format;
}

void
StylesheetHighlighter::setPropertyEndMarkerFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_propertyEndMarkerFormat.setFont(font);
  m_propertyEndMarkerFormat.setForeground(color);
  m_propertyEndMarkerFormat.setBackground(back);
  m_propertyEndMarkerFormat.setUnderlineColor(underline.color());
  m_propertyEndMarkerFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setPropertyEndMarkerFormat(QTextCharFormat format)
{
  m_propertyEndMarkerFormat = format;
}

void
StylesheetHighlighter::setStartBraceFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_startBraceFormat.setFont(font);
  m_startBraceFormat.setForeground(color);
  m_startBraceFormat.setBackground(back);
  m_startBraceFormat.setUnderlineColor(underline.color());
  m_startBraceFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setStartBraceFormat(QTextCharFormat format)
{
  m_startBraceFormat = format;
}

void
StylesheetHighlighter::setBadStartBraceFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_badStartBraceFormat.setFont(font);
  m_badStartBraceFormat.setForeground(color);
  m_badStartBraceFormat.setBackground(back);
  m_badStartBraceFormat.setUnderlineColor(underline.color());
  m_badStartBraceFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setBadStartBraceFormat(QTextCharFormat format)
{
  m_badStartBraceFormat = format;
}

void
StylesheetHighlighter::setEndBraceFormat(QBrush color,
                                         QBrush back,
                                         QFont font,
                                         QBrush underline,
                                         QTextCharFormat::UnderlineStyle style)
{
  m_endBraceFormat.setFont(font);
  m_endBraceFormat.setForeground(color);
  m_endBraceFormat.setBackground(back);
  m_endBraceFormat.setUnderlineColor(underline.color());
  m_endBraceFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setEndBraceFormat(QTextCharFormat format)
{
  m_endBraceFormat = format;
}

void
StylesheetHighlighter::setBadEndBraceFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_badEndBraceFormat.setFont(font);
  m_badEndBraceFormat.setForeground(color);
  m_badEndBraceFormat.setBackground(back);
  m_badEndBraceFormat.setUnderlineColor(underline.color());
  m_badEndBraceFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setBadEndBraceFormat(QTextCharFormat format)
{
  m_badEndBraceFormat = format;
}

void
StylesheetHighlighter::setBraceMatchFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_braceMatchFormat.setFont(font);
  m_braceMatchFormat.setForeground(color);
  m_braceMatchFormat.setBackground(back);
  m_braceMatchFormat.setUnderlineColor(underline.color());
  m_braceMatchFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setBraceMatchFormat(QTextCharFormat format)
{
  m_braceMatchFormat = format;
}

void
StylesheetHighlighter::setBadBraceMatchFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_badBraceMatchFormat.setFont(font);
  m_badBraceMatchFormat.setForeground(color);
  m_badBraceMatchFormat.setBackground(back);
  m_badBraceMatchFormat.setUnderlineColor(underline.color());
  m_badBraceMatchFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setBadBraceMatchFormat(QTextCharFormat format)
{
  m_badBraceMatchFormat = format;
}

void
StylesheetHighlighter::setCommentFormat(QBrush color,
                                        QBrush back,
                                        QFont font,
                                        QBrush underline,
                                        QTextCharFormat::UnderlineStyle style)
{
  m_commentFormat.setFont(font);
  m_commentFormat.setForeground(color);
  m_commentFormat.setBackground(back);
  m_commentFormat.setUnderlineColor(underline.color());
  m_commentFormat.setUnderlineStyle(style);
}

void
StylesheetHighlighter::setCommentFormat(QTextCharFormat format)
{
  m_commentFormat = format;
}

QTextCharFormat::UnderlineStyle
StylesheetHighlighter::underlinestyle() const
{
  return m_badValueFormat.underlineStyle();
}

QTextCharFormat
StylesheetHighlighter::widgetFormat() const
{
  return m_widgetFormat;
}

QTextCharFormat
StylesheetHighlighter::badWidgetFormat() const
{
  return m_badWidgetFormat;
}

QTextCharFormat
StylesheetHighlighter::seperatorFormat() const
{
  return m_seperatorFormat;
}

QTextCharFormat
StylesheetHighlighter::propertyValueFormat() const
{
  return m_valueFormat;
}

QTextCharFormat
StylesheetHighlighter::badPropertyValueFormat() const
{
  return m_badValueFormat;
}

QTextCharFormat
StylesheetHighlighter::idSelectorFormat() const
{
  return m_idSelectorFormat;
}

QTextCharFormat
StylesheetHighlighter::badIdSelectorFormat() const
{
  return m_badIdSelectorFormat;
}

QTextCharFormat
StylesheetHighlighter::idSelectorMarkerFormat() const
{
  return m_idSelectorMarkerFormat;
}

QTextCharFormat
StylesheetHighlighter::badIdSelectorMarkerFormat() const
{
  return m_badIdSelectorMarkerFormat;
}

QTextCharFormat
StylesheetHighlighter::pseudoStateFormat() const
{
  return m_pseudoStateFormat;
}

QTextCharFormat
StylesheetHighlighter::badPseudoStateFormat() const
{
  return m_badPseudoStateFormat;
}

QTextCharFormat
StylesheetHighlighter::pseudoStateMarkerFormat() const
{
  return m_pseudoStateMarkerFormat;
}

QTextCharFormat
StylesheetHighlighter::badPseudoStateMarkerFormat() const
{
  return m_badPseudoStateMarkerFormat;
}

QTextCharFormat
StylesheetHighlighter::subControlFormat() const
{
  return m_subControlFormat;
}

QTextCharFormat
StylesheetHighlighter::badSubControlFormat() const
{
  return m_badSubControlFormat;
}

QTextCharFormat
StylesheetHighlighter::subControlMarkerFormat() const
{
  return m_subControlMarkerFormat;
}

QTextCharFormat
StylesheetHighlighter::badSubControlMarkerFormat() const
{
  return m_badSubControlMarkerFormat;
}

QTextCharFormat
StylesheetHighlighter::propertyFormat() const
{
  return m_propertyFormat;
}

QTextCharFormat
StylesheetHighlighter::badPropertyFormat() const
{
  return m_badPropertyFormat;
}

QTextCharFormat
StylesheetHighlighter::propertyMarkerFormat() const
{
  return m_propertyMarkerFormat;
}

QTextCharFormat
StylesheetHighlighter::startBraceFormat() const
{
  return m_startBraceFormat;
}

QTextCharFormat
StylesheetHighlighter::badStartBraceFormat() const
{
  return m_badStartBraceFormat;
}

QTextCharFormat
StylesheetHighlighter::endBraceFormat() const
{
  return m_endBraceFormat;
}

QTextCharFormat
StylesheetHighlighter::badEndBraceFormat() const
{
  return m_badEndBraceFormat;
}

QTextCharFormat
StylesheetHighlighter::braceMatchFormat() const
{
  return m_braceMatchFormat;
}

QTextCharFormat
StylesheetHighlighter::badBraceMatchFormat() const
{
  return m_badBraceMatchFormat;
}

QTextCharFormat
StylesheetHighlighter::commentFormat() const
{
  return m_commentFormat;
}

QTextCharFormat
StylesheetHighlighter::propertyEndMarkerFormat() const
{
  return m_propertyEndMarkerFormat;
}
