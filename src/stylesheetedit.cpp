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
#include "stylesheetedit/stylesheetedit.h"

#include "bookmarkarea.h"
#include "common.h"
#include "datastore.h"
#include "hoverwidget.h"
#include "linenumberarea.h"
#include "node.h"
#include "parser.h"
#include "qyamlcpp/qyamlcpp.h"
#include "stylesheetedit/stylesheeteditdialog.h"
#include "stylesheetedit_p.h"
#include "stylesheethighlighter.h"

#include <QtDebug>

StylesheetEdit::StylesheetEdit(QWidget* parent)
  : QWidget(parent)
  , m_editor(new StylesheetEditor(new DataStore(), this))
  , m_linenumberArea(new LineNumberArea(m_editor))
  , m_bookmarkArea(new BookmarkArea(m_editor))
{
  qRegisterMetaType<QTextCursor>("QTextCursor");
  qRegisterMetaType<MenuData>("MenuData");
  qRegisterMetaType<NodeType>("NodeType");
  qRegisterMetaType<NodeState>("NodeCheck");
  qRegisterMetaType<QTextCharFormat>("QTextCharFormat");
  qRegisterMetaTypeStreamOperators<ColorDragData>("ColorDragData");

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
StylesheetEdit::loadConfig(const QString& filename)
{
  m_editor->loadYamlConfig(filename);
}

void
StylesheetEdit::saveConfig(const QString& filename)
{
  m_editor->saveYamlConfig(filename);
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
StylesheetEdit::setWidgetFormat(QBrush color,
                                QBrush back,
                                QFont font,
                                QBrush underline,
                                QTextCharFormat::UnderlineStyle style)
{
  m_editor->setWidgetFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setBadWidgetFormat(QBrush color,
                                   QBrush back,
                                   QFont font,
                                   QBrush underline,
                                   QTextCharFormat::UnderlineStyle style)
{
  m_editor->setBadWidgetFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setSeperatorFormat(QBrush color,
                                   QBrush back,
                                   QFont font,
                                   QBrush underline,
                                   QTextCharFormat::UnderlineStyle style)
{
  m_editor->setSeperatorFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setIdSelectorFormat(QBrush color,
                                    QBrush back,
                                    QFont font,
                                    QBrush underline,
                                    QTextCharFormat::UnderlineStyle style)
{
  m_editor->setIdSelectorFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setBadIdSelectorFormat(QBrush color,
                                       QBrush back,
                                       QFont font,
                                       QBrush underline,
                                       QTextCharFormat::UnderlineStyle style)
{
  m_editor->setBadIdSelectorFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setIDSelectorMarkerFormat(QBrush color,
                                          QBrush back,
                                          QFont font,
                                          QBrush underline,
                                          QTextCharFormat::UnderlineStyle style)
{
  m_editor->setIdSelectorMarkerFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setBadIDSelectorMarkerFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_editor->setBadIdSelectorMarkerFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setPseudoStateFormat(QBrush color,
                                     QBrush back,
                                     QFont font,
                                     QBrush underline,
                                     QTextCharFormat::UnderlineStyle style)
{
  m_editor->setPseudoStateFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setBadPseudoStateFormat(QBrush color,
                                        QBrush back,
                                        QFont font,
                                        QBrush underline,
                                        QTextCharFormat::UnderlineStyle style)
{
  m_editor->setBadPseudoStateFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setPseudoStateMarkerFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_editor->setPseudoStateMarkerFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setBadPseudoStateMarkerFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_editor->setBadPseudoStateMarkerFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setSubControlFormat(QBrush color,
                                    QBrush back,
                                    QFont font,
                                    QBrush underline,
                                    QTextCharFormat::UnderlineStyle style)
{
  m_editor->setSubControlFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setBadSubControlFormat(QBrush color,
                                       QBrush back,
                                       QFont font,
                                       QBrush underline,
                                       QTextCharFormat::UnderlineStyle style)
{
  m_editor->setBadSubControlFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setSubControlMarkerFormat(QBrush color,
                                          QBrush back,
                                          QFont font,
                                          QBrush underline,
                                          QTextCharFormat::UnderlineStyle style)
{
  m_editor->setSubControlMarkerFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setBadSubControlMarkerFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_editor->setBadSubControlMarkerFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setPropertyValueFormat(QBrush color,
                                       QBrush back,
                                       QFont font,
                                       QBrush underline,
                                       QTextCharFormat::UnderlineStyle style)
{
  m_editor->setPropertyValueFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setBadPropertyValueFormat(QBrush color,
                                          QBrush back,
                                          QFont font,
                                          QBrush underline,
                                          QTextCharFormat::UnderlineStyle style)
{
  m_editor->setBadPropertyValueFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setPropertyFormat(QBrush color,
                                  QBrush back,
                                  QFont font,
                                  QBrush underline,
                                  QTextCharFormat::UnderlineStyle style)
{
  m_editor->setPropertyFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setBadPropertyFormat(QBrush color,
                                     QBrush back,
                                     QFont font,
                                     QBrush underline,
                                     QTextCharFormat::UnderlineStyle style)
{
  m_editor->setBadPropertyFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setPropertyMarkerFormat(QBrush color,
                                        QBrush back,
                                        QFont font,
                                        QBrush underline,
                                        QTextCharFormat::UnderlineStyle style)
{
  m_editor->setPropertyMarkerFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setPropertyEndMarkerFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_editor->setPropertyEndMarkerFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setLineNumberFormat(QBrush color, QBrush back, QFont font)
{
  m_editor->setLineNumberFormat(color, back, font);
}

void
StylesheetEdit::setStartBraceFormat(QBrush color,
                                    QBrush back,
                                    QFont font,
                                    QBrush underline,
                                    QTextCharFormat::UnderlineStyle style)
{
  m_editor->setStartBraceFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setBadStartBraceFormat(QBrush color,
                                       QBrush back,
                                       QFont font,
                                       QBrush underline,
                                       QTextCharFormat::UnderlineStyle style)
{
  m_editor->setBadStartBraceFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setEndBraceFormat(QBrush color,
                                  QBrush back,
                                  QFont font,
                                  QBrush underline,
                                  QTextCharFormat::UnderlineStyle style)
{
  m_editor->setEndBraceFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setBadEndBraceFormat(QBrush color,
                                     QBrush back,
                                     QFont font,
                                     QBrush underline,
                                     QTextCharFormat::UnderlineStyle style)
{
  m_editor->setBadEndBraceFormat(color, back, font, underline, style);
}

void
StylesheetEdit::setBraceMatchFormat(QBrush color,
                                    QBrush back,
                                    QFont font,
                                    QBrush underline,
                                    QTextCharFormat::UnderlineStyle style)
{
  m_editor->setBraceMatchFormat(color, back, font, underline, style);
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
StylesheetEdit::addCustomWidget(const QString& name, const QString& parent)
{
  return m_editor->addCustomWidget(name, parent);
}

QStringList
StylesheetEdit::widgets()
{
  return m_editor->widgets();
}

bool
StylesheetEdit::addCustomWidgetPseudoStates(const QString& name,
                                            const QStringList& states)
{
  return m_editor->addCustomWidgetSubControls(name, states);
}

bool
StylesheetEdit::addCustomWidgetSubControls(const QString& name,
                                           const QStringList& controls)
{
  return m_editor->addCustomWidgetSubControls(name, controls);
}

bool
StylesheetEdit::addCustomWidgetProperties(const QString& name,
                                          const QStringList& properties)
{
  return m_editor->addCustomWidgetProperties(name, properties);
}

bool
StylesheetEdit::addCustomWidgetPropertyValue(const QString& widget,
                                             const QString& property,
                                             const QString& value)
{
  return m_editor->addCustomWidgetPropertyValue(widget, property, value);
}

bool
StylesheetEdit::addCustomWidgetPropertyValues(const QString& widget,
                                              const QString& property,
                                              QStringList values)
{
  return m_editor->addCustomWidgetPropertyValues(widget, property, values);
}

bool
StylesheetEditor::checkForEmpty(const QString& text)
{
  if (text.isEmpty() || text.trimmed().isEmpty()) {
    return true;
  }
  return false;
}

//=== StylesheetEditor
//================================================================

const QChar StylesheetEditor::m_arrow = QChar(0x2BC8);
const QString StylesheetEditor::GOOD = "good";
const QString StylesheetEditor::BAD = "bad";
const QString StylesheetEditor::MARKER = "marker";
const QString StylesheetEditor::ENDMARKER = "endmarker";
const QString StylesheetEditor::FORE = "foreground";
const QString StylesheetEditor::BACK = "background";
const QString StylesheetEditor::FONT = "font";
const QString StylesheetEditor::COLOR = "underlinecolor";
const QString StylesheetEditor::STYLE = "underlinestyle";
const QString StylesheetEditor::WIDGET = "widget";
const QString StylesheetEditor::SEPERATOR = "seperator";
const QString StylesheetEditor::IDSELECTOR = "idselector";
const QString StylesheetEditor::PSEUDOSTATE = "pseudostate";
const QString StylesheetEditor::SUBCONTROL = "subcontrol";
const QString StylesheetEditor::PROPERTY = "property";
const QString StylesheetEditor::VALUE = "value";
const QString StylesheetEditor::BRACE = "brace";
const QString StylesheetEditor::START = "start";
const QString StylesheetEditor::END = "end";
const QString StylesheetEditor::MATCH = "match";
const QString StylesheetEditor::COMMENT = "comment";

// void StylesheetEditor::setupConfiguration()
//{
//  m_configDir =
//    QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
//  m_configDir += "/QtProject/qtcreator/stylesheetedit";
//  QDir dir;
//  dir.mkpath(m_configDir);
//  m_configFile = "stylesheetedit.yaml";
//}

StylesheetEditor::StylesheetEditor(DataStore* datastore, QWidget* parent)
  : QPlainTextEdit(parent)
  , m_bookmarkArea(nullptr)
  , m_lineNumberArea(nullptr)
  , m_datastore(datastore)
  , m_parser(new Parser(m_datastore, this))
  , m_highlighter(new StylesheetHighlighter(this, m_datastore))
  , m_parseComplete(false)
  , m_oldSection(new NodeSection(this))
{
  m_datastore->setEditor(this);
  setFont(QFont("Source Code Pro", 9));
  setMouseTracking(true);
  loadYamlConfig();
}

StylesheetHighlighter*
StylesheetEditor::highlighter()
{
  return m_highlighter;
}

void
StylesheetEditor::setup(BookmarkArea* bookmarkArea,
                        LineNumberArea* linenumberArea)
{
  m_bookmarkArea = bookmarkArea;
  m_lineNumberArea = linenumberArea;

  connect(m_parser,
          &Parser::parseComplete,
          this,
          &StylesheetEditor::handleParseComplete);
  connect(
    m_parser, &Parser::rehighlight, this, &StylesheetEditor::handleRehighlight);
  connect(m_parser,
          &Parser::rehighlightBlock,
          this,
          &StylesheetEditor::handleRehighlightBlock);

  connect(this,
          &QPlainTextEdit::cursorPositionChanged,
          this,
          &StylesheetEditor::updateLineNumberArea);
  connect(this,
          &QPlainTextEdit::cursorPositionChanged,
          this,
          &StylesheetEditor::cursorPositionHasChanged);
  //  connect(document(),
  //          &QTextDocument::contentsChange,
  //          this,
  //          &StylesheetEditor::documentChanged,
  //          Qt::UniqueConnection);
}

void
StylesheetEditor::writeFormat(YAML::Emitter* emitter, QTextCharFormat format)
{
  auto fore = format.foreground();
  auto back = format.background();
  auto font = format.font();
  auto underlineColor = format.underlineColor();
  auto underlineStyle = format.underlineStyle();
  *emitter << YAML::BeginMap;
  *emitter << YAML::Key << FORE;
  *emitter << YAML::Value << fore;
  *emitter << YAML::Key << BACK;
  *emitter << YAML::Value << back;
  *emitter << YAML::Key << FONT;
  *emitter << YAML::Value << font;
  *emitter << YAML::Key << COLOR;
  *emitter << YAML::Value << underlineColor;
  *emitter << YAML::Key << STYLE;
  *emitter << YAML::Value << underlineStyle;
  *emitter << YAML::EndMap;
}

void
StylesheetEditor::saveYamlConfig(const QString& filename)
{
  QFile* file;
  if (filename.isEmpty())
    file = new QFile(
      QDir(m_datastore->configDir()).filePath(m_datastore->configFile()));
  else
    file = new QFile(filename);

  if (file->open((QFile::ReadWrite | QFile::Truncate))) {
    YAML::Emitter emitter;

    //    emitter << YAML::Key << STYLESHEETEDIT;
    { // Start of stylesheet.
      QVariant v;
      QTextCharFormat format;
      emitter << YAML::BeginMap;
      { // Start of WIDGET
        emitter << YAML::Key << WIDGET;
        emitter << YAML::BeginMap;
        {
          emitter << YAML::Key << GOOD;
          format = m_highlighter->widgetFormat();
          writeFormat(&emitter, format);
          emitter << YAML::Key << BAD;
          format = m_highlighter->badWidgetFormat();
          writeFormat(&emitter, format);
        }
        emitter << YAML::EndMap;
      } // End of WIDGET
      { // Start of SEPERATOR
        emitter << YAML::Key << SEPERATOR;
        format = m_highlighter->seperatorFormat();
        writeFormat(&emitter, format);
      } // End of SEPERATOR
      { // Start of IDSELECTOR
        emitter << YAML::Key << IDSELECTOR;
        emitter << YAML::BeginMap;
        {
          emitter << YAML::Key << GOOD;
          format = m_highlighter->idSelectorFormat();
          writeFormat(&emitter, format);
          emitter << YAML::Key << BAD;
          format = m_highlighter->badIdSelectorFormat();
          writeFormat(&emitter, format);
          emitter << YAML::Key << MARKER;
          {
            emitter << YAML::BeginMap;
            emitter << YAML::Key << GOOD;
            format = m_highlighter->idSelectorMarkerFormat();
            writeFormat(&emitter, format);
            emitter << YAML::Key << BAD;
            format = m_highlighter->badIdSelectorMarkerFormat();
            writeFormat(&emitter, format);
            emitter << YAML::EndMap;
          }
        }
        emitter << YAML::EndMap;
      } // End of IDSELECTOR
      { // Start of PSEUDOSTATE
        emitter << YAML::Key << PSEUDOSTATE;
        emitter << YAML::BeginMap;
        {
          emitter << YAML::Key << GOOD;
          format = m_highlighter->pseudoStateFormat();
          writeFormat(&emitter, format);
          emitter << YAML::Key << BAD;
          format = m_highlighter->badPseudoStateFormat();
          writeFormat(&emitter, format);
          emitter << YAML::Key << MARKER;
          {
            emitter << YAML::BeginMap;
            emitter << YAML::Key << GOOD;
            format = m_highlighter->pseudoStateMarkerFormat();
            writeFormat(&emitter, format);
            emitter << YAML::Key << BAD;
            format = m_highlighter->badPseudoStateMarkerFormat();
            writeFormat(&emitter, format);
            emitter << YAML::EndMap;
          }
        }
        emitter << YAML::EndMap;
      } // End of PSEUDOSTATE
      { // Start of SUBCONTROL
        emitter << YAML::Key << SUBCONTROL;
        emitter << YAML::BeginMap;
        {
          emitter << YAML::Key << GOOD;
          format = m_highlighter->subControlFormat();
          writeFormat(&emitter, format);
          emitter << YAML::Key << BAD;
          format = m_highlighter->badSubControlFormat();
          writeFormat(&emitter, format);
          emitter << YAML::Key << MARKER;
          {
            emitter << YAML::BeginMap;
            emitter << YAML::Key << GOOD;
            format = m_highlighter->subControlMarkerFormat();
            writeFormat(&emitter, format);
            emitter << YAML::Key << BAD;
            format = m_highlighter->badSubControlMarkerFormat();
            writeFormat(&emitter, format);
            emitter << YAML::EndMap;
          }
        }
        emitter << YAML::EndMap;
      } // End of PSEUDOSTATE
      { // Start of PROPERTY
        emitter << YAML::Key << PROPERTY;
        emitter << YAML::BeginMap;
        {
          emitter << YAML::Key << GOOD;
          format = m_highlighter->propertyFormat();
          writeFormat(&emitter, format);
          emitter << YAML::Key << BAD;
          format = m_highlighter->badPropertyFormat();
          writeFormat(&emitter, format);
          emitter << YAML::Key << MARKER;
          format = m_highlighter->propertyMarkerFormat();
          writeFormat(&emitter, format);
          emitter << YAML::Key << PROPERTY;
          {
            emitter << YAML::BeginMap;
            emitter << YAML::Key << GOOD;
            format = m_highlighter->propertyValueFormat();
            writeFormat(&emitter, format);
            emitter << YAML::Key << BAD;
            format = m_highlighter->badPropertyValueFormat();
            writeFormat(&emitter, format);
            emitter << YAML::EndMap;
          }
          emitter << YAML::Key << ENDMARKER;
          format = m_highlighter->propertyEndMarkerFormat();
          writeFormat(&emitter, format);
        }
        emitter << YAML::EndMap;
      } // End of PROPERTY
      { // Start of BRACES
        emitter << YAML::Key << "brace";
        emitter << YAML::BeginMap;
        { // Start of START BRACE
          emitter << YAML::Key << "start";
          emitter << YAML::BeginMap;
          {
            emitter << YAML::Key << GOOD;
            format = m_highlighter->startBraceFormat();
            writeFormat(&emitter, format);
            emitter << YAML::Key << BAD;
            format = m_highlighter->badStartBraceFormat();
            writeFormat(&emitter, format);
          }
          emitter << YAML::EndMap;
        } // End of START BRACE
        { // Start of END BRACE
          emitter << YAML::Key << "end";
          emitter << YAML::BeginMap;
          {
            emitter << YAML::Key << GOOD;
            format = m_highlighter->endBraceFormat();
            writeFormat(&emitter, format);
            emitter << YAML::Key << BAD;
            format = m_highlighter->badEndBraceFormat();
            writeFormat(&emitter, format);
          }
          emitter << YAML::EndMap;
        } // End of END BRACE
        { // Start of BRACE MATCH
          emitter << YAML::Key << "match";
          emitter << YAML::BeginMap;
          {
            emitter << YAML::Key << GOOD;
            format = m_highlighter->braceMatchFormat();
            writeFormat(&emitter, format);
            emitter << YAML::Key << BAD;
            format = m_highlighter->badBraceMatchFormat();
            writeFormat(&emitter, format);
          }
          emitter << YAML::EndMap;
        } // End of BRACE MATCH
        emitter << YAML::EndMap;
      } // End of BRACES
      { // Start of COMMENTS
        emitter << YAML::Key << "comment";
        format = m_highlighter->propertyMarkerFormat();
        writeFormat(&emitter, format);
      } // End of COMMENTS
      emitter << YAML::EndMap;
    } // End of stylesheetedit

    QTextStream out(file);
    out << emitter.c_str();
    file->close();
  }
}

void
StylesheetEditor::readFormat(YAML::Node* subnode1, QTextCharFormat** format)
{
  YAML::Node subnode2;

  if ((*subnode1)[FORE]) {
    subnode2 = (*subnode1)[FORE];
    try {
      (*format)->setForeground(subnode2.as<QBrush>());
    } catch (const std::exception& e) {
      errorMsg(e.what());
    }
  }
  if ((*subnode1)[BACK]) {
    subnode2 = (*subnode1)[BACK];
    try {
      (*format)->setBackground(subnode2.as<QBrush>());
    } catch (const std::exception& e) {
      errorMsg(e.what());
    }
  }
  if ((*subnode1)[FONT]) {
    subnode2 = (*subnode1)[FONT];
    try {
      (*format)->setFont(subnode2.as<QFont>());
    } catch (const std::exception& e) {
      errorMsg(e.what());
    }
  }
  if ((*subnode1)[COLOR]) {
    subnode2 = (*subnode1)[COLOR];
    try {
      (*format)->setUnderlineColor(subnode2.as<QColor>());
    } catch (const std::exception& e) {
      errorMsg(e.what());
    }
  }
  if ((*subnode1)[STYLE]) {
    try {
      (*format)->setUnderlineStyle(
        QTextCharFormat::UnderlineStyle((*subnode1)[STYLE].as<int>()));
    } catch (const std::exception& e) {
      errorMsg(e.what());
    }
  }
}

void
StylesheetEditor::errorMsg(const char* text)
{
  qWarning() << "Yaml parsing error : " << text;
}

void
StylesheetEditor::loadYamlConfig(const QString& filename)
{
  QFile* file;
  if (filename.isEmpty())
    file = new QFile(
      QDir(m_datastore->configDir()).filePath(m_datastore->configFile()));
  else
    file = new QFile(filename);

  if (file->exists()) {
    auto config = YAML::LoadFile(*file);
    YAML::Node stylesheetedit, node, subnode1, subnode2;
    QTextCharFormat format;
    QTextCharFormat* formatPtr = &format;
    if (config[WIDGET]) {
      node = config[WIDGET];
      if (node[GOOD]) {
        subnode1 = node[GOOD];
        readFormat(&subnode1, &formatPtr);
        m_highlighter->setWidgetFormat(*formatPtr);
      }
      if (node[BAD]) {
        subnode1 = node[BAD];
        readFormat(&subnode1, &formatPtr);
        m_highlighter->setBadWidgetFormat(*formatPtr);
      }
    }
    if (config[SEPERATOR]) {
      node = config[SEPERATOR];
      readFormat(&node, &formatPtr);
      m_highlighter->setSeperatorFormat(*formatPtr);
    }
    if (config[IDSELECTOR]) {
      node = config[IDSELECTOR];
      if (node[GOOD]) {
        subnode1 = node[GOOD];
        readFormat(&subnode1, &formatPtr);
        m_highlighter->setIdSelectorFormat(*formatPtr);
      }
      if (node[BAD]) {
        subnode1 = node[BAD];
        readFormat(&subnode1, &formatPtr);
        m_highlighter->setBadIdSelectorFormat(*formatPtr);
      }
      if (node[MARKER]) {
        subnode1 = node[MARKER];
        if (subnode1[GOOD]) {
          subnode2 = subnode1[GOOD];
          readFormat(&subnode2, &formatPtr);
          m_highlighter->setIdSelectorMarkerFormat(*formatPtr);
        }
        if (subnode1[BAD]) {
          subnode2 = subnode1[BAD];
          readFormat(&subnode2, &formatPtr);
          m_highlighter->setBadIdSelectorMarkerFormat(*formatPtr);
        }
      }
    }
    if (config[PSEUDOSTATE]) {
      node = config[PSEUDOSTATE];
      if (node[GOOD]) {
        subnode1 = node[GOOD];
        readFormat(&subnode1, &formatPtr);
        m_highlighter->setPseudoStateFormat(*formatPtr);
      }
      if (node[BAD]) {
        subnode1 = node[BAD];
        readFormat(&subnode1, &formatPtr);
        m_highlighter->setBadPseudoStateFormat(*formatPtr);
      }
      if (node[MARKER]) {
        subnode1 = node[MARKER];
        if (subnode1[GOOD]) {
          subnode2 = subnode1[GOOD];
          readFormat(&subnode2, &formatPtr);
          m_highlighter->setPseudoStateMarkerFormat(*formatPtr);
        }
        if (subnode1[BAD]) {
          subnode2 = subnode1[BAD];
          readFormat(&subnode2, &formatPtr);
          m_highlighter->setBadPseudoStateMarkerFormat(*formatPtr);
        }
      }
    }
    if (config[SUBCONTROL]) {
      node = config[SUBCONTROL];
      if (node[GOOD]) {
        subnode1 = node[GOOD];
        readFormat(&subnode1, &formatPtr);
        m_highlighter->setSubControlFormat(*formatPtr);
      }
      if (node[BAD]) {
        subnode1 = node[BAD];
        readFormat(&subnode1, &formatPtr);
        m_highlighter->setBadSubControlFormat(*formatPtr);
      }
      if (node[MARKER]) {
        subnode1 = node[MARKER];
        if ((subnode2 = subnode1[GOOD])) {
          readFormat(&subnode2, &formatPtr);
          m_highlighter->setSubControlMarkerFormat(*formatPtr);
        }
        if (subnode1[BAD]) {
          subnode2 = subnode1[BAD];
          readFormat(&subnode2, &formatPtr);
          m_highlighter->setBadSubControlMarkerFormat(*formatPtr);
        }
      }
    }
    if (config[PROPERTY]) {
      node = config[PROPERTY];
      if (node[GOOD]) {
        subnode1 = node[GOOD];
        readFormat(&subnode1, &formatPtr);
        m_highlighter->setPropertyFormat(*formatPtr);
      }
      if (node[BAD]) {
        subnode1 = node[BAD];
        readFormat(&subnode1, &formatPtr);
        m_highlighter->setBadPropertyFormat(*formatPtr);
      }
      if (node[MARKER]) {
        subnode1 = node[MARKER];
        readFormat(&subnode1, &formatPtr);
        m_highlighter->setPropertyMarkerFormat(*formatPtr);
      }
      if (node[VALUE]) {
        subnode1 = node[VALUE];
        if (subnode1[GOOD]) {
          subnode2 = subnode1[GOOD];
          readFormat(&subnode2, &formatPtr);
          m_highlighter->setPropertyValueFormat(*formatPtr);
        }
        if (subnode1[BAD]) {
          subnode2 = subnode1[BAD];
          readFormat(&subnode2, &formatPtr);
          m_highlighter->setBadPropertyValueFormat(*formatPtr);
        }
      }
      if (node[ENDMARKER]) {
        subnode1 = node[ENDMARKER];
        readFormat(&subnode1, &formatPtr);
        m_highlighter->setPropertyEndMarkerFormat(*formatPtr);
      }
    }
    if (config[BRACE]) {
      node = config[BRACE];
      if (node[START]) {
        subnode1 = node[START];
        if (node[GOOD]) {
          subnode1 = node[GOOD];
          readFormat(&subnode2, &formatPtr);
          m_highlighter->setStartBraceFormat(*formatPtr);
        }
        if (node[BAD]) {
          subnode1 = node[BAD];
          readFormat(&subnode2, &formatPtr);
          m_highlighter->setBadStartBraceFormat(*formatPtr);
        }
      }
      if (node[END]) {
        subnode1 = node[END];
        if (subnode1[GOOD]) {
          subnode2 = subnode1[GOOD];
          readFormat(&subnode2, &formatPtr);
          m_highlighter->setEndBraceFormat(*formatPtr);
        }
        if (subnode1[BAD]) {
          subnode2 = subnode1[BAD];
          readFormat(&subnode2, &formatPtr);
          m_highlighter->setBadEndBraceFormat(*formatPtr);
        }
      }
      if (node[MATCH]) {
        subnode1 = node[MATCH];
        if (node[GOOD]) {
          subnode2 = node[GOOD];
          readFormat(&subnode2, &formatPtr);
          m_highlighter->setBraceMatchFormat(*formatPtr);
        }
        if (node[BAD]) {
          subnode2 = node[BAD];
          readFormat(&subnode2, &formatPtr);
          m_highlighter->setBadBraceMatchFormat(*formatPtr);
        }
      }
    }
    if (config[COMMENT]) {
      node = config[COMMENT];
      readFormat(&node, &formatPtr);
      m_highlighter->setCommentFormat(*formatPtr);
    }
  }
}

// bool
// StylesheetEditor::saveXmlConfig(const QString& filename)
//{
//  QFile file(filename);
//  QMap<QString, QList<QString>> names;
//  if (file.exists()) {
//    // TODO OVERWRITE ??
//  } else {
//    if (file.open(QFile::ReadOnly | QFile::Text)) {
//      QXmlStreamWriter writer(&file);
//      writer.writeStartDocument();
//      writer.writeStartElement(QStringLiteral("style-scheme"));
//      writer.writeAttribute("version", "1.0");
//      writer.writeAttribute("name", name);
//      //          writer.writeAttribute(XbelReader::versionAttribute(),
//      //          QStringLiteral("1.0")); for (int i = 0; i <
//      //          treeWidget->topLevelItemCount(); ++i)
//      //              writeItem(treeWidget->topLevelItem(i));

//      writer.writeEndDocument();
//      return true;
//    }
//  }
//  return false;
//}

// void
// StylesheetEditor::loadXmlConfig(const QString& filename)
//{}

int
StylesheetEditor::lineCount()
{
  return m_lineNumberArea->lineCount();
}

bool
StylesheetEditor::addCustomWidget(const QString& name, const QString& parent)
{
  return m_datastore->addCustomWidget(name, parent);
}

QStringList
StylesheetEditor::widgets()
{
  return m_datastore->widgets();
}

bool
StylesheetEditor::addCustomWidgetPseudoStates(const QString& name,
                                              const QStringList& states)
{
  return m_datastore->addCustomWidgetPseudoStates(name, states);
}

bool
StylesheetEditor::addCustomWidgetSubControls(const QString& name,
                                             const QStringList& controls)
{
  return m_datastore->addCustomWidgetSubControls(name, controls);
}

bool
StylesheetEditor::addCustomWidgetProperties(const QString& name,
                                            const QStringList& properties)
{
  return m_datastore->addCustomWidgetProperties(name, properties);
}

bool
StylesheetEditor::addCustomWidgetPropertyValue(const QString& widget,
                                               const QString& property,
                                               const QString& value)
{
  return m_datastore->addCustomWidgetPropertyValue(widget, property, value);
}

bool
StylesheetEditor::addCustomWidgetPropertyValues(const QString& widget,
                                                const QString& property,
                                                QStringList values)
{
  return m_datastore->addCustomWidgetPropertyValues(widget, property, values);
}

void
StylesheetEditor::setPlainText(const QString& text)
{
  QPlainTextEdit::setPlainText(text);
  calculateLineNumber(textCursor());
  m_parser->parseInitialText(text);
  connect(document(),
          &QTextDocument::contentsChange,
          this,
          &StylesheetEditor::documentChanged,
          Qt::UniqueConnection);
  handleRehighlight();
}

void
StylesheetEditor::handleRehighlight()
{
  m_highlighter->rehighlight();
}

void
StylesheetEditor::handleRehighlightBlock(const QTextBlock& block)
{
  m_highlighter->rehighlightBlock(block);
}

DataStore*
StylesheetEditor::datastore()
{
  return m_datastore;
}

void
StylesheetEditor::handleParseComplete()
{
  m_highlighter->rehighlight();
  setLineNumber(1);
  m_parseComplete = true;
  emit lineNumber(currentLineNumber());
  emit lineCount(m_lineNumberArea->lineCount());
}

void
StylesheetEditor::setShowNewlineMarkers(bool show)
{
  m_parser->setShowLineMarkers(show);
}

bool
StylesheetEditor::showNewlineMarkers()
{
  return m_parser->showLineMarkers();
}

// void
// StylesheetEditorPrivate::setShowNewlineMarkers(bool show)
//{
//  m_parser->setShowLineMarkers(show);
//}

// bool
// StylesheetEditorPrivate::showLineMarkers()
//{
//  return m_parser->showLineMarkers();
//}

QString
StylesheetEditor::styleSheet() const
{
  return m_stylesheet;
}

// QString
// StylesheetEditorPrivate::styleSheet() const
//{
//  return m_stylesheet;
//}

bool
StylesheetEditor::checkStylesheetColors(StylesheetData* data,
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
    for (auto& match : matcher.capturedTexts()) {
      re.setPattern(reBetweenCurly);
      matcher = re.match(match);

      // matches curly braces
      if (matcher.hasMatch()) {
        match = matcher.captured(0);
        // remove curly braces.
        sheet = match.mid(1, match.length() - 2).toLower();

        int pos = 0;
        StylesheetData* data = nullptr;
        //        QColor color1, color2, color3;
        //        QTextCharFormat::UnderlineStyle underlineStyle =
        //          QTextCharFormat::NoUnderline;
        //        QFont::Weight fontWeight = QFont::Light;
        //        bool colorError = false;

        //        while (pos < sheet.length()) {
        //          data = m_parser->getStylesheetProperty(sheet, pos);

        //          if (data->name.isEmpty()) {
        //            return;
        //          }

        //          //          color1 = (data->colors.size() > 0 ?
        //          data->colors.at(0) :
        //          //          QColor()); color2 = (data->colors.size() > 1 ?
        //          //          data->colors.at(1) : QColor()); color3 =
        //          //          (data->colors.size() > 2 ? data->colors.at(1) :
        //          //          QColor()); colorError =
        //          checkStylesheetColors(data,
        //          //          color1, color2, color3); underlineStyle =
        //          //            (data->underline.size() > 0 ?
        //          data->underline.at(0)
        //          //                                        :
        //          // QTextCharFormat::NoUnderline);
        //          //          fontWeight =
        //          //            (data->weights.size() > 0 ?
        //          data->weights.at(0) :
        //          //            QFont::Normal);
        //          //          // TODO ignore any more than 3 colours or one
        //          weight or
        //          //          underline
        //          //          // type,
        //          //          // TODO maybe drop earlier values.
        //          //          QFont font("");

        //          //          if (data->name == WIDGET) {
        //          //            m_highlighter->setWidgetFormat(color1, color2,
        //          //            fontWeight); continue;

        //          //          } else if (data->name == PSEUDOSTATE) {
        //          //            m_highlighter->setSubControlFormat(color1,
        //          color2,
        //          //            fontWeight); continue;

        //          //          } else if (data->name == "subcontrolmarker") {
        //          //            m_highlighter->setSubControlMarkerFormat(
        //          //              color1, color2, fontWeight);
        //          //            continue;

        //          //          } else if (data->name == PSEUDOSTATE) {
        //          //            m_highlighter->setPseudoStateFormat(color1,
        //          color2,
        //          //            fontWeight); continue;

        //          //          } else if (data->name == "pseudostatemarker") {
        //          //            m_highlighter->setPseudoStateMarkerFormat(
        //          //              color1, color2, fontWeight);
        //          //            continue;

        //          //          } else if (data->name == PROPERTY) {
        //          //            m_highlighter->setPropertyFormat(color1,
        //          color2,
        //          //            fontWeight); continue;

        //          //          } else if (data->name == "propertymarker") {
        //          //            m_highlighter->setPropertyMarkerFormat(color1,
        //          color2,
        //          //            fontWeight); continue;

        //          //          } else if (data->name == PROPERTY) {
        //          //            m_highlighter->setValueFormat(color1, color2,
        //          //            fontWeight); continue;

        //          //          } else if (data->name == "startbrace") {
        //          //            m_highlighter->setStartBraceFormat(color1,
        //          color2,
        //          //            fontWeight); continue;

        //          //          } else if (data->name == "endbrace") {
        //          //            m_highlighter->setEndBraceFormat(color1,
        //          color2,
        //          //            fontWeight); continue;

        //          //          } else if (data->name == "bracematch") {
        //          //            m_highlighter->setBraceMatchFormat(color1,
        //          color2,
        //          //            fontWeight); continue;

        //          //          } else if (data->name == "comment") {
        //          //            m_highlighter->setCommentFormat(color1,
        //          color2,
        //          //            fontWeight); continue;

        //          //          } /* else if (data->name == BAD) {

        //          //             m_highlighter->setBadValueFormat(
        //          //               color1, color2, fontWeight, true,
        //          underlineStyle,
        //          //               color3);
        //          //             continue;
        //          //           }*/
        //        }
      }
    }
  }

  m_highlighter->rehighlight();
}

void
StylesheetEditor::setPropertyValueFormat(QBrush color,
                                         QBrush back,
                                         QFont font,
                                         QBrush underline,
                                         QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setPropertyValueFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setPropertyValueFormat(QTextCharFormat format)
{
  m_highlighter->setPropertyValueFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadPropertyValueFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setBadPropertyValueFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadPropertyValueFormat(QTextCharFormat format)
{
  m_highlighter->setBadPropertyValueFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setWidgetFormat(QBrush color,
                                  QBrush back,
                                  QFont font,
                                  QBrush underline,
                                  QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setWidgetFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setWidgetFormat(QTextCharFormat format)
{
  m_highlighter->setWidgetFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadWidgetFormat(QBrush color,
                                     QBrush back,
                                     QFont font,
                                     QBrush underline,
                                     QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setBadWidgetFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadWidgetFormat(QTextCharFormat format)
{
  m_highlighter->setBadWidgetFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setSeperatorFormat(QBrush color,
                                     QBrush back,
                                     QFont font,
                                     QBrush underline,
                                     QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setSeperatorFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setSeperatorFormat(QTextCharFormat format)
{
  m_highlighter->setSeperatorFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setIdSelectorFormat(QBrush color,
                                      QBrush back,
                                      QFont font,
                                      QBrush underline,
                                      QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setIdSelectorFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setIdSelectorFormat(QTextCharFormat format)
{
  m_highlighter->setIdSelectorFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadIdSelectorFormat(QBrush color,
                                         QBrush back,
                                         QFont font,
                                         QBrush underline,
                                         QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setBadIdSelectorFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadIdSelectorFormat(QTextCharFormat format)
{
  m_highlighter->setBadIdSelectorFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setIdSelectorMarkerFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setIdSelectorMarkerFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setIdSelectorMarkerFormat(QTextCharFormat format)
{
  m_highlighter->setIdSelectorMarkerFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadIdSelectorMarkerFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setBadIdSelectorMarkerFormat(
    color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadIdSelectorMarkerFormat(QTextCharFormat format)
{
  m_highlighter->setBadIdSelectorMarkerFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setPseudoStateFormat(QBrush color,
                                       QBrush back,
                                       QFont font,
                                       QBrush underline,
                                       QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setPseudoStateFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setPseudoStateFormat(QTextCharFormat format)
{
  m_highlighter->setPseudoStateFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadPseudoStateFormat(QBrush color,
                                          QBrush back,
                                          QFont font,
                                          QBrush underline,
                                          QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setBadPseudoStateFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadPseudoStateFormat(QTextCharFormat format)
{
  m_highlighter->setBadPseudoStateFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setPseudoStateMarkerFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setPseudoStateMarkerFormat(
    color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setPseudoStateMarkerFormat(QTextCharFormat format)
{
  m_highlighter->setPseudoStateMarkerFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadPseudoStateMarkerFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setBadPseudoStateMarkerFormat(
    color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadPseudoStateMarkerFormat(QTextCharFormat format)
{
  m_highlighter->setBadPseudoStateMarkerFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setSubControlFormat(QBrush color,
                                      QBrush back,
                                      QFont font,
                                      QBrush underline,
                                      QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setSubControlFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setSubControlFormat(QTextCharFormat format)
{
  m_highlighter->setSubControlFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadSubControlFormat(QBrush color,
                                         QBrush back,
                                         QFont font,
                                         QBrush underline,
                                         QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setBadSubControlFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadSubControlFormat(QTextCharFormat format)
{
  m_highlighter->setBadSubControlFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setSubControlMarkerFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setSubControlMarkerFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setSubControlMarkerFormat(QTextCharFormat format)
{
  m_highlighter->setSubControlMarkerFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadSubControlMarkerFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setBadSubControlMarkerFormat(
    color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadSubControlMarkerFormat(QTextCharFormat format)
{
  m_highlighter->setBadSubControlMarkerFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setPropertyFormat(QBrush color,
                                    QBrush back,
                                    QFont font,
                                    QBrush underline,
                                    QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setPropertyFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setPropertyFormat(QTextCharFormat format)
{
  m_highlighter->setPropertyFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadPropertyFormat(QBrush color,
                                       QBrush back,
                                       QFont font,
                                       QBrush underline,
                                       QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setBadPropertyFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadPropertyFormat(QTextCharFormat format)
{
  m_highlighter->setBadPropertyFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setPropertyMarkerFormat(QBrush color,
                                          QBrush back,
                                          QFont font,
                                          QBrush underline,
                                          QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setPropertyMarkerFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setPropertyMarkerFormat(QTextCharFormat format)
{
  m_highlighter->setPropertyMarkerFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setPropertyEndMarkerFormat(
  QBrush color,
  QBrush back,
  QFont font,
  QBrush underline,
  QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setPropertyMarkerFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setPropertyEndMarkerFormat(QTextCharFormat format)
{
  m_highlighter->setPropertyEndMarkerFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setLineNumberFormat(QBrush color, QBrush back, QFont font)
{
  m_lineNumberArea->setForeSelected(color);
  m_lineNumberArea->setBack(back);
  m_lineNumberArea->setFont(font);
}

void
StylesheetEditor::setStartBraceFormat(QBrush color,
                                      QBrush back,
                                      QFont font,
                                      QBrush underline,
                                      QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setStartBraceFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setStartBraceFormat(QTextCharFormat format)
{
  m_highlighter->setStartBraceFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadStartBraceFormat(QBrush color,
                                         QBrush back,
                                         QFont font,
                                         QBrush underline,
                                         QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setBadStartBraceFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadStartBraceFormat(QTextCharFormat format)
{
  m_highlighter->setBadStartBraceFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setEndBraceFormat(QBrush color,
                                    QBrush back,
                                    QFont font,
                                    QBrush underline,
                                    QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setEndBraceFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setEndBraceFormat(QTextCharFormat format)
{
  m_highlighter->setEndBraceFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadEndBraceFormat(QBrush color,
                                       QBrush back,
                                       QFont font,
                                       QBrush underline,
                                       QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setBadEndBraceFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadEndBraceFormat(QTextCharFormat format)
{
  m_highlighter->setBadEndBraceFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBraceMatchFormat(QBrush color,
                                      QBrush back,
                                      QFont font,
                                      QBrush underline,
                                      QTextCharFormat::UnderlineStyle style)
{
  m_highlighter->setBraceMatchFormat(color, back, font, underline, style);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBraceMatchFormat(QTextCharFormat format)
{
  m_highlighter->setBraceMatchFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setBadBraceMatchFormat(QTextCharFormat format)
{
  m_highlighter->setBadBraceMatchFormat(format);
  m_highlighter->rehighlight();
}

void
StylesheetEditor::setCommentFormat(QTextCharFormat format)
{
  m_highlighter->setCommentFormat(format);
  m_highlighter->rehighlight();
}

int
StylesheetEditor::maxSuggestionCount() const
{
  return m_parser->maxSuggestionCount();
}

void
StylesheetEditor::setMaxSuggestionCount(int maxSuggestionCount)
{
  m_parser->setMaxSuggestionCount(maxSuggestionCount);
}

void
StylesheetEditor::mousePressEvent(QMouseEvent* event)
{
  auto tc = cursorForPosition(event->pos());
  auto lineNumber = calculateLineNumber(tc);
  if (lineNumber >= 1) {
    setLineNumber(lineNumber);
    update();
  }
  QPlainTextEdit::mousePressEvent(event);
}

void
StylesheetEditor::mouseMoveEvent(QMouseEvent* event)
{
  //  auto pos = event->pos();
  //  auto cursorPos = cursorForPosition(pos).anchor();
  //  if (cursorPos >= toPlainText().length()) {
  //    // outside text
  //    return;
  //  }
  //  NodeSection* section = nullptr;
  //  QString hover;
  //  m_parser->nodeForPoint(pos, &section);
  //  setToolTip(QString());
  //  if (section) {
  //    if (*section != *m_oldSection) {
  //      if (section->node) {
  //        auto node = section->node;
  //        if (section->isWidgetType()) {
  //          auto widget = qobject_cast<WidgetNode*>(node);
  //          switch (section->type) {
  //            case SectionType::WidgetName: {
  //              if (widget && widget->isNameFuzzy()) {
  //                setHoverFuzzyWidgetName(hover, widget->name());
  //              }
  //              break;
  //            }
  //            case SectionType::WidgetSubControlMarker:
  //            case SectionType::WidgetSubControlName: {
  //              //              if (widget->isSubControlFuzzy()) {
  //              //                setHoverFuzzySubControl(hover,
  //              //                widget->subControls()->name());
  //              //              } else if (widget->isSubControlBad()) {
  //              //                setHoverBadSubControl(
  //              //                  hover, widget->name(),
  //              //                  widget->subControls()->name());
  //              //              }
  //              break;
  //            }
  //            case SectionType::WidgetPseudoStateMarker:
  //            case SectionType::WidgetPseudoState: {
  //              //              if (widget->isSubControlFuzzy()) {
  //              //                if (widget->isPseudoState()) {
  //              //                  setHoverFuzzyPseudoState(hover,
  //              //                  widget->extensionName());
  //              //                }
  //              //              }
  //              break;
  //            }
  //            case WidgetPropertyName:
  //              break;
  //            case WidgetPropertyValue:
  //              break;
  //            case WidgetStartBrace:
  //            case WidgetEndBrace:
  //              break;
  //            default: {
  //              qWarning();
  //              break;
  //            }
  //          }
  //        } else if (section->isPropertyType()) {
  //          auto property = qobject_cast<PropertyNode*>(node);
  //          switch (section->type) {
  //            case SectionType::PropertyName: {
  //              if (!property->isValidPropertyName()) {
  //                hover.append(tr("Invalid property name <em>%1</em>")
  //                               .arg(property->name()));
  //              } else if (!property->hasPropertyMarker()) {
  //                setHoverBadPropertyMarker(hover);
  //              } else if (!property->hasPropertyEndMarker()) {
  //                setHoverBadPropertyEndMarker(hover, property->end());
  //              }
  //              break;
  //            }
  //            case SectionType::PropertyValue: {
  //              if (!property->hasPropertyEndMarker()) {
  //                setHoverBadPropertyEndMarker(hover, property->end());
  //              } else if (!property->hasPropertyMarker()) {
  //                setHoverBadPropertyMarker(hover);
  //              } else if (!property->isValueValid(section->position)) {
  //                hover.append(tr("Invalid property value <em>%1</em>")
  //                               .arg(property->name()));
  //              } else {
  //                switch (property->valueStatus(section->position)->state) {
  //                  case FuzzyColorValue:
  //                    hover.append(tr("Fuzzy color value <em>%1</em>")
  //                                   .arg(property->value(section->position)));
  //                    break;
  //                  case FuzzyGradientName:
  //                    hover.append(tr("Fuzzy gradient name <em>%1</em>")
  //                                   .arg(property->value(section->position)));
  //                    break;
  //                  case BadGradientValue:
  //                    hover.append(tr("Bad gradient value <em>%1</em>")
  //                                   .arg(property->value(section->position)));
  //                    break;
  //                  case BadGradientValueCount:
  //                    hover.append(tr("Wrong number of parameters
  //                    <em>%1</em>")
  //                                   .arg(property->value(section->position)));
  //                    break;
  //                  case BadGradientNumericalValue:
  //                    hover.append(tr("Bad numerical value <em>%1</em>")
  //                                   .arg(property->value(section->position)));
  //                    break;
  //                  case BadGradientNumericalAndColorValue:
  //                    hover.append(tr("Bad numerical & color values
  //                    <em>%1</em>")
  //                                   .arg(property->value(section->position)));
  //                    break;
  //                  case BadGradientColorValue:
  //                    hover.append(tr("Bad color value <em>%1</em>")
  //                                   .arg(property->value(section->position)));
  //                    break;
  //                }
  //              }
  //            }
  //            case SectionType::PropertyEndMarker:
  //            default:
  //              break;
  //          }
  //        }
  //      }
  //      if (!hover.isEmpty()) {
  //        QToolTip::showText(event->globalPos(), hover, this,
  //        viewport()->rect());
  //      } else {
  //        QToolTip::hideText();
  //      }
  //      m_oldSection = section;
  //    }
  //  } else {
  //    QToolTip::hideText();
  //    m_oldSection->clear();
  //  }
  QPlainTextEdit::mouseMoveEvent(event);
}

// void
// StylesheetEditorPrivate::handleMouseMove(QMouseEvent* event)
//{
//  auto pos = event->pos();
//  NodeSection* section = nullptr;
//  QString hover;
//  m_parser->nodeForPoint(pos, &section);
//  setToolTip(QString());
//  if (section) {
//    if (*section != *m_oldSection) {
//      if (section->node) {
//        auto node = section->node;
//        if (section->isWidgetType()) {
//          auto widget = qobject_cast<WidgetNode*>(node);
//          switch (section->type) {
//            case SectionType::WidgetName: {
//              if (widget->isNameFuzzy()) {
//                setHoverFuzzyWidgetName(hover, widget->name());
//              }
//              break;
//            }
//            case SectionType::WidgetSubControlMarker:
//            case SectionType::WidgetSubControl: {
//              if (widget->isExtensionFuzzy()) {
//                setHoverFuzzySubControl(hover, widget->extensionName());
//              } else if (widget->isExtensionBad()) {
//                setHoverBadSubControl(
//                  hover, widget->name(), widget->extensionName());
//              }
//              break;
//            }
//            case SectionType::WidgetPseudoStateMarker:
//            case SectionType::WidgetPseudoState: {
//              if (widget->isExtensionFuzzy()) {
//                if (widget->isPseudoState()) {
//                  setHoverFuzzyPseudoState(hover, widget->extensionName());
//                }
//              }
//              break;
//            }
//            default: {
//              qWarning();
//              break;
//            }
//          }
//        } else if (section->isPropertyType()) {
//          auto property = qobject_cast<PropertyNode*>(node);
//          switch (section->type) {
//            case SectionType::PropertyName: {
//              if (!property->isValidPropertyName()) {
//                hover.append(tr("Invalid property name <em>%1</em>")
//                               .arg(property->name()));
//              }
//              if (!property->hasPropertyMarker()) {
//                setHoverBadPropertyMarker(hover);
//              }
//              if (!property->hasPropertyEndMarker()) {
//                setHoverBadPropertyEndMarker(hover, property);
//              }
//              break;
//            }
//            case SectionType::PropertyValue: {
//              if (!property->hasPropertyEndMarker()) {
//                setHoverBadPropertyEndMarker(hover, property);
//              } else if (!property->hasPropertyMarker()) {
//                setHoverBadPropertyMarker(hover);
//              } else if (!property->isValueValid(section->position)) {
//                hover.append(tr("Invalid property value <em>%1</em>")
//                               .arg(property->name()));
//              }
//              break;
//            }
//            case SectionType::PropertyEndMarker:
//            default:
//              break;
//          }
//        }
//      }
//      if (!hover.isEmpty()) {
//        QToolTip::showText(
//          event->globalPos(), hover, this, viewport()->rect());
//      } else {
//        QToolTip::hideText();
//      }
//      m_oldSection = section;
//    }
//  } else {
//    QToolTip::hideText();
//    m_oldSection->clear();
//  }
//}

void
StylesheetEditor::setHoverBadPropertyMarker(QString& hover)
{
  if (!hover.isEmpty()) {
    hover.append("\n");
  }
  hover.append(tr("No property marker (<b>:</b>)"));
}

void
StylesheetEditor::setHoverFuzzyWidgetName(QString& hover, const QString& name)
{
  if (!hover.isEmpty()) {
    hover.append("\n");
  }
  hover.append(tr("Invalid <em>fuzzy</em> widget name <em>%1</em>").arg(name));
}

void
StylesheetEditor::setHoverFuzzySubControl(QString& hover, const QString& name)
{
  if (!hover.isEmpty()) {
    hover.append("\n");
  }
  hover.append(tr("Invalid <em>fuzzy</em> subcontrol <em>%1</em>").arg(name));
}

void
StylesheetEditor::setHoverBadSubControl(QString& hover,
                                        const QString& widget,
                                        const QString& name)
{
  if (!hover.isEmpty()) {
    hover.append("\n");
  }
  hover.append(
    tr("Invalid subcontrol <em>%1</em> for <em>%2</em>").arg(name).arg(widget));
}

void
StylesheetEditor::setHoverFuzzyPseudoState(QString& hover, const QString& name)
{
  if (!hover.isEmpty()) {
    hover.append("\n");
  }
  hover.append(tr("Invalid <em>fuzzy</em> pseudo state <em>%1</em>").arg(name));
}

void
StylesheetEditor::setHoverBadPropertyEndMarker(QString& hover, int end)
{
  auto text = toPlainText().mid(end);
  if (!checkForEmpty(text)) {
    if (!hover.isEmpty()) {
      hover.append("\n");
    }
    hover.append(tr("No end marker (<b>;</b>)"));
  }
}

void
StylesheetEditor::contextMenuEvent(QContextMenuEvent* event)
{
  handleCustomMenuRequested(event->pos());
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

  if (m_lineNumberArea)
    m_lineNumberArea->setLineCount(count);

  return lines;
}

void
StylesheetEditor::format()
{
  // TODO pretty print format
}

void
StylesheetEdit::options()
{
  m_editor->options();
}

void
StylesheetEditor::options()
{
  auto dlg = new StylesheetEditDialog(m_datastore, this, this);

  if (dlg->exec() == QDialog::Accepted) {
    saveYamlConfig();
  }
}

QString
StylesheetEditor::getValueAtCursor(int pos, const QString& text)
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
  m_lineNumberArea->setLineNumber(currentLineNumber);
  m_lineNumberArea->update();
}

/*
   Calculates the current text column.
*/
int
StylesheetEditor::calculateColumn(QTextCursor textCursor)
{
  QTextCursor cursor(textCursor);
  cursor.movePosition(QTextCursor::StartOfLine);
  return textCursor.anchor() - cursor.anchor();
}

void
StylesheetEditor::cursorPositionHasChanged()
{
  m_parser->handleCursorPositionChanged(textCursor());
}

void
StylesheetEditor::suggestionMade(bool)
{
  auto act = dynamic_cast<QAction*>(sender());

  if (act) {
    m_parser->handleSuggestions(act);
  }
}

void StylesheetEditor::bookmarkMenuRequested(QPoint /*pos*/)
{
  qWarning();
}

void StylesheetEditor::linenumberMenuRequested(QPoint /*pos*/)
{
  qWarning();
}

void
StylesheetEditor::handleCustomMenuRequested(QPoint pos)
{
  QMenu* menu = new QMenu(this); // createStandardContextMenu();

  auto act = new QAction(tr("&Undo"), this);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z));
  connect(act, &QAction::triggered, this, &StylesheetEditor::undo);
  menu->addAction(act);

  act = new QAction(tr("&Redo"), this);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z));
  connect(act, &QAction::triggered, this, &StylesheetEditor::redo);
  menu->addAction(act);

  menu->addSeparator();

  act = new QAction(tr("Cu&t"), this);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_X));
  connect(act, &QAction::triggered, this, &StylesheetEditor::cut);
  menu->addAction(act);

  act = new QAction(tr("&Copy"), this);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
  connect(act, &QAction::triggered, this, &StylesheetEditor::copy);
  menu->addAction(act);

  act = new QAction(tr("&Paste"), this);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_V));
  connect(act, &QAction::triggered, this, &StylesheetEditor::paste);
  menu->addAction(act);

  menu->addSeparator();

  act = new QAction(tr("Select All"), this);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_A));
  connect(act, &QAction::triggered, this, &StylesheetEditor::selectAll);
  menu->addAction(act);

  menu->addSeparator();

  menu->addMenu(m_parser->handleMouseClicked(pos));

  menu->addSeparator();

  act = new QAction(tr("&Format"));
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
  act->setStatusTip(tr("Prettyfy the stylesheet"));
  connect(act, &QAction::triggered, this, &StylesheetEditor::format);
  menu->addAction(act);

  menu->addSeparator();

  act = new QAction(tr("&Options..."));
  connect(act, &QAction::triggered, this, &StylesheetEditor::options);
  menu->addAction(act);

  menu->popup(viewport()->mapToGlobal(pos));
}

// void
// StylesheetEditorPrivate::suggestionMade(QAction* act)
//{
//  m_parser->handleSuggestions(act);
//}

void
StylesheetEditor::documentChanged(int pos, int charsRemoved, int charsAdded)
{
  m_parser->handleDocumentChanged(pos, charsRemoved, charsAdded);
}

void
StylesheetEditor::handleTextChanged()
{
  // TODO probably remove
  qWarning();
}

// void
// StylesheetEditorPrivate::handleDocumentChanged(int pos,
//                                               int charsRemoved,
//                                               int charsAdded)
//{
//  m_parser->handleDocumentChanged(pos, charsRemoved, charsAdded);
//}

// void
// StylesheetEditorPrivate::handleTextChanged()
//{
//  qWarning();
//}

int
StylesheetEditor::bookmarkLineNumber() const
{
  return m_bookmarkLineNumber;
}

void
StylesheetEditor::setBookmarkLineNumber(int bookmarkLineNumber)
{
  m_bookmarkLineNumber = bookmarkLineNumber;
}

int
StylesheetEditor::bookmarkAreaWidth()
{
  return m_bookmarkArea->bookmarkAreaWidth();
}

// int
// StylesheetEditorPrivate::bookmarkAreaWidth()
//{
//  return m_bookmarkArea->bookmarkAreaWidth();
//}

int
StylesheetEditor::lineNumberAreaWidth()
{
  return m_lineNumberArea->lineNumberAreaWidth();
}

// int
// StylesheetEditorPrivate::lineNumberAreaWidth()
//{
//  return m_lineNumberArea->lineNumberAreaWidth();
//}

QMap<int, BookmarkData*>*
StylesheetEditor::bookmarks()
{
  return m_bookmarkArea->bookmarks();
}

// QMap<int, BookmarkData*>*
// StylesheetEditorPrivate::bookmarks()
//{
//  return m_bookmarkArea->bookmarks();
//}

void
StylesheetEditor::setBookmarks(QMap<int, BookmarkData*>* bookmarks)
{
  m_bookmarkArea->setBookmarks(bookmarks);
}

// void
// StylesheetEditorPrivate::setBookmarks(QMap<int, BookmarkData*>* bookmarks)
//{
//  m_bookmarkArea->setBookmarks(bookmarks);
//}

void
StylesheetEditor::handleAddBookmark(bool)
{
  insertBookmark();
}

void
StylesheetEditor::insertBookmark(int bookmark, const QString& text)
{
  int bm = (bookmark >= 0 ? bookmark : m_bookmarkLineNumber);
  m_bookmarkArea->insertBookmark(bm, text);
}

// void
// StylesheetEditorPrivate::insertBookmark(int bookmark, const QString& text)
//{
//  int bm = (bookmark >= 0 ? bookmark : m_bookmarkLineNumber);
//  m_bookmarkArea->insertBookmark(bm, text);
//}

void
StylesheetEditor::toggleBookmark(int bookmark)
{
  m_bookmarkArea->toggleBookmark(bookmark);
}

// void
// StylesheetEditorPrivate::toggleBookmark(int bookmark)
//{
//  m_bookmarkArea->toggleBookmark(bookmark);
//}

void
StylesheetEditor::editBookmark(int bookmark)
{
  int lineNumber = (bookmark >= 0 ? bookmark : m_bookmarkLineNumber);
  m_bookmarkArea->editBookmark(lineNumber);
}

void
StylesheetEditor::handleEditBookmark(bool)
{
  editBookmark();
}

// void
// StylesheetEditorPrivate::editBookmark(int bookmark)
//{
//  int lineNumber = (bookmark >= 0 ? bookmark : m_bookmarkLineNumber);
//  m_bookmarkArea->editBookmark(lineNumber);
//}

void
StylesheetEditor::removeBookmark(int bookmark)
{
  int lineNumber = (bookmark >= 0 ? bookmark : m_bookmarkLineNumber);
  m_bookmarkArea->removeBookmark(lineNumber);
}

void
StylesheetEditor::handleRemoveBookmark(bool)
{
  removeBookmark();
}

// void
// StylesheetEditorPrivate::removeBookmark(int bookmark)
//{
//  int lineNumber = (bookmark >= 0 ? bookmark : m_bookmarkLineNumber);
//  m_bookmarkArea->removeBookmark(lineNumber);
//}

void
StylesheetEditor::clearBookmarks()
{
  m_bookmarkArea->clearBookmarks();
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
    clearBookmarks();
  }
}

// void
// StylesheetEditorPrivate::clearBookmarks()
//{
//  m_bookmarkArea->clearBookmarks();
//}

bool
StylesheetEditor::hasBookmark(int bookmark)
{
  return m_bookmarkArea->hasBookmark(bookmark);
}

// bool
// StylesheetEditorPrivate::hasBookmark(int linenumber)
//{
//  return m_bookmarkArea->hasBookmark(linenumber);
//}

bool
StylesheetEditor::hasBookmarkText(int bookmark)
{
  return m_bookmarkArea->hasBookmarkText(bookmark);
}

// bool
// StylesheetEditorPrivate::hasBookmarkText(int bookmark)
//{
//  return m_bookmarkArea->hasBookmarkText(bookmark);
//}

QString
StylesheetEditor::bookmarkText(int bookmark)
{
  return m_bookmarkArea->bookmarkText(bookmark);
}

// QString
// StylesheetEditorPrivate::bookmarkText(int bookmark)
//{
//  return m_bookmarkArea->bookmarkText(bookmark);
//}

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
      setLineNumber(bookmark);
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
//    thread()->sleep(100);
//  }
//}

// void
// StylesheetEditorPrivate::handleBookmarkMenuEvent(QPoint pos)
//{
//  auto tc = cursorForPosition(pos);
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
StylesheetEditor::currentCursor() const
{
  return m_parser->currentCursor();
}

void
StylesheetEditor::setCurrentCursor(const QTextCursor& currentCursor)
{
  m_parser->setCurrentCursor(currentCursor);
  setLineData(currentCursor);
}

// void
// StylesheetEditorPrivate::setCurrentLineNumber(const int number)
//{
//  m_lineNumberArea->setLineNumber(number);
//}

int
StylesheetEditor::currentLineNumber() const
{
  return m_lineNumberArea->currentLineNumber();
}

// int
// StylesheetEditorPrivate::currentLineCount() const
//{
//  return m_lineNumberArea->lineCount();
//}

void
StylesheetEditor::setLineNumber(int lineNumber)
{
  m_datastore->setManualMove(true);
  QTextCursor cursor(document());
  cursor.movePosition(QTextCursor::Start);
  cursor.movePosition(
    QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber - 1);
  setTextCursor(cursor);
  setFocus(Qt::OtherFocusReason);
  setCurrentCursor(cursor);
  //  setLineData(cursor);
  m_lineNumberArea->setLineNumber(lineNumber);
  m_datastore->setManualMove(false);
}

void
StylesheetEditor::up(int n)
{
  m_datastore->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, n);
  setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_datastore->setManualMove(false);
}

void
StylesheetEditor::setLineData(QTextCursor cursor)
{
  // this handles display of linenumber, linecount and character column.
  int ln = calculateLineNumber(cursor);
  m_lineNumberArea->setLineNumber(ln);
}

// void
// StylesheetEditorPrivate::up(int n)
//{
//  m_datastore->setManualMove(true);
//  QTextCursor cursor(currentCursor());
//  cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, n);
//  setTextCursor(cursor);
//  setCurrentCursor(cursor);
//  setLineData(cursor);
//  m_datastore->setManualMove(false);
//}

void
StylesheetEditor::down(int n)
{
  m_datastore->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, n);
  setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_datastore->setManualMove(false);
}

// void
// StylesheetEditorPrivate::down(int n)
//{
//  m_datastore->setManualMove(true);
//  QTextCursor cursor(currentCursor());
//  cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, n);
//  setTextCursor(cursor);
//  setCurrentCursor(cursor);
//  setLineData(cursor);
//  m_datastore->setManualMove(false);
//}

void
StylesheetEditor::left(int n)
{
  m_datastore->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, n);
  setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_datastore->setManualMove(false);
}

// void
// StylesheetEditorPrivate::left(int n)
//{
//  m_datastore->setManualMove(true);
//  QTextCursor cursor(currentCursor());
//  cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, n);
//  setTextCursor(cursor);
//  setCurrentCursor(cursor);
//  setLineData(cursor);
//  m_datastore->setManualMove(false);
//}

void
StylesheetEditor::right(int n)
{
  m_datastore->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, n);
  setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_datastore->setManualMove(false);
}

void
StylesheetEditor::start()
{
  m_datastore->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::Start);
  setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_datastore->setManualMove(false);
}

void
StylesheetEditor::end()
{
  m_datastore->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::End);
  setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_datastore->setManualMove(false);
}

void
StylesheetEditor::startOfLine()
{
  m_datastore->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::StartOfLine);
  setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_datastore->setManualMove(false);
}

void
StylesheetEditor::endOfLine()
{
  m_datastore->setManualMove(true);
  QTextCursor cursor(currentCursor());
  cursor.movePosition(QTextCursor::EndOfLine);
  setTextCursor(cursor);
  setCurrentCursor(cursor);
  setLineData(cursor);
  m_datastore->setManualMove(false);
}

void
StylesheetEditor::goToLine(int lineNumber)
{
  setLineNumber(lineNumber);
}

int
StylesheetEditor::bookmarkCount()
{
  return m_bookmarkArea->count();
}
