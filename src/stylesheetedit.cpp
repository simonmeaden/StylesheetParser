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
#include "stylesheetparser/stylesheetedit.h"
#include "bookmarkarea.h"
#include "common.h"
#include "datastore.h"
#include "hoverwidget.h"
#include "linenumberarea.h"
#include "parser.h"
#include "stylesheetedit_p.h"
#include "stylesheethighlighter.h"

#include <QtDebug>

StylesheetEdit::StylesheetEdit(QWidget* parent)
  : QWidget(parent)
  , m_editor(new StylesheetEditor(this))
  , m_linenumberArea(new LineNumberArea(m_editor))
  , m_bookmarkArea(new BookmarkArea(m_editor))
{
  qRegisterMetaType<QTextCursor>("QTextCursor");
  qRegisterMetaType<MenuData>("MenuData");
  qRegisterMetaType<NodeType>("NodeType");
  qRegisterMetaType<NodeCheck>("NodeCheck");

  m_editor->setup(m_bookmarkArea, m_linenumberArea);

  auto layout = new QGridLayout;
  setContentsMargins(0, 0, 0, 0);
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);

  layout->addWidget(m_linenumberArea, 0, 0);
  layout->addWidget(m_bookmarkArea, 0, 1);
  layout->addWidget(m_editor, 0, 2);
}

void
StylesheetEdit::setPlainText(const QString& text)
{
  m_editor->setPlainText(text);
}

void
StylesheetEdit::setShowNewlineMarkers(bool show)
{
  m_editor->setShowNewlineMarkers(show);
}

bool
StylesheetEdit::showNewlineMarkers()
{
  return m_editor->showNewlineMarkers();
}

QString
StylesheetEdit::styleSheet() const
{
  return m_editor->styleSheet();
}

void
StylesheetEdit::setStyleSheet(const QString& stylesheet)
{
  m_editor->setStyleSheet(stylesheet);
}

void
StylesheetEdit::setValueFormat(QColor color, QColor back, QFont::Weight weight)
{
  m_editor->setValueFormat(color, back, weight);
}

void
StylesheetEdit::setWidgetFormat(QColor color, QColor back, QFont::Weight weight)
{
  m_editor->setWidgetFormat(color, back, weight);
}

void
StylesheetEdit::setPseudoStateFormat(QColor color,
                                     QColor back,
                                     QFont::Weight weight)
{
  m_editor->setPseudoStateFormat(color, back, weight);
}

void
StylesheetEdit::setPseudoStateMarkerFormat(QColor color,
                                           QColor back,
                                           QFont::Weight weight)
{
  m_editor->setPseudoStateMarkerFormat(color, back, weight);
}

void
StylesheetEdit::setSubControlFormat(QColor color,
                                    QColor back,
                                    QFont::Weight weight)
{
  m_editor->setSubControlFormat(color, back, weight);
}

void
StylesheetEdit::setSubControlMarkerFormat(QColor color,
                                          QColor back,
                                          QFont::Weight weight)
{
  m_editor->setSubControlMarkerFormat(color, back, weight);
}

void
StylesheetEdit::setPropertyFormat(QColor color,
                                  QColor back,
                                  QFont::Weight weight)
{
  m_editor->setPropertyFormat(color, back, weight);
}

void
StylesheetEdit::setPropertyMarkerFormat(QColor color,
                                        QColor back,
                                        QFont::Weight weight)
{
  m_editor->setPropertyMarkerFormat(color, back, weight);
}

void
StylesheetEdit::setLineNumberFormat(QColor color,
                                    QColor back,
                                    QFont::Weight weight)
{
  m_editor->setLineNumberFormat(color, back, weight);
}

void
StylesheetEdit::setStartBraceFormat(QColor color,
                                    QColor back,
                                    QFont::Weight weight)
{
  m_editor->setStartBraceFormat(color, back, weight);
}

void
StylesheetEdit::setEndBraceFormat(QColor color,
                                  QColor back,
                                  QFont::Weight weight)
{
  m_editor->setEndBraceFormat(color, back, weight);
}

void
StylesheetEdit::setBraceMatchFormat(QColor color,
                                    QColor back,
                                    QFont::Weight weight)
{
  m_editor->setBraceMatchFormat(color, back, weight);
}

void
StylesheetEdit::setBookmarks(QMap<int, BookmarkData*>* bookmarks)
{
  m_editor->setBookmarks(bookmarks);
}

void
StylesheetEdit::insertBookmark(int bookmark, const QString& text)
{
  m_bookmarkArea->insertBookmark(bookmark, text);
  //  m_editor->insertBookmark(bookmark, text);
}

void
StylesheetEdit::toggleBookmark(int bookmark)
{
  m_editor->toggleBookmark(bookmark);
}

void
StylesheetEdit::editBookmark(int bookmark)
{
  m_editor->editBookmark(bookmark);
}

void
StylesheetEdit::removeBookmark(int bookmark)
{
  m_editor->removeBookmark(bookmark);
}

void
StylesheetEdit::clearBookmarks()
{
  m_editor->clearBookmarks();
}

bool
StylesheetEdit::hasBookmark(int linenumber)
{
  return m_editor->hasBookmark(linenumber);
}

bool
StylesheetEdit::hasBookmarkText(int bookmark)
{
  return m_editor->hasBookmarkText(bookmark);
}

QString
StylesheetEdit::bookmarkText(int bookmark)
{
  return m_editor->bookmarkText(bookmark);
}

int
StylesheetEdit::bookmarkCount()
{
  return m_editor->bookmarkCount();
}

void
StylesheetEdit::gotoBookmark(int bookmark)
{
  if (m_bookmarkArea->hasBookmark(bookmark)) {
    m_editor->goToLine(bookmark);
  }
  //  m_editor->gotoBookmark(bookmark);
}

void
StylesheetEdit::gotoBookmarkDialog()
{
  m_editor->gotoBookmarkDialog();
}

void
StylesheetEdit::format()
{
  m_editor->format();
}

int
StylesheetEdit::currentLineNumber() const
{
  return m_editor->currentLineNumber();
}

void
StylesheetEdit::up(int n)
{
  m_editor->up(n);
}

void
StylesheetEdit::down(int n)
{
  m_editor->down(n);
}

void
StylesheetEdit::left(int n)
{
  m_editor->left(n);
}

void
StylesheetEdit::right(int n)
{
  m_editor->right(n);
}

void
StylesheetEdit::start()
{
  m_editor->start();
}

void
StylesheetEdit::end()
{
  m_editor->end();
}

void
StylesheetEdit::startOfLine()
{
  m_editor->startOfLine();
}

void
StylesheetEdit::endOfLine()
{
  m_editor->endOfLine();
}

void
StylesheetEdit::goToLine(int lineNumber)
{
  //  if (lineNumber > 0 && lineNumber < getLineCount())
  m_editor->goToLine(lineNumber);
}

int
StylesheetEdit::maxSuggestionCount() const
{
  return m_editor->maxSuggestionCount();
}

void
StylesheetEdit::setMaxSuggestionCount(int maxSuggestionCount)
{
  m_editor->setMaxSuggestionCount(maxSuggestionCount);
}

bool
StylesheetEditorPrivate::checkForEmpty(const QString& text)
{
  if (text.isEmpty() || text.trimmed().isEmpty()) {
    return true;
  }
  return false;
}

const QChar StylesheetEditor::m_arrow = QChar(0x2BC8);

//=== StylesheetEdit
//================================================================
StylesheetEditor::StylesheetEditor(QWidget* parent)
  : QPlainTextEdit(parent)
  , d_ptr(new StylesheetEditorPrivate(this))
{
  setMouseTracking(true);
}

void
StylesheetEditor::setup(BookmarkArea* bookmarkArea,
                        LineNumberArea* linenumberArea)
{
  d_ptr->setup(bookmarkArea, linenumberArea);
}

StylesheetEditorPrivate::StylesheetEditorPrivate(StylesheetEditor* parent)
  : q_ptr(parent)
  , m_parseComplete(false)
  , m_oldSection(new NodeSection())
{}

void
StylesheetEditorPrivate::setup(BookmarkArea* bookmarkArea,
                               LineNumberArea* linenumberArea)
{
  m_bookmarkArea = bookmarkArea;
  m_lineNumberArea = linenumberArea;

  m_datastore = new DataStore();
  m_parser = new Parser(m_datastore, q_ptr);

  m_highlighter = new StylesheetHighlighter(q_ptr, m_datastore);

  q_ptr->connect(m_parser,
                 &Parser::parseComplete,
                 q_ptr,
                 &StylesheetEditor::handleParseComplete);
  q_ptr->connect(m_parser,
                 &Parser::rehighlight,
                 q_ptr,
                 &StylesheetEditor::handleRehighlight);
  q_ptr->connect(m_parser,
                 &Parser::rehighlightBlock,
                 q_ptr,
                 &StylesheetEditor::handleRehighlightBlock);

  q_ptr->connect(q_ptr,
                 &QPlainTextEdit::cursorPositionChanged,
                 q_ptr,
                 &StylesheetEditor::updateLineNumberArea);
  q_ptr->connect(q_ptr,
                 &QPlainTextEdit::cursorPositionChanged,
                 q_ptr,
                 &StylesheetEditor::cursorPositionHasChanged);
  q_ptr->connect(q_ptr->document(),
                 &QTextDocument::contentsChange,
                 q_ptr,
                 &StylesheetEditor::documentChanged,
                 Qt::UniqueConnection);
}

int
StylesheetEditorPrivate::maxSuggestionCount() const
{
  return m_parser->maxSuggestionCount();
}

void
StylesheetEditorPrivate::setMaxSuggestionCount(int maxSuggestionCount)
{
  m_parser->setMaxSuggestionCount(maxSuggestionCount);
}

int
StylesheetEditor::getLineCount()
{
  return d_ptr->getLineCount();
}

int
StylesheetEditorPrivate::getLineCount() const
{
  return m_lineNumberArea->lineCount();
}

void
StylesheetEditor::setPlainText(const QString& text)
{
  QPlainTextEdit::setPlainText(text);
  calculateLineNumber(textCursor());
  d_ptr->setPlainText(text);
  connect(document(),
          &QTextDocument::contentsChange,
          this,
          &StylesheetEditor::documentChanged,
          Qt::UniqueConnection);
}

void
StylesheetEditorPrivate::setPlainText(const QString& text)
{
  m_parser->parseInitialText(text);
}

void
StylesheetEditor::handleRehighlight()
{
  d_ptr->handleRehighlight();
}

void
StylesheetEditor::handleRehighlightBlock(const QTextBlock& block)
{
  d_ptr->handleRehighlightBlock(block);
}

void
StylesheetEditorPrivate::handleRehighlight()
{
  m_highlighter->rehighlight();
}

void
StylesheetEditorPrivate::handleRehighlightBlock(const QTextBlock& block)
{
  m_highlighter->rehighlightBlock(block);
}

void
StylesheetEditor::handleParseComplete()
{
  d_ptr->handleParseComplete();
}

void
StylesheetEditorPrivate::handleParseComplete()
{
  m_highlighter->rehighlight();
  setLineNumber(1);
  m_parseComplete = true;
  emit q_ptr->lineNumber(currentLineNumber());
  emit q_ptr->lineCount(m_lineNumberArea->lineCount());
}

void
StylesheetEditor::setShowNewlineMarkers(bool show)
{
  d_ptr->setShowNewlineMarkers(show);
}

bool
StylesheetEditor::showNewlineMarkers()
{
  return d_ptr->showLineMarkers();
}

void
StylesheetEditorPrivate::setShowNewlineMarkers(bool show)
{
  m_parser->setShowLineMarkers(show);
}

bool
StylesheetEditorPrivate::showLineMarkers()
{
  return m_parser->showLineMarkers();
}

QString
StylesheetEditor::styleSheet() const
{
  return d_ptr->styleSheet();
}

QString
StylesheetEditorPrivate::styleSheet() const
{
  return m_stylesheet;
}

bool
StylesheetEditorPrivate::checkStylesheetColors(StylesheetData* data,
                                               QColor& color1,
                                               QColor& color2,
                                               QColor& color3)
{
  if (data->colors.count() == 1) {
    color1 = QColor(data->colors.at(0));
    color2 = QColor(Qt::white);
    color2 = QColor(Qt::red);

  } else if (data->colors.count() == 2) {
    color1 = QColor(data->colors.at(0));
    color2 = QColor(data->colors.at(1));
    color2 = QColor(Qt::red);
    return true;

  } else if (data->colors.count() == 3) {
    color1 = QColor(data->colors.at(0));
    color2 = QColor(data->colors.at(1));
    color3 = QColor(data->colors.at(1));
    return true;
  }

  return false;
}

void
StylesheetEditor::setStyleSheet(const QString& stylesheet)
{
  d_ptr->setStyleSheet(stylesheet);
}

void
StylesheetEditorPrivate::setStyleSheet(const QString& stylesheet)
{
  // TODO implement standard stylesheet shit.
  m_stylesheet = stylesheet;
  QString sheet = stylesheet.simplified();
  QString reEditor, reBetweenCurly;

  reEditor = "StylesheetEdit[\\s]*{[\\-\\w:;\\s]*}";
  reBetweenCurly = "{([^}]*)}";

  QRegularExpression re;
  re.setPattern(reEditor);
  QRegularExpressionMatchIterator it = re.globalMatch(sheet);

  while (it.hasNext()) {
    QRegularExpressionMatch matcher = it.next();

    // detects StylesheetEdit widget
    for (auto match : matcher.capturedTexts()) {
      re.setPattern(reBetweenCurly);
      matcher = re.match(match);

      // matches curly braces
      if (matcher.hasMatch()) {
        match = matcher.captured(0);
        // remove curly braces.
        sheet = match.mid(1, match.length() - 2).toLower();

        int pos = 0;
        StylesheetData* data = nullptr;
        QColor color1, color2, color3;
        QTextCharFormat::UnderlineStyle underlineStyle =
          QTextCharFormat::NoUnderline;
        QFont::Weight fontWeight = QFont::Light;
        bool colorError = false;

        while (pos < sheet.length()) {
          data = m_parser->getStylesheetProperty(sheet, pos);

          if (data->name.isEmpty()) {
            return;
          }

          color1 = (data->colors.size() > 0 ? data->colors.at(0) : QColor());
          color2 = (data->colors.size() > 1 ? data->colors.at(1) : QColor());
          color3 = (data->colors.size() > 2 ? data->colors.at(1) : QColor());
          colorError = checkStylesheetColors(data, color1, color2, color3);
          underlineStyle =
            (data->underline.size() > 0 ? data->underline.at(0)
                                        : QTextCharFormat::NoUnderline);
          fontWeight =
            (data->weights.size() > 0 ? data->weights.at(0) : QFont::Normal);
          // TODO ignore any more than 3 colours or one weight or underline
          // type,
          // TODO maybe drop earlier values.

          if (data->name == "widget") {
            m_highlighter->setWidgetFormat(color1, color2, fontWeight);
            continue;

          } else if (data->name == "subcontrol") {
            m_highlighter->setSubControlFormat(color1, color2, fontWeight);
            continue;

          } else if (data->name == "subcontrolmarker") {
            m_highlighter->setSubControlMarkerFormat(
              color1, color2, fontWeight);
            continue;

          } else if (data->name == "pseudostate") {
            m_highlighter->setPseudoStateFormat(color1, color2, fontWeight);
            continue;

          } else if (data->name == "pseudostatemarker") {
            m_highlighter->setPseudoStateMarkerFormat(
              color1, color2, fontWeight);
            continue;

          } else if (data->name == "property") {
            m_highlighter->setPropertyFormat(color1, color2, fontWeight);
            continue;

          } else if (data->name == "propertymarker") {
            m_highlighter->setPropertyMarkerFormat(color1, color2, fontWeight);
            continue;

          } else if (data->name == "value") {
            m_highlighter->setValueFormat(color1, color2, fontWeight);
            continue;

          } else if (data->name == "startbrace") {
            m_highlighter->setStartBraceFormat(color1, color2, fontWeight);
            continue;

          } else if (data->name == "endbrace") {
            m_highlighter->setEndBraceFormat(color1, color2, fontWeight);
            continue;

          } else if (data->name == "bracematch") {
            m_highlighter->setBraceMatchFormat(color1, color2, fontWeight);
            continue;

          } else if (data->name == "comment") {
            m_highlighter->setCommentFormat(color1, color2, fontWeight);
            continue;

          } /* else if (data->name == "bad") {

             m_highlighter->setBadValueFormat(
               color1, color2, fontWeight, true, underlineStyle, color3);
             continue;
           }*/
        }
      }
    }
  }

  m_highlighter->rehighlight();
}

void
StylesheetEditor::setValueFormat(QColor color,
                                 QColor back,
                                 QFont::Weight weight)
{
  d_ptr->setValueFormat(color, back, weight);
}

void
StylesheetEditorPrivate::setValueFormat(QColor color,
                                        QColor back,
                                        QFont::Weight weight)
{
  m_highlighter->setValueFormat(color, back, weight);
}

void
StylesheetEditor::setWidgetFormat(QColor color,
                                  QColor back,
                                  QFont::Weight weight)
{
  d_ptr->setWidgetFormat(color, back, weight);
}

void
StylesheetEditorPrivate::setWidgetFormat(QColor color,
                                         QColor back,
                                         QFont::Weight weight)
{
  m_highlighter->setWidgetFormat(color, back, weight);
}

void
StylesheetEditor::setPseudoStateFormat(QColor color,
                                       QColor back,
                                       QFont::Weight weight)
{
  d_ptr->setPseudoStateFormat(color, back, weight);
}

void
StylesheetEditorPrivate::setPseudoStateFormat(QColor color,
                                              QColor back,
                                              QFont::Weight weight)
{
  m_highlighter->setPseudoStateFormat(color, back, weight);
}

void
StylesheetEditor::setPseudoStateMarkerFormat(QColor color,
                                             QColor back,
                                             QFont::Weight weight)
{
  d_ptr->setPseudoStateMarkerFormat(color, back, weight);
}

void
StylesheetEditorPrivate::setPseudoStateMarkerFormat(QColor color,
                                                    QColor back,
                                                    QFont::Weight weight)
{
  m_highlighter->setPseudoStateMarkerFormat(color, back, weight);
}

void
StylesheetEditor::setSubControlFormat(QColor color,
                                      QColor back,
                                      QFont::Weight weight)
{
  d_ptr->setSubControlFormat(color, back, weight);
}
void
StylesheetEditorPrivate::setSubControlFormat(QColor color,
                                             QColor back,
                                             QFont::Weight weight)
{
  m_highlighter->setSubControlFormat(color, back, weight);
}

void
StylesheetEditor::setSubControlMarkerFormat(QColor color,
                                            QColor back,
                                            QFont::Weight weight)
{
  d_ptr->setSubControlMarkerFormat(color, back, weight);
}

void
StylesheetEditorPrivate::setSubControlMarkerFormat(QColor color,
                                                   QColor back,
                                                   QFont::Weight weight)
{
  m_highlighter->setSubControlMarkerFormat(color, back, weight);
}

void
StylesheetEditor::setPropertyFormat(QColor color,
                                    QColor back,
                                    QFont::Weight weight)
{
  d_ptr->setPropertyFormat(color, back, weight);
}

void
StylesheetEditorPrivate::setPropertyFormat(QColor color,
                                           QColor back,
                                           QFont::Weight weight)
{
  m_highlighter->setPropertyFormat(color, back, weight);
}

void
StylesheetEditor::setPropertyMarkerFormat(QColor color,
                                          QColor back,
                                          QFont::Weight weight)
{
  d_ptr->setPropertyMarkerFormat(color, back, weight);
}

void
StylesheetEditorPrivate::setPropertyMarkerFormat(QColor color,
                                                 QColor back,
                                                 QFont::Weight weight)
{
  m_highlighter->setPropertyMarkerFormat(color, back, weight);
}

void
StylesheetEditorPrivate::setPropertyEndMarkerFormat(QColor color,
                                                    QColor back,
                                                    QFont::Weight weight)
{
  m_highlighter->setPropertyMarkerFormat(color, back, weight);
}

void
StylesheetEditor::setLineNumberFormat(QColor color,
                                      QColor back,
                                      QFont::Weight weight)
{
  d_ptr->setLineNumberFormat(color, back, weight);
}

void
StylesheetEditorPrivate::setLineNumberFormat(QColor color,
                                             QColor back,
                                             QFont::Weight weight)
{
  m_lineNumberArea->setForeSelected(color);
  m_lineNumberArea->setBack(back);
  m_lineNumberArea->setWeight(weight);
}

void
StylesheetEditor::setStartBraceFormat(QColor color,
                                      QColor back,
                                      QFont::Weight weight)
{
  d_ptr->setStartBraceFormat(color, back, weight);
}

void
StylesheetEditorPrivate::setStartBraceFormat(QColor color,
                                             QColor back,
                                             QFont::Weight weight)
{
  m_highlighter->setStartBraceFormat(color, back, weight);
}

void
StylesheetEditor::setEndBraceFormat(QColor color,
                                    QColor back,
                                    QFont::Weight weight)
{
  d_ptr->setEndBraceFormat(color, back, weight);
}

void
StylesheetEditorPrivate::setEndBraceFormat(QColor color,
                                           QColor back,
                                           QFont::Weight weight)
{
  m_highlighter->setEndBraceFormat(color, back, weight);
}

void
StylesheetEditor::setBraceMatchFormat(QColor color,
                                      QColor back,
                                      QFont::Weight weight)
{
  d_ptr->setBraceMatchFormat(color, back, weight);
}

void
StylesheetEditorPrivate::setBraceMatchFormat(QColor color,
                                             QColor back,
                                             QFont::Weight weight)
{
  m_highlighter->setBraceMatchFormat(color, back, weight);
}

// void
// StylesheetEditor::initMenus()
//{
//  d_ptr->initMenus();
//}

int
StylesheetEditor::maxSuggestionCount() const
{
  return d_ptr->maxSuggestionCount();
}

void
StylesheetEditor::setMaxSuggestionCount(int maxSuggestionCount)
{
  d_ptr->setMaxSuggestionCount(maxSuggestionCount);
}

// void
// StylesheetEditor::keyPressEvent(QKeyEvent* event)
//{
//  QPlainTextEdit::keyPressEvent(event);
//  if (!toPlainText().trimmed().isEmpty()) {
//    connect(document(),
//            &QTextDocument::contentsChange,
//            this,
//            &StylesheetEditor::documentChanged,
//            Qt::UniqueConnection);
//  } else {
//    disconnect(document(),
//               &QTextDocument::contentsChange,
//               this,
//               &StylesheetEditor::documentChanged);
//  }
//}

void
StylesheetEditor::mousePressEvent(QMouseEvent* event)
{
  d_ptr->handleMousePress(event);
  QPlainTextEdit::mousePressEvent(event);
}

bool
StylesheetEditorPrivate::handleMousePress(QMouseEvent* event)
{
  auto tc = q_ptr->cursorForPosition(event->pos());
  auto lineNumber = calculateLineNumber(tc);
  if (lineNumber >= 1) {
    setLineNumber(lineNumber);
    q_ptr->update();
  }
  return false;
}

void
StylesheetEditor::mouseMoveEvent(QMouseEvent* event)
{
  d_ptr->handleMouseMove(event);
  QPlainTextEdit::mouseMoveEvent(event);
}

void
StylesheetEditorPrivate::handleMouseMove(QMouseEvent* event)
{
  auto pos = event->pos();
  NodeSection* section = nullptr;
  QString hover;
  m_parser->nodeForPoint(pos, &section);
  q_ptr->setToolTip(QString());
  if (section) {
    if (*section != *m_oldSection) {
      if (section->node) {
        auto node = section->node;
        if (section->isWidgetType()) {
          auto widget = qobject_cast<WidgetNode*>(node);
          switch (section->type) {
            case SectionType::WidgetName: {
              if (widget->isNameFuzzy()) {
                setHoverFuzzyWidgetName(hover, widget->name());
              }
              break;
            }
            case SectionType::WidgetSubControlMarker:
            case SectionType::WidgetSubControl: {
              if (widget->isExtensionFuzzy()) {
                setHoverFuzzySubControl(hover, widget->extensionName());
              } else if (widget->isExtensionBad()) {
                setHoverBadSubControl(
                  hover, widget->name(), widget->extensionName());
              }
              break;
            }
            case SectionType::WidgetPseudoStateMarker:
            case SectionType::WidgetPseudoState: {
              if (widget->isExtensionFuzzy()) {
                if (widget->isPseudoState()) {
                  setHoverFuzzyPseudoState(hover, widget->extensionName());
                }
              }
              break;
            }
            default: {
              qWarning();
              break;
            }
          }
        } else if (section->isPropertyType()) {
          auto property = qobject_cast<PropertyNode*>(node);
          switch (section->type) {
            case SectionType::PropertyName: {
              if (!property->isValidPropertyName()) {
                hover.append(q_ptr->tr("Invalid property name <em>%1</em>")
                               .arg(property->name()));
              }
              if (!property->hasPropertyMarker()) {
                setHoverBadPropertyMarker(hover);
              }
              if (!property->hasPropertyEndMarker()) {
                setHoverBadPropertyEndMarker(hover, property);
              }
              break;
            }
            case SectionType::PropertyValue: {
              if (!property->hasPropertyEndMarker()) {
                setHoverBadPropertyEndMarker(hover, property);
              } else if (!property->hasPropertyMarker()) {
                setHoverBadPropertyMarker(hover);
              } else if (!property->isValueValid(section->position)) {
                hover.append(q_ptr->tr("Invalid property value <em>%1</em>")
                               .arg(property->name()));
              }
              break;
            }
            case SectionType::PropertyEndMarker:
            default:
              break;
          }
        }
      }
      if (!hover.isEmpty()) {
        QToolTip::showText(
          event->globalPos(), hover, q_ptr, q_ptr->viewport()->rect());
      } else {
        QToolTip::hideText();
      }
      m_oldSection = section;
    }
  } else {
    QToolTip::hideText();
    m_oldSection->clear();
  }
}

void
StylesheetEditorPrivate::setHoverBadPropertyMarker(QString& hover)
{
  if (!hover.isEmpty()) {
    hover.append("\n");
  }
  hover.append(q_ptr->tr("No property marker (<b>:</b>)"));
}

void
StylesheetEditorPrivate::setHoverFuzzyWidgetName(QString& hover,
                                                 const QString& name)
{
  if (!hover.isEmpty()) {
    hover.append("\n");
  }
  hover.append(
    q_ptr->tr("Invalid <em>fuzzy</em> widget name <em>%1</em>").arg(name));
}

void
StylesheetEditorPrivate::setHoverFuzzySubControl(QString& hover,
                                                 const QString& name)
{
  if (!hover.isEmpty()) {
    hover.append("\n");
  }
  hover.append(
    q_ptr->tr("Invalid <em>fuzzy</em> subcontrol <em>%1</em>").arg(name));
}

void
StylesheetEditorPrivate::setHoverBadSubControl(QString& hover,
                                               const QString& widget,
                                               const QString& name)
{
  if (!hover.isEmpty()) {
    hover.append("\n");
  }
  hover.append(q_ptr->tr("Invalid subcontrol <em>%1</em> for <em>%2</em>")
                 .arg(name)
                 .arg(widget));
}

void
StylesheetEditorPrivate::setHoverFuzzyPseudoState(QString& hover,
                                                  const QString& name)
{
  if (!hover.isEmpty()) {
    hover.append("\n");
  }
  hover.append(
    q_ptr->tr("Invalid <em>fuzzy</em> pseudo state <em>%1</em>").arg(name));
}

void
StylesheetEditorPrivate::setHoverBadPropertyEndMarker(
  QString& hover,
  const PropertyNode* property)
{
  auto text = q_ptr->toPlainText().mid(property->end());
  if (!checkForEmpty(text)) {
    if (!hover.isEmpty()) {
      hover.append("\n");
    }
    hover.append(q_ptr->tr("No end marker (<b>;</b>)"));
  }
}

void
StylesheetEditor::contextMenuEvent(QContextMenuEvent* event)
{
  d_ptr->handleCustomMenuRequested(event->pos());
}

void
StylesheetEditor::mouseReleaseEvent(QMouseEvent* event)
{
  QPlainTextEdit::mouseReleaseEvent(event);
}

void
StylesheetEditor::mouseDoubleClickEvent(QMouseEvent* event)
{
  QPlainTextEdit::mouseDoubleClickEvent(event);
}

int
StylesheetEditor::calculateLineNumber(QTextCursor textCursor)
{
  return d_ptr->calculateLineNumber(textCursor);
}

void
StylesheetEditor::format()
{
  d_ptr->format();
}

void
StylesheetEditorPrivate::format()
{
  // TODO pretty print format
}

QString
StylesheetEditorPrivate::getValueAtCursor(int pos, const QString& text)
{
  QChar c;
  QString value;
  int p = pos;

  while (true) {
    c = text.at(p);

    if (c.isLetterOrNumber()) {
      value.prepend(c);
      p--;

    } else {
      break;
    }
  }

  p = pos + 1;

  while (true) {
    c = text.at(p);

    if (c.isLetterOrNumber()) {
      value.append(c);
      p++;

    } else {
      break;
    }
  }

  return value;
}

void
StylesheetEditor::updateLineNumberArea()
{
  auto currentLineNumber =
    document()->findBlock(textCursor().position()).blockNumber() + 1;
  d_ptr->updateLineNumberArea(currentLineNumber);
}

// void
// StylesheetEditor::updateLeftArea(const QRect& rect, int dy)
//{
//  //  d_ptr->updateLineNumberArea(rect, dy);
//}

void
StylesheetEditorPrivate::updateLineNumberArea(int linenumber)
{
  m_lineNumberArea->setLineNumber(linenumber);
  m_lineNumberArea->update();
}

/*
   Calculates current line number and total line count.
*/
int
StylesheetEditorPrivate::calculateLineNumber(QTextCursor textCursor)
{
  QTextCursor cursor(textCursor);
  cursor.movePosition(QTextCursor::StartOfLine);

  int lines = 1;

  while (cursor.positionInBlock() > 0) {
    cursor.movePosition(QTextCursor::Up);
    lines++;
  }

  QTextBlock block = cursor.block().previous();

  while (block.isValid()) {
    lines += block.lineCount();
    block = block.previous();
  }

  int count = lines;

  block = cursor.block().next();

  while (block.isValid()) {
    count += block.lineCount();
    block = block.next();
  }

  m_lineNumberArea->setLineCount(count);

  return lines;
}

/*
   Calculates the current text column.
*/
int
StylesheetEditorPrivate::calculateColumn(QTextCursor textCursor)
{
  QTextCursor cursor(textCursor);
  cursor.movePosition(QTextCursor::StartOfLine);
  return textCursor.anchor() - cursor.anchor();
}

void
StylesheetEditor::cursorPositionHasChanged()
{
  d_ptr->cursorPositionChanged(textCursor());
}

void
StylesheetEditorPrivate::cursorPositionChanged(QTextCursor textCursor)
{
  m_parser->handleCursorPositionChanged(textCursor);
}

void
StylesheetEditor::suggestionMade(bool)
{
  auto act = dynamic_cast<QAction*>(sender());

  if (act) {
    d_ptr->suggestionMade(act);
  }
}

void
StylesheetEditor::bookmarkMenuRequested(QPoint pos)
{
  qWarning();
}

void
StylesheetEditor::linenumberMenuRequested(QPoint pos)
{
  qWarning();
}

void
StylesheetEditorPrivate::handleCustomMenuRequested(QPoint pos)
{
  QMenu* menu = new QMenu(q_ptr); // q_ptr->createStandardContextMenu();

  auto act = new QAction(q_ptr->tr("&Undo"), q_ptr);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z));
  q_ptr->connect(act, &QAction::triggered, q_ptr, &StylesheetEditor::undo);
  menu->addAction(act);

  act = new QAction(q_ptr->tr("&Redo"), q_ptr);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z));
  q_ptr->connect(act, &QAction::triggered, q_ptr, &StylesheetEditor::redo);
  menu->addAction(act);

  menu->addSeparator();

  act = new QAction(q_ptr->tr("Cu&t"), q_ptr);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_X));
  q_ptr->connect(act, &QAction::triggered, q_ptr, &StylesheetEditor::cut);
  menu->addAction(act);

  act = new QAction(q_ptr->tr("&Copy"), q_ptr);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
  q_ptr->connect(act, &QAction::triggered, q_ptr, &StylesheetEditor::copy);
  menu->addAction(act);

  act = new QAction(q_ptr->tr("&Paste"), q_ptr);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_V));
  q_ptr->connect(act, &QAction::triggered, q_ptr, &StylesheetEditor::paste);
  menu->addAction(act);

  menu->addSeparator();

  act = new QAction(q_ptr->tr("Select All"), q_ptr);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_A));
  q_ptr->connect(act, &QAction::triggered, q_ptr, &StylesheetEditor::selectAll);
  menu->addAction(act);

  menu->addSeparator();

  menu->addMenu(m_parser->handleMouseClicked(pos));

  menu->addSeparator();

  act = new QAction(q_ptr->tr("&Format"));
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
  act->setStatusTip(q_ptr->tr("Prettyfy the stylesheet"));
  q_ptr->connect(act, &QAction::triggered, q_ptr, &StylesheetEditor::format);
  menu->addAction(act);

  menu->popup(q_ptr->viewport()->mapToGlobal(pos));
}

void
StylesheetEditorPrivate::suggestionMade(QAction* act)
{
  m_parser->handleSuggestions(act);
}

void
StylesheetEditor::documentChanged(int pos, int charsRemoved, int charsAdded)
{
  d_ptr->handleDocumentChanged(pos, charsRemoved, charsAdded);
}

void
StylesheetEditor::handleTextChanged()
{
  d_ptr->handleTextChanged();
}

void
StylesheetEditorPrivate::handleDocumentChanged(int pos,
                                               int charsRemoved,
                                               int charsAdded)
{
  m_parser->handleDocumentChanged(pos, charsRemoved, charsAdded);
}

void
StylesheetEditorPrivate::handleTextChanged()
{
  qWarning();
}

int
StylesheetEditorPrivate::bookmarkLineNumber() const
{
  return m_bookmarkLineNumber;
}

void
StylesheetEditorPrivate::setBookmarkLineNumber(int bookmarkLineNumber)
{
  m_bookmarkLineNumber = bookmarkLineNumber;
}

int
StylesheetEditor::bookmarkAreaWidth()
{
  return d_ptr->bookmarkAreaWidth();
}

int
StylesheetEditorPrivate::bookmarkAreaWidth()
{
  return m_bookmarkArea->bookmarkAreaWidth();
}

int
StylesheetEditor::lineNumberAreaWidth()
{
  return d_ptr->lineNumberAreaWidth();
}

int
StylesheetEditorPrivate::lineNumberAreaWidth()
{
  return m_lineNumberArea->lineNumberAreaWidth();
}

QMap<int, BookmarkData*>*
StylesheetEditor::bookmarks()
{
  return d_ptr->bookmarks();
}

QMap<int, BookmarkData*>*
StylesheetEditorPrivate::bookmarks()
{
  return m_bookmarkArea->bookmarks();
}

void
StylesheetEditor::setBookmarks(QMap<int, BookmarkData*>* bookmarks)
{
  d_ptr->setBookmarks(bookmarks);
}

void
StylesheetEditorPrivate::setBookmarks(QMap<int, BookmarkData*>* bookmarks)
{
  m_bookmarkArea->setBookmarks(bookmarks);
}

void
StylesheetEditor::handleAddBookmark(bool)
{
  d_ptr->insertBookmark();
}

void
StylesheetEditor::insertBookmark(int bookmark, const QString& text)
{
  d_ptr->insertBookmark(bookmark, text);
}

void
StylesheetEditorPrivate::insertBookmark(int bookmark, const QString& text)
{
  int bm = (bookmark >= 0 ? bookmark : m_bookmarkLineNumber);
  m_bookmarkArea->insertBookmark(bm, text);
}

void
StylesheetEditor::toggleBookmark(int bookmark)
{
  d_ptr->toggleBookmark(bookmark);
}

void
StylesheetEditorPrivate::toggleBookmark(int bookmark)
{
  m_bookmarkArea->toggleBookmark(bookmark);
}

void
StylesheetEditor::editBookmark(int bookmark)
{
  d_ptr->editBookmark(bookmark);
}

void
StylesheetEditor::handleEditBookmark(bool)
{
  d_ptr->editBookmark();
}

void
StylesheetEditorPrivate::editBookmark(int bookmark)
{
  int lineNumber = (bookmark >= 0 ? bookmark : m_bookmarkLineNumber);
  m_bookmarkArea->editBookmark(lineNumber);
}

void
StylesheetEditor::removeBookmark(int bookmark)
{
  d_ptr->removeBookmark(bookmark);
}

void
StylesheetEditor::handleRemoveBookmark(bool)
{
  d_ptr->removeBookmark();
}

void
StylesheetEditorPrivate::removeBookmark(int bookmark)
{
  int lineNumber = (bookmark >= 0 ? bookmark : m_bookmarkLineNumber);
  m_bookmarkArea->removeBookmark(lineNumber);
}

void
StylesheetEditor::clearBookmarks()
{
  d_ptr->clearBookmarks();
}

void
StylesheetEditor::handleClearBookmarks(bool)
{
  auto btn = QMessageBox::warning(
    this,
    tr("Clearing Book marks"),
    tr("You are about to remove ALL bookmarks!\nAre you sure?"),
    QMessageBox::Cancel | QMessageBox::Ok,
    QMessageBox::Cancel);

  if (btn == QMessageBox::Ok) {
    d_ptr->clearBookmarks();
  }
}

void
StylesheetEditorPrivate::clearBookmarks()
{
  m_bookmarkArea->clearBookmarks();
}

bool
StylesheetEditor::hasBookmark(int bookmark)
{
  return d_ptr->hasBookmark(bookmark);
}

bool
StylesheetEditorPrivate::hasBookmark(int linenumber)
{
  return m_bookmarkArea->hasBookmark(linenumber);
}

bool
StylesheetEditor::hasBookmarkText(int bookmark)
{
  return d_ptr->hasBookmarkText(bookmark);
}

bool
StylesheetEditorPrivate::hasBookmarkText(int bookmark)
{
  return m_bookmarkArea->hasBookmarkText(bookmark);
}

QString
StylesheetEditor::bookmarkText(int bookmark)
{
  return d_ptr->bookmarkText(bookmark);
}

QString
StylesheetEditorPrivate::bookmarkText(int bookmark)
{
  return m_bookmarkArea->bookmarkText(bookmark);
}

// void
// StylesheetEditor::gotoBookmark(int bookmark)
//{
//  d_ptr->gotoBookmark(bookmark);
//}

void
StylesheetEditor::gotoBookmarkDialog()
{
  handleGotoBookmark();
}

void
StylesheetEditor::handleGotoBookmark()
{
  GoToBookmarkDialog* dlg = new GoToBookmarkDialog(bookmarks(), this);

  if (dlg->exec() == QDialog::Accepted) {
    int bookmark = dlg->bookmark();

    if (bookmark != -1) {
      d_ptr->setLineNumber(bookmark);
    }
  }
}

// void
// StylesheetEditorPrivate::gotoBookmark(int bookmark)
//{
//  if (hasBookmark(bookmark)) {
//    setLineNumber(bookmark);
//  }
//}

// void
// StylesheetEditPrivate::setContextMenu(QMenu* menu)
//{
//  m_contextMenu = menu;
//  m_contextMenu->show();
//}

// void
// StylesheetEditorPrivate::handleContextMenuEvent(QPoint pos)
//{
//  while (true) {
//    if (m_contextMenu) {
//      m_contextMenu->exec(pos);
//      break;
//    }
//    q_ptr->thread()->sleep(100);
//  }
//}

// void
// StylesheetEditorPrivate::handleBookmarkMenuEvent(QPoint pos)
//{
//  auto tc = q_ptr->cursorForPosition(pos);
//  int lineNumber = calculateLineNumber(tc);
//  setBookmarkLineNumber(lineNumber);

//  if (hasBookmark(lineNumber)) {
//    m_addBookmarkAct->setEnabled(false);
//    m_editBookmarkAct->setEnabled(true);
//    m_removeBookmarkAct->setEnabled(true);

//  } else {
//    m_addBookmarkAct->setEnabled(true);
//    m_editBookmarkAct->setEnabled(false);
//    m_removeBookmarkAct->setEnabled(false);
//  }

//  if (bookmarkCount() == 0) {
//    m_clearBookmarksAct->setEnabled(false);
//    m_gotoBookmarkAct->setEnabled(false);

//  } else {
//    m_clearBookmarksAct->setEnabled(true);
//    m_gotoBookmarkAct->setEnabled(true);
//  }

//  m_bookmarkMenu->exec(pos);
//}

QTextCursor
StylesheetEditorPrivate::currentCursor() const
{
  return m_parser->currentCursor();
}

void
StylesheetEditorPrivate::setCurrentCursor(const QTextCursor& currentCursor)
{
  m_parser->setCurrentCursor(currentCursor);
  setLineData(currentCursor);
}

void
StylesheetEditorPrivate::setCurrentLineNumber(const int number)
{
  m_lineNumberArea->setLineNumber(number);
}

int
StylesheetEditor::currentLineNumber() const
{
  return d_ptr->currentLineNumber();
}

int
StylesheetEditorPrivate::currentLineNumber() const
{
  return m_lineNumberArea->currentLineNumber();
}

int
StylesheetEditorPrivate::currentLineCount() const
{
  return m_lineNumberArea->lineCount();
}

void
StylesheetEditor::setLineNumber(int lineNumber)
{
  d_ptr->setLineNumber(lineNumber);
}

void
StylesheetEditorPrivate::setLineNumber(int linenumber)
{
  m_datastore->setManualMove(true);
  QTextCursor cursor(q_ptr->document());
  cursor.movePosition(QTextCursor::Start);
  cursor.movePosition(
    QTextCursor::Down, QTextCursor::MoveAnchor, linenumber - 1);
  q_ptr->setTextCursor(cursor);
  q_ptr->setFocus(Qt::OtherFocusReason);
  setCurrentCursor(cursor);
  //  setLineData(cursor);
  m_lineNumberArea->setLineNumber(linenumber);
  m_datastore->setManualMove(false);
}

void
StylesheetEditor::up(int n)
{
  d_ptr->up(n);
}

void
StylesheetEditorPrivate::setLineData(QTextCursor cursor)
{
  // this handles display of linenumber, linecount and character column.
  int ln = calculateLineNumber(cursor);
  m_lineNumberArea->setLineNumber(ln);
}

void
StylesheetEditorPrivate::up(int n)
{
  m_datastore->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, n);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_datastore->setManualMove(false);
}

void
StylesheetEditor::down(int n)
{
  d_ptr->down(n);
}

void
StylesheetEditorPrivate::down(int n)
{
  m_datastore->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, n);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_datastore->setManualMove(false);
}

void
StylesheetEditor::left(int n)
{
  d_ptr->left(n);
}

void
StylesheetEditorPrivate::left(int n)
{
  m_datastore->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, n);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_datastore->setManualMove(false);
}

void
StylesheetEditor::right(int n)
{
  d_ptr->right(n);
}

void
StylesheetEditorPrivate::right(int n)
{
  m_datastore->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, n);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_datastore->setManualMove(false);
}

void
StylesheetEditor::start()
{
  d_ptr->start();
}

void
StylesheetEditorPrivate::start()
{
  m_datastore->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Start);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_datastore->setManualMove(false);
}

void
StylesheetEditor::end()
{
  d_ptr->end();
}

void
StylesheetEditorPrivate::end()
{
  m_datastore->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::End);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_datastore->setManualMove(false);
}

void
StylesheetEditor::startOfLine()
{
  d_ptr->startOfLine();
}

void
StylesheetEditorPrivate::startOfLine()
{
  m_datastore->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::StartOfLine);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_datastore->setManualMove(false);
}

void
StylesheetEditor::endOfLine()
{
  d_ptr->endOfLine();
}

void
StylesheetEditorPrivate::endOfLine()
{
  m_datastore->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::EndOfLine);
  q_ptr->setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_datastore->setManualMove(false);
}

void
StylesheetEditor::goToLine(int lineNumber)
{
  d_ptr->setLineNumber(lineNumber);
}

int
StylesheetEditor::bookmarkCount()
{
  return d_ptr->bookmarkCount();
}

int
StylesheetEditorPrivate::bookmarkCount()
{
  return m_bookmarkArea->count();
}
