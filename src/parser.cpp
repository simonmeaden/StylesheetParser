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
#include "parser.h"

#include "datastore.h"
#include "node.h"
#include "stylesheetedit_p.h"

#include <QGridLayout>
#include <QLabel>
#include <QWidgetAction>
#include <QtDebug>

IconLabel::IconLabel(const QIcon& icon, const QString& text, QWidget* parent)
  : QWidget(parent)
{
  auto layout = new QGridLayout;
  //  setContentsMargins(0, 0, 0, 0);
  //  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);
  auto iconLbl = new QLabel(this);
  auto pix = icon.pixmap(QSize(16, 16));
  iconLbl->setPixmap(pix);
  layout->addWidget(iconLbl, 0, 0);
  auto textLbl = new QLabel(text, this);
  layout->addWidget(textLbl, 0, 1);
}

Parser::Parser(DataStore* datastore, StylesheetEditor* editor, QObject* parent)
  : QObject(parent)
  , m_editor(editor)
  , m_datastore(datastore)
  , m_showLineMarkers(false)
  , m_initialising(true)
{
  connect(this, &Parser::setBraceCount, m_datastore, &DataStore::setBraceCount);
}

Parser::Parser(const Parser& /*other*/)
{
  //  d_ptr = new ParserData(*other.d_ptr);
}

Parser::~Parser()
{
  //  delete d_ptr;
  emit finished();
}

Parser&
Parser::operator=(const Parser& /*other*/)
{
  //  *d_ptr = *other.d_ptr;
  return *this;
}

void
Parser::stepBack(int& pos, const QString& block)
{
  pos -= block.length();
}

enum NodeType
Parser::checkType(const QString& block, PropertyNode* property) const
{
  if (property) {
    if (m_datastore->isValidPropertyValueForProperty(property->name(), block)) {
      return NodeType::PropertyValueType;
    }
  }
  if (m_datastore->containsWidget(block)) {
    return NodeType::WidgetType;
  } else if (m_datastore->containsProperty(block)) {
    return NodeType::PropertyType;
  } else if (m_datastore->containsPseudoState(block)) {
    return NodeType::PseudoStateType;
  } else if (m_datastore->containsSubControl(block)) {
    return NodeType::SubControlType;
  } else if (block == ":") {
    return NodeType::ColonType;
  } else if (block == "::") {
    return NodeType::SubControlMarkerType;
  } else if (block == "/*") {
    return NodeType::CommentType;
  } else if (block == "{") {
    return NodeType::StartBraceType;
  } else if (block == "}") {
    return NodeType::EndBraceType;
  } else if (block == "\n") {
    return NodeType::NewlineType;
  } else {
    QMultiMap<int, QString> possibilities;
    if (property) {
      possibilities =
        m_datastore->fuzzySearchPropertyValue(property->name(), block);
      if (possibilities.size() > 0)
        return NodeType::FuzzyPropertyValueType;
    }
    possibilities = m_datastore->fuzzySearchProperty(block);
    if (possibilities.size() > 0)
      return NodeType::FuzzyPropertyType;
    possibilities = m_datastore->fuzzySearchWidgets(block);
    if (possibilities.size() > 0)
      return NodeType::FuzzyWidgetType;
    possibilities = m_datastore->fuzzySearchPseudoStates(block);
    if (possibilities.size() > 0)
      return NodeType::FuzzyPseudoStateType;
    possibilities = m_datastore->fuzzySearchSubControl(block);
    if (possibilities.size() > 0)
      return NodeType::FuzzySubControlType;
  }
  return NoType;
}

void
Parser::parseInitialText(const QString& text)
{
  m_initialising = true;
  m_datastore->setBraceCount(0);
  QString block;
  int start;
  int pos = 0;

  while (true) {
    if ((block = findNext(text, pos)).isEmpty()) {
      break;
    }

    start = pos - block.length();
    QTextCursor cursor;

    auto type = checkType(block);

    switch (type) {
      case NodeType::WidgetType:
      case NodeType::FuzzyWidgetType: {
        auto widget = stashWidget(start, block);
        if (type == NodeType::FuzzyWidgetType)
          widget->setWidgetValid(false);

        while (!(block = findNext(text, pos)).isEmpty()) {
          start = pos - block.length();
          cursor = getCursorForPosition(start);
          auto leaveWidget = false;

          type = checkType(block);
          if (type == NodeType::ColonType) {
            // a single colon could be either a pseudo state marker or a
            // property marker. if a property is detected then it should be
            // eaten within the property parsing so this should only appear
            // within a widget.
            type = NodeType::PseudoStateMarkerType;
          }
          switch (type) {
            case NodeType::WidgetType:
            case NodeType::FuzzyWidgetType:
              // if we find another widget than accept that the previous one is
              // incomplete.
              stepBack(pos, block);
              leaveWidget = true;
              break;
            case NodeType::PropertyType:
            case NodeType::FuzzyPropertyType: {
              PropertyNode* property =
                new PropertyNode(block, cursor, m_editor);
              property->setWidget(widget);
              if (type == NodeType::FuzzyPropertyType)
                property->setValidPropertyName(false);
              else
                property->setValidPropertyName(true);
              widget->addProperty(property);
              parsePropertyWithValues(property, text, start, pos, block);
              continue;
            }
            case NodeType::SubControlType:
            case NodeType::PseudoStateType:
            case NodeType::FuzzySubControlType:
            case NodeType::FuzzyPseudoStateType:
              widget->setExtensionName(block);
              widget->setExtensionCursor(cursor);
              widget->setExtensionType(type);
              if (widget->doesMarkerMatch(type)) {
                widget->setExtensionMarkerCorrect(true);
              } else {
                widget->setExtensionMarkerCorrect(false);
              }
              continue;
            case NodeType::CommentType:
              parseComment(text, start, pos);
              continue;
            case NodeType::SubControlMarkerType:
              widget->setSubControlMarkerCursor(cursor);
              break;
            case NodeType::PseudoStateMarkerType:
              widget->setPseudoStateMarkerCursor(cursor);
              break;
            case NodeType::StartBraceType:
              widget->setStartBraceCursor(cursor);
              break;
            case NodeType::EndBraceType:
              widget->setEndBraceCursor(cursor);
              leaveWidget = true;
              break;
            case NodeType::NewlineType:
              stashNewline(pos++);
              break;
            default:
              // TODO error
              break;
          }
          if (leaveWidget)
            break;
        }
        continue;
      }
      case NodeType::PropertyType:
      case NodeType::FuzzyPropertyType: {
        cursor = getCursorForPosition(start);
        PropertyNode* property =
          new PropertyNode(block, cursor, m_editor, this);
        if (type == NodeType::FuzzyPropertyType)
          property->setValidPropertyName(false);
        else
          property->setValidPropertyName(true);
        m_datastore->insertNode(cursor, property);
        parsePropertyWithValues(property, text, start, pos, block);
        continue;
      }
      case NodeType::CommentType:
        parseComment(text, start, pos);
        continue;
      case NodeType::StartBraceType:
        stashStartBrace(pos - block.length());
        break;
      case NodeType::EndBraceType:
        stashEndBrace(pos - block.length());
        break;
      case NodeType::NewlineType:
        stashNewline(pos++);
        break;
      default:
        QString nextBlock;

        if (!m_datastore->isNodesEmpty()) {
          auto oldPos = pos;
          nextBlock = findNext(text, pos);

          if (nextBlock == ":") {
            auto colonPos = pos;
            nextBlock = findNext(text, pos);

            if (m_datastore->containsPseudoState(nextBlock)) {
              stashWidget(start, block, false);
              continue;

            } else if (m_datastore->propertyValueAttribute(nextBlock) !=
                       NoAttributeValue) {
              cursor = getCursorForPosition(start);
              PropertyNode* property =
                new PropertyNode(block, cursor, m_editor);
              property->setPropertyMarker(true);
              property->setPropertyMarkerCursor(getCursorForPosition(colonPos));
              property->setValidPropertyName(false);
              m_datastore->insertNode(cursor, property);
              pos -= block.length(); // step back
              parsePropertyWithValues(property, text, start, pos, block);
              continue;
            }

          } else if (nextBlock == "::") {
            nextBlock = findNext(text, pos);

            if (m_datastore->containsSubControl(nextBlock)) {
              /*auto widget =*/stashWidget(start, block, false);
              continue;
            }
          }
          // step back
          pos = oldPos;
          stashWidget(start, block, false);
        } else { // anomalous type - see what comes next.
          int oldPos = pos;
          nextBlock = findNext(text, pos);

          if (nextBlock == ":") {
            nextBlock = findNext(text, pos);

            if (m_datastore->containsPseudoState(nextBlock)) {
              stashWidget(start, block, false);
              continue;

            } else if (m_datastore->propertyValueAttribute(nextBlock) !=
                       NoAttributeValue) {
              cursor = getCursorForPosition(start);
              PropertyNode* property =
                new PropertyNode(block, cursor, m_editor);
              property->setPropertyMarker(true);
              property->setPropertyMarkerCursor(getCursorForPosition(oldPos));
              property->setValidPropertyName(false);
              m_datastore->insertNode(cursor, property);
              pos -= block.length(); // step back
              parsePropertyWithValues(property, text, start, pos, block);
              continue;
            }

          } else if (nextBlock == "::") {
            nextBlock = findNext(text, pos);

            if (m_datastore->containsSubControl(nextBlock)) {
              stashWidget(start, block, false);
              continue;
            }
          }

          stashBadNode(start, block, ParserState::AnomalousType);
        }
        break;
    }
  }

  emit parseComplete();
}

void
Parser::parsePropertyWithValues(PropertyNode* property,
                                const QString& text,
                                int start,
                                int& pos,
                                QString& block)
{
  QString propertyName = property->name();

  while (!(block = findNext(text, pos)).isEmpty()) {
    if (block == ":") {
      if (!property->hasPropertyMarker()) {
        property->setPropertyMarker(true);
        property->setPropertyMarkerCursor(getCursorForPosition(pos - 1));
      } else {
        /*TODO error too many :*/
      }
    } else if (block == "/*") { // comment start
      parseComment(text, start, pos);
    } else if (block == "\n") { // comment start
      stashNewline(pos++);
    } else if (block == ";") {
      property->setPropertyEndMarker(true);
      property->setPropertyEndMarkerCursor(getCursorForPosition(pos - 1));
      break;
    } else if (block == "}") {
      stepBack(pos, block);
      break;
    } else {
      bool validForProperty =
        m_datastore->isValidPropertyValueForProperty(propertyName, block);
      AttributeType attributeType = m_datastore->propertyValueAttribute(block);

      if (validForProperty) {
        // valid property and valid value.
        property->addValue(block,
                           PropertyValueCheck::GoodValue,
                           getCursorForPosition((pos - block.length())),
                           attributeType);
      } else {
        if (attributeType == NoAttributeValue) {
          // not a valid value for any property
          auto type = checkType(block, property);
          switch (type) {
            case NodeType::WidgetType:
            case NodeType::FuzzyWidgetType:
              stepBack(pos, block);
              return;

            case NodeType::PropertyType:
            case NodeType::FuzzyPropertyType: {
              // another property follows incomplete property.
              stepBack(pos, block);
              return;
            }
            default:
              property->addValue(block,
                                 PropertyValueCheck::BadValue,
                                 getCursorForPosition((pos - block.length())),
                                 attributeType);
          }
        } else {
          // invalid property name but this is a valid property attribute
          // anyway.
          property->addValue(block,
                             PropertyValueCheck::ValidPropertyType,
                             getCursorForPosition((pos - block.length())),
                             attributeType);
        }
      }
    }
  }
}

void
Parser::parseComment(const QString& text, int start, int& pos)
{
  QTextCursor cursor = getCursorForPosition(start);

  QChar c;
  CommentNode* comment = new CommentNode(cursor, m_editor);
  m_datastore->insertNode(cursor, comment);

  while (true) {
    c = text.at(pos);
    if (c.isSpace()) {
      pos++;
    } else {
      comment->setTextCursor(getCursorForPosition(pos));
      pos++;
      break;
    }
  }

  while (pos < text.length()) {

    if (c != '*') {
      comment->append(c);

    } else {
      if (pos < text.length()) {
        c = text.at(pos++);

        if (c == '/') {
          comment->setEndCommentExists(true);
          comment->setEndCommentCursor(getCursorForPosition(pos - 2));
          break;

        } else {
          comment->append(c);
        }
      }
    }
    c = text.at(pos++);
  }
}

QString
Parser::findNext(const QString& text, int& pos)
{
  QString block;
  QChar c;
  skipBlanks(text, pos);

  if (pos < text.length()) {

    while (pos < text.length()) {
      c = text.at(pos);

      if (c.isNull()) {
        return block;
      }

      if (c.isLetterOrNumber() || c == '-') {
        if (!block.isEmpty()) {
          QChar b = block.back();

          if (b == '{' || b == '}' || b == ';' || b == ':') {
            return block;
          }
        }

        block += c;
        pos++;

      } else if (m_showLineMarkers && c == '\n') {
        if (block.isEmpty()) {
          block += c;
        }
        return block;
      } else if (c.isSpace() && !block.isEmpty()) {
        return block;

      } else if (c == '{' || c == '}' || c == ';' || c == ':') {
        if (!block.isEmpty()) {
          if (block.back().isLetterOrNumber()) {
            // a possibly correct name/number string
            return block;
          }
        }

        if (block.length() == 0 || block.back() == c) {
          block += c;
          pos++;

        } else {
          return block;
        }

      } else if (c == '/') {
        if (pos < text.length() - 1) {
          if (text.at(pos + 1) == '*') {
            // a comment
            if (!block.isEmpty()) {
              pos--; // step back.
              return block;

            } else {
              pos += 2;
              return "/*";
            }
          }
        }
      }

      if (pos < text.length()) {
        c = text.at(pos);
      }
    }
  }

  return QString();
}

void
Parser::skipBlanks(const QString& text, int& pos)
{
  QChar c;

  for (; pos < text.length(); pos++) {
    c = text.at(pos);

    if (m_showLineMarkers && c == '\n') {
      break;
    } else if (c.isSpace() /* || c == '\n' || c == '\r' || c == '\t'*/) {
      continue;

    } else {
      break;
    }
  }
}

QTextCursor
Parser::getCursorForPosition(int position)
{
  QTextCursor c(m_editor->document());
  c.setPosition(position);
  return c;
}

void
Parser::nodeForPoint(const QPoint& pos, NodeSection** nodeSection)
{
  auto values = m_datastore->nodes().values();

  for (auto& n : values) {
    auto section = n->isIn(pos);

    if (section->type != SectionType::None) {
      section->node = n;
      *nodeSection = section;
      return;
    }
  }
}

bool
Parser::showLineMarkers() const
{
  return m_showLineMarkers;
}

void
Parser::setShowLineMarkers(bool showLineMarkers)
{
  m_showLineMarkers = showLineMarkers;
}

void
Parser::setInitialisingFlag(bool initialising)
{
  m_initialising = initialising;
}

CursorData
Parser::getNodeAtCursor(QTextCursor cursor)
{
  CursorData data;
  data.cursor = cursor;

  nodeAtCursorPosition(&data, cursor.anchor());

  return data;
}

CursorData
Parser::getNodeAtCursor(int position)
{
  CursorData data;
  data.cursor = QTextCursor(m_editor->document());

  nodeAtCursorPosition(&data, position);

  return data;
}

void
Parser::nodeAtCursorPosition(CursorData* data, int position)
{
  Node* previous;
  QMap<QTextCursor, Node*> nodes = m_datastore->nodes();

  for (auto key : nodes.keys()) {
    Node* node = nodes.value(key);

    if (!node) {
      return;
    }

    if (position > node->end()) {
      previous = node;
      continue;

    } else if (position >= node->start() && position <= node->end()) {
      data->node = node;
      data->cursor = node->cursor();
      data->prevNode = previous;
      break;
    }
  }
}

WidgetNode*
Parser::stashWidget(int position, const QString& block, bool valid)
{
  auto cursor = getCursorForPosition(position);
  auto widgetnode = new WidgetNode(block, cursor, m_editor);
  widgetnode->setWidgetValid(valid);
  m_datastore->insertNode(cursor, widgetnode);
  return widgetnode;
}

void
Parser::stashBadNode(int position,
                     const QString& block,
                     ParserState::Error error)
{
  auto cursor = getCursorForPosition(position);
  auto badblock = new BadBlockNode(block, cursor, error, m_editor);
  m_datastore->insertNode(cursor, badblock);
}

void
Parser::stashNewline(int position)
{
  auto cursor = getCursorForPosition(position);
  auto newline = new NewlineNode(cursor, m_editor);
  m_datastore->insertNode(cursor, newline);
}

void
Parser::stashEndBrace(int position)
{
  m_datastore->decrementBraceCount();
  auto cursor = getCursorForPosition(position);
  auto endbrace = new EndBraceNode(cursor, m_editor);
  m_datastore->insertNode(cursor, endbrace);
}

void
Parser::stashStartBrace(int position)
{
  m_datastore->incrementBraceCount();
  auto cursor = getCursorForPosition(position);
  auto startbrace = new StartBraceNode(cursor, m_editor);
  m_datastore->insertNode(cursor, startbrace);
}

// QMenu*
// Parser::createContextMenu()
//{
//  QMenu* menu = m_editor->createStandardContextMenu();
//  menu->addSeparator();
//  m_suggestionsMenu = menu->addMenu(m_editor->tr("&Suggestions"));
//  menu->addSeparator();
//  menu->addAction(m_formatAct);

//  m_suggestionsMenu->setEnabled(false);

//  return menu;
//}

void
Parser::updateContextMenu(QMap<int, QString> matches,
                          WidgetNode* node,
                          const QPoint& pos,
                          QMenu** suggestionsMenu)
{
  QString typeName;
  (*suggestionsMenu)->clear();

  switch (node->type()) {
    case NodeType::WidgetType:
      typeName = "widget";
      break;
  }

  QAction* act = (*suggestionsMenu)
                   ->addSection(m_datastore->invalidIcon(),
                                m_editor->tr("%1 is not a valid %2 name")
                                  .arg(node->name())
                                  .arg(typeName));
  act->setData(pos);
  (*suggestionsMenu)->addSeparator();

  updateMenu(matches, node, pos, suggestionsMenu, SectionType::WidgetName);
}

void
Parser::updatePropertyContextMenu(
  PropertyNode* property,
  const QPoint& pos,
  QMenu** suggestionsMenu,
  QMap<int, QString> matches = QMap<int, QString>())
{
  //  QString typeName;
  QAction* act;
  (*suggestionsMenu)->clear();

  if (!property->isValidPropertyName()) {
    act = (*suggestionsMenu)
            ->addSection(m_datastore->invalidIcon(),
                         m_editor->tr("%1 is not a valid property name")
                           .arg(property->name()));
    act->setData(pos);
    (*suggestionsMenu)->addSeparator();
    updateMenu(
      matches, property, pos, suggestionsMenu, SectionType::PropertyName);
  } else if (!property->hasPropertyMarker()) {
    act = new QAction(m_datastore->invalidIcon(),
                      m_editor->tr("%1 is missing a property marker (:)")
                        .arg(property->name()));
    //    act->setData(pos);
    (*suggestionsMenu)->addAction(act);
    (*suggestionsMenu)->addSeparator();
    act = new QAction(m_datastore->addColonIcon(),
                      m_editor->tr("Add property marker (:)"));
    (*suggestionsMenu)->addAction(act);
    setMenuData(act, property, pos, SectionType::PropertyMarker);
    m_editor->connect(
      act, &QAction::triggered, m_editor, &StylesheetEditor::suggestionMade);
    (*suggestionsMenu)->setEnabled(true);
  } else if (!property->hasPropertyEndMarker()) {
    act = new QAction(
      m_datastore->badSColonIcon(),
      m_editor->tr("%1 is missing an end marker (;)").arg(property->name()));
    //    act->setData(pos);
    (*suggestionsMenu)->addAction(act);
    (*suggestionsMenu)->addSeparator();
    act = new QAction(m_datastore->addSColonIcon(),
                      m_editor->tr("Add property end marker (;)"));
    (*suggestionsMenu)->addAction(act);
    setMenuData(act, property, pos, SectionType::PropertyEndMarker);
    m_editor->connect(
      act, &QAction::triggered, m_editor, &StylesheetEditor::suggestionMade);
    (*suggestionsMenu)->setEnabled(true);
  } else {
    act = new QAction(
      m_datastore->validIcon(),
      m_editor->tr("Property %1 appears to be valid!").arg(property->name()));
    act->setData(pos);
    (*suggestionsMenu)->addAction(act);
    (*suggestionsMenu)->addSeparator();
    (*suggestionsMenu)->setEnabled(true);
  }
}

QWidgetAction*
Parser::getWidgetAction(const QIcon& icon, const QString& text, QMenu* menu)
{
  auto lbl = new IconLabel(icon, text, m_editor);
  auto act = new QWidgetAction(menu);
  act->setDefaultWidget(lbl);
  return act;
}

void
Parser::updateValidPropertyValueContextMenu(QMultiMap<int, QString> matches,
                                            PropertyNode* property,
                                            const QString& valueName,
                                            const QPoint& pos,
                                            QMenu** suggestionsMenu)
{
  (*suggestionsMenu)->clear();

  auto keys = matches.keys(valueName);
  for (auto key : keys) {
    matches.remove(key, valueName);
  }

  auto act = getWidgetAction(
    m_datastore->validIcon(),
    m_editor->tr("%1 is a valid value for %2.<br>Other suggestions are : ")
      .arg(valueName)
      .arg(property->name()),
    *suggestionsMenu);
  (*suggestionsMenu)->addAction(act);
  (*suggestionsMenu)->addSeparator();

  updateMenu(matches, property, pos, suggestionsMenu, SectionType::None);
}

void
Parser::setMenuData(QAction* act,
                    Node* node,
                    QPoint pos,
                    SectionType type,
                    const QString& oldName)
{
  MenuData data;
  data.node = node;
  data.type = type;
  data.pos = pos;
  data.oldName = oldName;
  QVariant v;
  v.setValue<MenuData>(data);
  act->setData(v);
}

void
Parser::updateInvalidPropertyValueContextMenu(QMultiMap<int, QString> matches,
                                              PropertyNode* property,
                                              const QString& valueName,
                                              const QPoint& pos,
                                              QMenu** suggestionsMenu)
{
  (*suggestionsMenu)->clear();

  auto keys = matches.keys(valueName);
  for (auto key : keys) {
    matches.remove(key, valueName);
  }

  auto act = getWidgetAction(
    m_datastore->invalidIcon(),
    m_editor
      ->tr("%1 is an invalid value for %2.<br>Possible suggestions are : ")
      .arg(valueName)
      .arg(property->name()),
    *suggestionsMenu);
  (*suggestionsMenu)->addAction(act);
  (*suggestionsMenu)->addSeparator();

  updateMenu(matches,
             property,
             pos,
             suggestionsMenu,
             SectionType::PropertyValue,
             valueName);
}

void
Parser::updateInvalidNameAndPropertyValueContextMenu(
  QMultiMap<int, QPair<QString, QString>> matches,
  PropertyNode* nNode,
  const QString& valueName,
  const QPoint& pos,
  QMenu** suggestionsMenu)
{
  (*suggestionsMenu)->clear();

  (*suggestionsMenu)
    ->addSection(m_datastore->invalidIcon(),
                 m_editor->tr("%1 is not a valid property value for %2")
                   .arg(valueName)
                   .arg(nNode->name()));
  if (matches.isEmpty()) {
    (*suggestionsMenu)
      ->addSection(m_datastore->noIcon(),
                   m_editor->tr("No suggestions are available!"));
    return;
  }

  auto reversed = sortLastNValues(matches);

  QString s("%1 : %2");
  for (auto pair : reversed) {
    auto act = new QAction(s.arg(pair.first).arg(pair.second));
    (*suggestionsMenu)->addAction(act);
    QVariant v;
    v.setValue(qMakePair<Node*, QPoint>(nNode, pos));
    act->setData(v);
    m_editor->connect(
      act, &QAction::triggered, m_editor, &StylesheetEditor::suggestionMade);
  }

  if (reversed.size() > 0) {
    (*suggestionsMenu)->setEnabled(true);
  }
}

void
Parser::updateMenu(QMap<int, QString> matches,
                   Node* nNode,
                   const QPoint& pos,
                   QMenu** suggestionsMenu,
                   SectionType type,
                   const QString& oldName)
{
  QAction* act;

  if (matches.isEmpty()) {
    (*suggestionsMenu)
      ->addSection(m_datastore->noIcon(),
                   m_editor->tr("No suggestions are available!"));
    return;
  }

  auto reversed = reverseLastNValues(matches);

  for (auto key : reversed) {
    act = new QAction(matches.value(key));
    (*suggestionsMenu)->addAction(act);
    setMenuData(act, nNode, pos, type, oldName);
    m_editor->connect(
      act, &QAction::triggered, m_editor, &StylesheetEditor::suggestionMade);
  }

  if (reversed.size() > 0) {
    (*suggestionsMenu)->setEnabled(true);
  }
}

QList<int>
Parser::reverseLastNValues(QMultiMap<int, QString> matches)
{
  QMultiMap<int, QString> rMatches;
  auto keys = matches.keys();
  QList<int> reversed;
  QList<int>::reverse_iterator i;

  for (i = keys.rbegin(); i != keys.rend(); ++i) {
    if (reversed.size() > m_datastore->maxSuggestionCount()) {
      break;
    }

    reversed << *i;
  }

  return reversed;
}

QList<QPair<QString, QString>>
Parser::sortLastNValues(QMultiMap<int, QPair<QString, QString>> matches)
{
  QMultiMap<int, QPair<QString, QString>> rMatches;
  QList<QPair<QString, QString>> sorted;
  for (auto v : matches.values()) {
    auto f = v.first;
    auto s = v.second;
    auto lf = f.length();
    if (sorted.size() == 0) {
      sorted.append(v);
      continue;
    } else {
      bool success = false;
      for (int i = 0; i < sorted.length(); i++) {
        if (i == m_datastore->maxSuggestionCount())
          return sorted;
        auto spair = sorted.at(i);
        if (spair.first.length() > lf) {
          sorted.insert(i, v);
          success = true;
        }
      } // end for
      if (!success) {
        sorted.append(v);
      }
      continue;
    }
  } // end for
  return sorted;
}

StylesheetData*
Parser::getStylesheetProperty(const QString& sheet, int& pos)
{
  QString property, sep, value;
  StylesheetData* data = nullptr;

  property = findNext(sheet, pos);

  if (m_datastore->containsStylesheetProperty(property)) {
    data = new StylesheetData();
    data->name = property;

    if (pos < sheet.length()) {
      sep = findNext(sheet, pos);

      if (sep == ":") {
        while (true) {
          value = findNext(sheet, pos);

          if (value == ";" || value == "}") {
            break;
          }

          m_datastore->ifValidStylesheetValue(property, value, data);
        }

      } else {
        // TODO show stylesheet error - no property seperator.
      }
    }
  }

  return data;
}

void
Parser::handleDocumentChanged(int offset, int charsRemoved, int charsAdded)
{
  // If the text is changed due to a suggestion then ignore
  // it as is is handled elsewhere.
  if (m_datastore->hasSuggestion()) {
    m_datastore->setHasSuggestion(false);
    return;
  }

  if (m_initialising) {
    return;
  }

  //  auto charChange = charsAdded - charsRemoved;
  if (m_datastore->isNodesEmpty()) {
    // If no text has been loaded but text has been added then asssume modifying
    // a clean new stylesheet and handle changes normally.
    if (charsAdded == 0)
      // initial text has not yet been parsed.
      return;
  }

  QString text = m_editor->toPlainText();

  if (text.isEmpty()) {
    return;
  }

  CursorData data = getNodeAtCursor(offset);
  Node* node = data.node;

  if (node) {
    //    QString value;

    switch (node->type()) {
      case NodeType::PropertyType: {
        auto property = qobject_cast<PropertyNode*>(node);

        if (property) {
          auto status = property->partAtOffset(offset);
          if (status.status() == SectionType::PropertyName) {
            int length = property->name().length() + charsAdded - charsRemoved;
            auto newName = text.mid(property->start(), length);
            property->setName(newName);
            if (m_datastore->containsProperty(newName)) {
              property->setValidPropertyName(true);
            } else {
              property->setValidPropertyName(false);
            }
            emit rehighlightBlock(data.cursor.block());
          } else if (status.status() == SectionType::PropertyValue) {
            int index = property->values().indexOf(status.name());
            int length = status.name().length() + charsAdded - charsRemoved;
            auto newName = text.mid(property->valuePosition(index), length);
            property->setValue(index, newName);
            if (m_datastore->isValidPropertyValueForProperty(property->name(),
                                                             newName)) {
              property->setCheck(index, PropertyValueCheck::GoodValue);
            } else {
              property->setCheck(index, PropertyValueCheck::BadValue);
            }
            emit rehighlightBlock(data.cursor.block());
          }
        }
      }
    }
  }
}

void
Parser::handleCursorPositionChanged(QTextCursor textCursor)
{
  if (m_datastore->isManualMove()) {
    return;
  }

  setCurrentCursor(textCursor);

  Node* node;
  CursorData data;
  data.cursor = textCursor;
  nodeAtCursorPosition(&data, textCursor.anchor());
  node = data.node;
  if (!node) {
    return;
  }

  // TODO move this capability into node/datastore.
  //  for (auto startbrace : m_datastore->startBraces()) {
  //    if (startbrace->isBraceAtCursor()) {
  //      startbrace->setBraceAtCursor(false);
  //    }
  //  }

  //  for (auto endbrace : d_ptr->endbraces) {
  //    if (endbrace->isBraceAtCursor()) {
  //      endbrace->setBraceAtCursor(false);
  //    }
  //  }

  //  if (node) {
  //    auto type = node->type();

  //    if (type == NodeType::StartBraceType) {
  //      StartBraceNode* startbrace = qobject_cast<StartBraceNode*>(node);
  //      startbrace->setBraceAtCursor(true);
  //      if (startbrace->hasEndBrace()) {
  //        EndBraceNode* endbrace = startbrace->endBrace();
  //        endbrace->setBraceAtCursor(true);
  //      }
  //    } else if (type == NodeType::EndBraceType) {
  //      EndBraceNode* endbrace = qobject_cast<EndBraceNode*>(node);
  //      endbrace->setBraceAtCursor(true);
  //      if (endbrace->hasStartBrace()) {
  //        StartBraceNode* startbrace = endbrace->startBrace();
  //        startbrace->setBraceAtCursor(true);
  //      }
  //    } // end end brace type
  //  }   // end if node

  emit rehighlight();
}

QMenu*
Parser::handleMouseClicked(const QPoint& pos)
{
  NodeSection* section = nullptr;
  nodeForPoint(pos, &section);

  QMenu* suggestionsMenu = new QMenu(tr("&Suggestions"));

  if (section && section->node && section->node != m_datastore->currentNode()) {
    switch (section->node->type()) {

      case NodeType::WidgetType: {
        auto widget = qobject_cast<WidgetNode*>(section->node);
        if (!widget->isWidgetValid()) {
          //          // not a valid node
          //          if (widget != m_datastore->currentNode()) {
          //            auto matches =
          //              m_datastore->fuzzySearchWidgets(section->node->name());
          //            updateContextMenu(matches, widget, pos,
          //            &suggestionsMenu);
          //          }
          switch (section->type) {
            case SectionType::WidgetPropertyMarker: {
              qWarning();
              // TODO marker errors? maybe :: or ;
              break;
            }
            case SectionType::WidgetPseudoStateMarker: {
              qWarning();
              // TODO marker errors? maybe :: or ;
              break;
            }
            case SectionType::WidgetSubControlMarker: {
              qWarning();
              // TODO marker errors? maybe :: or ;
              break;
            }
          }
        }
        suggestionsMenu->setTitle(
          m_editor->tr("%1 is appears to be a valid QWidget name")
            .arg(widget->name()));
        break;
      } // end WidgetType

      case NodeType::PropertyType: {
        auto property = qobject_cast<PropertyNode*>(section->node);
        switch (section->type) {
          case SectionType::WidgetName: {
            if (m_datastore->containsProperty(property->name())) {
              updatePropertyContextMenu(property, pos, &suggestionsMenu);
            } else {
              auto matches = m_datastore->fuzzySearchProperty(property->name());
              updatePropertyContextMenu(
                property, pos, &suggestionsMenu, matches);
            }
            break;
          }
          case SectionType::PropertyName: {
            if (property->isValidPropertyName()) {
              updatePropertyContextMenu(property, pos, &suggestionsMenu);
            } else {
              auto matches = m_datastore->fuzzySearchProperty(property->name());
              updatePropertyContextMenu(
                property, pos, &suggestionsMenu, matches);
            }
            break;
          }
          case SectionType::PropertyValue: {
            auto valName = property->value(section->position);
            if (property->isValidPropertyName()) {
              if (property->isValueValid(section->position)) {
                // must have a valid property to check value types.
                auto matches = m_datastore->fuzzySearchPropertyValue(
                  property->name(), valName);
                updateValidPropertyValueContextMenu(
                  matches,
                  property,
                  valName,
                  pos,
                  &suggestionsMenu); //                suggestionsMenu->setEnabled(true);
              } else {
                auto matches = m_datastore->fuzzySearchPropertyValue(
                  property->name(), valName);
                QMultiMap<int, QPair<QString, QString>> propValMatches;
                updateInvalidPropertyValueContextMenu(
                  matches, property, valName, pos, &suggestionsMenu);
                // TODO
              }
            } else {
              auto matches = m_datastore->fuzzySearchProperty(property->name());
              QMultiMap<int, QPair<QString, QString>> propValMatches;
              for (auto name : matches) {
                auto vMatches =
                  m_datastore->fuzzySearchPropertyValue(name, valName);
                for (auto key : vMatches.keys()) {
                  propValMatches.insert(
                    key,
                    qMakePair<QString, QString>(name, vMatches.value(key)));
                }
              }
              updateInvalidNameAndPropertyValueContextMenu(
                propValMatches, property, valName, pos, &suggestionsMenu);
            }
            break;
          }
          case SectionType::Comment: {
            qWarning();
            break;
          }
          case SectionType::PropertyMarker: {
            qWarning();
            // TODO marker errors? maybe :: or ;
            break;
          }
        } // end switch type
        break;
      } // end PropertyType

      case NodeType::BadNodeType: {
        // TODO probably remove this.
        break;
      } // end case Node::BadNodeType

      default: {
        break;
      }
    }
  }
  return suggestionsMenu;
}

QTextCursor
Parser::currentCursor() const
{
  return m_datastore->currentCursor();
}

void
Parser::setCurrentCursor(const QTextCursor& currentCursor)
{
  m_datastore->setCurrentCursor(currentCursor);
}

void
Parser::setMaxSuggestionCount(int maxSuggestionCount)
{
  m_datastore->setMaxSuggestionCount(maxSuggestionCount);
}

void
Parser::updateTextChange(QTextCursor& cursor,
                         const QString& oldName,
                         const QString& newName)
{
  cursor.movePosition(QTextCursor::Right,
                      QTextCursor::KeepAnchor,
                      oldName.length()); // selection
  cursor.insertText(newName);
  cursor.movePosition(
    QTextCursor::Left, QTextCursor::MoveAnchor, newName.length());
}

void
Parser::actionPropertyNameChange(PropertyNode* property, const QString& newName)
{
  auto cursor = property->cursor();
  auto oldName = property->name();

  updateTextChange(cursor, oldName, newName);

  property->setValidPropertyName(true);
  property->setName(newName);
  property->setCursor(cursor);
  emit rehighlightBlock(cursor.block());
}

void
Parser::actionPropertyValueChange(PropertyNode* property,
                                  const QString& oldName,
                                  const QString& newName)
{
  auto index = property->values().indexOf(oldName);
  auto cursor = property->valueCursors().at(index);

  updateTextChange(cursor, oldName, newName);

  property->setValue(index, newName);
  property->setCheck(index, PropertyValueCheck::GoodValue);
  property->setValueCursor(index, cursor);
  emit rehighlightBlock(property->cursor().block());
}

void
Parser::actionPropertyMarker(PropertyNode* property)
{
  QTextCursor cursor = property->cursor();
  cursor.movePosition(QTextCursor::Right,
                      QTextCursor::MoveAnchor,
                      property->name().length()); // selection
  cursor.insertText(":");
  property->setPropertyMarkerCursor(cursor);
  property->setPropertyMarker(true);
  emit rehighlightBlock(cursor.block());
}

void
Parser::actionPropertyEndMarker(PropertyNode* property)
{
  QTextCursor cursor = property->cursor();
  cursor.movePosition(QTextCursor::Right,
                      QTextCursor::MoveAnchor,
                      property->length()); // selection
  cursor.insertText(";");
  property->setPropertyEndMarkerCursor(cursor);
  property->setPropertyEndMarker(true);
  emit rehighlightBlock(cursor.block());
}

void
Parser::handleSuggestions(QAction* act)
{
  QVariant v = act->data();
  auto menuData = v.value<MenuData>();
  auto nNode = menuData.node;
  auto pos = menuData.pos;
  auto type = menuData.type;
  auto oldName = menuData.oldName;

  if (nNode) {
    auto newName = act->text();

    switch (type) {
      case WidgetName:
      case WidgetPseudoState:
      case WidgetPseudoStateMarker:
      case WidgetSubControl:
      case WidgetSubControlMarker:
      case WidgetPropertyName:
      case WidgetPropertyValue:
      case WidgetPropertyMarker:
      case WidgetPropertyEndMarker:
      case WidgetStartBrace:
      case WidgetEndBrace:
        break;

      case Comment:
        break;

      case SectionType::PropertyName: {
        m_datastore->setHasSuggestion(true);
        actionPropertyNameChange(qobject_cast<PropertyNode*>(nNode), newName);
        break;
      }
      case SectionType::PropertyValue: {
        m_datastore->setHasSuggestion(true);
        actionPropertyValueChange(
          qobject_cast<PropertyNode*>(nNode), oldName, newName);
        break;
      }
      case SectionType::PropertyMarker: {
        m_datastore->setHasSuggestion(true);
        actionPropertyMarker(qobject_cast<PropertyNode*>(nNode));
        break;
      }
      case SectionType::PropertyEndMarker: {
        m_datastore->setHasSuggestion(true);
        actionPropertyEndMarker(qobject_cast<PropertyNode*>(nNode));
        break;
      }
    }

    emit rehighlight();
  }
}

int
Parser::maxSuggestionCount() const
{
  return m_datastore->maxSuggestionCount();
}
