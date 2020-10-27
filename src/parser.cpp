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
#include "stylesheetparser/stylesheetedit.h"

#include <QtDebug>

Parser::Parser(StylesheetEdit* parent)
  : QObject(parent)
  , m_editor(parent)
  , m_datastore(new DataStore(parent))
  , m_braceCount(0)
  , m_nodes(new QMap<QTextCursor, Node*>())
  , m_startComment(false)
  , m_manualMove(false)
  , m_maxSuggestionCount(30)
{
  m_formatAct = new QAction(m_editor->tr("&Format"), m_editor);
  m_formatAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
  m_formatAct->setStatusTip(m_editor->tr("Prettyfy the stylesheet"));
  m_editor->connect(
    m_formatAct, &QAction::triggered, m_editor, &StylesheetEdit::format);

  m_contextMenu = createContextMenu();
}

Parser::~Parser()
{
  emit finished();
}

void
Parser::parseInitialText(const QString& text, int pos)
{
  m_braceCount = 0;
  QString block;
  int start;

  while (true) {
    if ((block = findNext(text, pos)).isEmpty()) {
      break;
    }

    start = pos - block.length();
    QTextCursor cursor;

    if (m_datastore->containsWidget(block)) {
      stashWidget(start, block);

      while (!(block = findNext(text, pos)).isEmpty()) {
        start = pos - block.length();

        if (block == "/*") { // comment start
          parseComment(text, start);

        } else if (block == "::") { // subcontrol
          if (m_braceCount == 0) {
            stashSubControlMarker(start);

            if (!(block = findNext(text, pos)).isEmpty()) {
              if (m_datastore->containsSubControl(block)) {
                stashSubControl(pos - block.length(), block);

              } else {
                stashSubControl(pos - block.length(), block, false);
                break;
              }
            }
          }

        } else if (block == ":") { // pseudostate or property
          if (m_braceCount == 0) {
            stashPseudoStateMarker(start);

            if (!(block = findNext(text, pos)).isEmpty()) {
              if (m_datastore->containsPseudoState(block)) {
                stashPseudoState(pos - block.length(), block);

              } else {
                stashPseudoState(pos - block.length(), block, false);
                break;
              }
            }

          } else {
            Node* endnode = nullptr;
            cursor = getCursorForNode(start);
            PropertyNode* property = new PropertyNode(block, cursor, m_editor);
            m_nodes->insert(cursor, property);
            int end = parsePropertyWithValues(
              cursor, property, text, start, pos, block, &endnode);

            // run out of text.
            if (!endnode) {
              QTextCursor cursor = getCursorForNode(end);
              endnode = new PropertyEndNode(cursor, m_editor);
              m_nodes->insert(cursor, endnode);
            }
          }

        } else if (block == "{") { // start of widget block
          stashStartBrace(pos - block.length());

        } else if (block == "}") { // end of widget block
          stashEndBrace(pos - block.length());
          break;

        } else if (m_datastore->containsProperty(block)) {
          Node* endnode = nullptr;
          cursor = getCursorForNode(start);
          PropertyNode* property = new PropertyNode(block, cursor, m_editor);
          m_nodes->insert(cursor, property);
          parsePropertyWithValues(
            cursor, property, text, start, pos, block, &endnode);

        } else {
          if (m_braceCount == 1) {
            int oldPos = pos;
            QString oldBlock = block;

            if (!(block = findNext(text, pos)).isEmpty()) {
              if (block == ":") {
                Node* endnode = nullptr;
                pos = oldPos;
                cursor = getCursorForNode(start);
                PropertyNode* property =
                  new PropertyNode(oldBlock, cursor, m_editor);
                property->setPropertyMarkerExists(true);
                property->setPropertyMarkerOffset(pos);
                m_nodes->insert(cursor, property);
                int end = parsePropertyWithValues(
                  cursor, property, text, start, pos, block, &endnode);

                // run out of text.
                if (!endnode) {
                  QTextCursor cursor = getCursorForNode(end);
                  endnode = new PropertyEndNode(cursor, m_editor);
                  m_nodes->insert(cursor, endnode);
                }
              }

            } else {
              stashBadNode(
                pos - block.length(), block, ParserState::InvalidPropertyName);
            }
          }
        }
      }

    } else if (block == "/*") { // comment start
      parseComment(text, pos);

    } else if (block == "::") { // comment start
      int prevPos = pos;

      if (m_datastore->containsSubControl(block)) {
        stashSubControlMarker(prevPos);
        stashSubControl(pos - block.length(), block);

      } else {
        stashBadSubControlMarkerNode(prevPos - 1,
                                     ParserState::InvalidSubControlMarker);
        pos = prevPos;
      }

    } else if (block == ":") { // comment start
      int prevPos = pos;

      if (!(block = findNext(text, pos)).isEmpty()) {
        if (m_datastore->containsPseudoState(block)) {
          stashPseudoStateMarker(prevPos);
          stashPseudoState(pos - block.length(), block);

        } else {
          stashBadPseudoStateMarkerNode(prevPos - 1,
                                        ParserState::InvalidPseudoStateMarker);
          // revert to prev position and ignore last find.
          pos = prevPos;
        }
      }

    } else if (block == "{") {
      stashStartBrace(pos - block.length());

    } else if (block == "}") {
      stashEndBrace(pos - block.length());

    } else if (m_datastore->containsProperty(block)) {
      Node* endnode = nullptr;
      cursor = getCursorForNode(start);
      PropertyNode* property = new PropertyNode(block, cursor, m_editor);
      m_nodes->insert(cursor, property);
      int end = parsePropertyWithValues(
        cursor, property, text, start, pos, block, &endnode);

      // run out of text.
      if (!endnode) {
        cursor = getCursorForNode(end);
        endnode = new PropertyEndNode(cursor, m_editor);
        m_nodes->insert(cursor, endnode);
      }

    } else {
      QString nextBlock;

      if (!m_nodes->isEmpty()) {
        int oldPos = pos;
        nextBlock = findNext(text, pos);

        if (nextBlock == ":") {
          nextBlock = findNext(text, pos);

          if (m_datastore->containsPseudoState(nextBlock)) {
            stashWidget(start, block, false);
            stashPseudoStateMarker(oldPos); // correct last node
            stashPseudoState(pos - nextBlock.length(), nextBlock);
            continue;

          } else if (m_datastore->propertyValueAttribute(nextBlock) !=
                     DataStore::NoAttributeValue) {
            Node* endnode = nullptr;
            cursor = getCursorForNode(start);
            PropertyNode* property = new PropertyNode(block, cursor, m_editor);
            property->setPropertyMarkerExists(true);
            property->setValidProperty(false);
            m_nodes->insert(cursor, property);
            pos -= block.length(); // step back
            int end = parsePropertyWithValues(
              cursor, property, text, start, pos, block, &endnode);

            // run out of text.
            if (!endnode) {
              cursor = getCursorForNode(end);
              endnode = new PropertyEndNode(cursor, m_editor);
              m_nodes->insert(cursor, endnode);
            }

            continue;
          }

        } else if (nextBlock == "::") {
          nextBlock = findNext(text, pos);

          if (m_datastore->containsSubControl(nextBlock)) {
            stashWidget(start, block, false);
            stashSubControlMarker(oldPos);
            stashSubControl(pos - nextBlock.length(), block);
            continue;
          }
        }

        // step back
        pos = oldPos;
        Node* lastnode = m_nodes->last();
        Node::Type lasttype = lastnode->type();

        if (lasttype == Node::BadSubControlMarkerType) {
          if (m_datastore->containsSubControl(block)) {
            stashSubControlMarker(lastnode->start());
            stashSubControl(start, block);

          } else {
            stashSubControlMarker(lastnode->start());
            stashSubControl(start, block, false);
          }

        } else {
          stashWidget(start, block, false);
        }

      } else { // anomalous type - see what comes next.
        int oldPos = pos;
        nextBlock = findNext(text, pos);

        if (nextBlock == ":") {
          nextBlock = findNext(text, pos);

          if (m_datastore->containsPseudoState(nextBlock)) {
            stashWidget(start, block, false);
            stashPseudoStateMarker(oldPos); // correct last node
            stashPseudoState(pos - nextBlock.length(), nextBlock);
            continue;

          } else if (m_datastore->propertyValueAttribute(nextBlock) !=
                     DataStore::NoAttributeValue) {
            Node* endnode = nullptr;
            cursor = getCursorForNode(start);
            PropertyNode* property = new PropertyNode(block, cursor, m_editor);
            property->setPropertyMarkerExists(true);
            property->setValidProperty(false);
            m_nodes->insert(cursor, property);
            pos -= block.length(); // step back
            int end = parsePropertyWithValues(
              cursor, property, text, start, pos, block, &endnode);

            // run out of text.
            if (!endnode) {
              cursor = getCursorForNode(end);
              endnode = new PropertyEndNode(cursor, m_editor);
              m_nodes->insert(cursor, endnode);
            }

            continue;
          }

        } else if (nextBlock == "::") {
          nextBlock = findNext(text, pos);

          if (m_datastore->containsSubControl(nextBlock)) {
            stashWidget(start, block, false);
            stashSubControlMarker(oldPos);
            stashSubControl(pos - nextBlock.length(), block);
            continue;
          }
        }

        stashBadNode(start, block, ParserState::AnomalousType);
      }
    }
  }

  emit rehighlight();
}

QMap<QTextCursor, Node*>*
Parser::nodes() const
{
  return m_nodes;
}

int
Parser::parsePropertyWithValues(QTextCursor cursor,
                                PropertyNode* property,
                                const QString& text,
                                int start,
                                int& pos,
                                QString& block,
                                Node** endnode)
{
  QString propertyName = property->name();

  while (!(block = findNext(text, pos)).isEmpty()) {
    if (block == ":") {
      if (!property->hasPropertyMarker()) {
        property->setPropertyMarkerExists(true);
        property->setPropertyMarkerOffset(pos);
      }

    } else if (block == "/*") { // comment start
      parseComment(text, pos);

    } else if (block == ";") {
      stashPropertyEndMarkerNode(property->end(), endnode);
      break;

    } else if (block == "}") {
      if (!(*endnode)) { // already a property end node (;)
        stashPropertyEndNode(property->end(), endnode);
      }

      stashEndBrace(cursor.anchor());

      pos--; // step back from endbrace
      break;

    } else {
      bool validForProperty =
        m_datastore->isValidPropertyValueForProperty(propertyName, block);
      DataStore::AttributeType attributeType =
        m_datastore->propertyValueAttribute(block);

      if (validForProperty) {
        // valid property and valid value.
        property->addValue(block,
                           PropertyNode::GoodValue,
                           (pos - block.length()) - start,
                           attributeType);

      } else {

        if (attributeType == DataStore::NoAttributeValue) {
          // not a valid value for any property
          if (m_datastore->containsProperty(block)) {
            Node* lastnode = m_nodes->last();

            if (!(lastnode->type() == Node::PropertyEndType ||
                  lastnode->type() == Node::PropertyEndMarkerType)) {
              // the block is actually another property. Probably a missing ';'.
              // set the last check value to missing property end.
              property->setBadCheck(PropertyNode::MissingPropertyEnd);
              stashPropertyEndNode(property->end(), endnode);
            }

            pos -= block.length(); // skip back before block.
            return property->end();

          } else {
            property->addValue(block,
                               PropertyNode::BadValue,
                               (pos - block.length()) - start,
                               attributeType);
          }

        } else {
          // invalid property name but this is a valid property attribute
          // anyway.
          property->addValue(block,
                             PropertyNode::ValidPropertyType,
                             (pos - block.length()) - start,
                             attributeType);
        }
      }
    }
  }

  return property->end();
}

void
Parser::parseComment(const QString& text, int& pos)
{
  m_startComment = true;
  QTextCursor cursor = getCursorForNode(pos);
  Node* marker = new StartCommentNode(cursor, m_editor);
  m_nodes->insert(cursor, marker);
  pos += 2;

  QChar c;
  CommentNode* comment = new CommentNode(getCursorForNode(pos), m_editor);
  m_nodes->insert(cursor, comment);

  while (pos < text.length()) {
    c = text.at(pos++);

    if (c != '*') {
      comment->append(c);

    } else {
      if (pos < text.length() - 1) {
        c = text.at(pos++);

        if (c == '/') {
          cursor = getCursorForNode(pos - 2);
          EndCommentNode* endcomment = new EndCommentNode(cursor, m_editor);
          m_nodes->insert(cursor, endcomment);
          break;

        } else {
          comment->append(c);
        }
      }
    }
  }
}

QString
Parser::findNext(const QString& text, int& pos)
{
  QString block;
  QChar c;
  skipBlanks(text, pos);

  if (pos < text.length()) {
    c = text.at(pos);

    while (true) {
      if (c.isNull() || pos >= text.length()) {
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

    if (c.isSpace() /* || c == '\n' || c == '\r' || c == '\t'*/) {
      continue;

    } else {
      break;
    }
  }
}

QTextCursor
Parser::getCursorForNode(int position)
{
  QTextCursor c(m_editor->document());
  c.movePosition(QTextCursor::Start);
  c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, position);
  return c;
}

QPair<NodeSectionType, int>
Parser::nodeForPoint(const QPoint& pos, NamedNode** nNode)
{
  QPair<NodeSectionType, int> isin =
    qMakePair<NodeSectionType, int>(NodeSectionType::None, -1);

  for (auto node : m_nodes->values()) {
    *nNode = qobject_cast<NamedNode*>(node);

    if (*nNode) {
      isin = (*nNode)->isIn(pos);

      if (isin.first != NodeSectionType::None) {
        return isin;
      }
    }
  }

  return isin;
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

  for (auto key : m_nodes->keys()) {
    Node* node = m_nodes->value(key);

    if (!node) {
      return;
    }

    //    auto start = node->start();
    //    auto end = node->end();

    if (position > node->end()) {
      previous = node;
      continue;

    } else if (position >= node->start() && position <= node->end()) {
      data->node = node;
      data->cursor = node->cursor();
      data->prevNode = previous;
      break;
    }

    // TODO add property value hovers.
  }
}

void
Parser::stashWidget(int position, const QString& block, bool valid)
{
  auto cursor = getCursorForNode(position);
  auto widgetnode = new WidgetNode(block, cursor, m_editor);
  widgetnode->setWidgetValid(valid);
  m_nodes->insert(cursor, widgetnode);
}

void
Parser::stashBadNode(int position,
                     const QString& block,
                     ParserState::Error error)
{
  auto cursor = getCursorForNode(position);
  auto badblock = new BadBlockNode(block, cursor, error, m_editor);
  m_nodes->insert(cursor, badblock);
}

void
Parser::stashBadSubControlMarkerNode(int position, ParserState::Error error)
{
  auto cursor = getCursorForNode(position);
  auto badblock = new BadSubControlMarkerNode(cursor, error, m_editor);
  m_nodes->insert(cursor, badblock);
}

void
Parser::stashBadPseudoStateMarkerNode(int position, ParserState::Error error)
{
  auto cursor = getCursorForNode(position);
  auto badblock = new BadPseudoStateMarkerNode(cursor, error, m_editor);
  m_nodes->insert(cursor, badblock);
}

void
Parser::stashPseudoState(int position, const QString& block, bool valid)
{
  auto cursor = getCursorForNode(position);
  auto pseudostate = new PseudoStateNode(block, cursor, m_editor);
  pseudostate->setStateValid(valid);
  m_nodes->insert(cursor, pseudostate);
}

void
Parser::stashSubControl(int position, const QString& block, bool valid)
{
  auto cursor = getCursorForNode(position);
  auto subcontrol = new SubControlNode(block, cursor, m_editor);
  subcontrol->setStateValid(valid);
  m_nodes->insert(cursor, subcontrol);
}

void
Parser::stashEndBrace(int position)
{
  m_braceCount--;
  auto cursor = getCursorForNode(position);
  auto brace = new EndBraceNode(cursor, m_editor);
  m_nodes->insert(cursor, brace);
  m_endbraces.append(brace);
}

void
Parser::stashStartBrace(int position)
{
  m_braceCount++;
  auto cursor = getCursorForNode(position);
  auto brace = new StartBraceNode(cursor, m_editor);
  m_nodes->insert(cursor, brace);
  m_startbraces.append(brace);
}

void
Parser::stashPseudoStateMarker(int position)
{
  auto cursor = getCursorForNode(position);
  auto marker = new PseudoStateMarkerNode(cursor, m_editor);
  m_nodes->insert(cursor, marker);
}

void
Parser::stashSubControlMarker(int position)
{
  auto cursor = getCursorForNode(position);
  auto marker = new SubControlMarkerNode(cursor, m_editor);
  m_nodes->insert(cursor, marker);
}

void
Parser::stashPropertyEndNode(int position, Node** endnode)
{
  QTextCursor cursor = getCursorForNode(position);
  *endnode = new PropertyEndNode(cursor, m_editor);
  m_nodes->insert(cursor, *endnode);
}

void
Parser::stashPropertyEndMarkerNode(int position, Node** endnode)
{
  QTextCursor cursor = getCursorForNode(position);
  *endnode = new PropertyEndMarkerNode(cursor, m_editor);
  m_nodes->insert(cursor, *endnode);
}

void
Parser::updatePropertyValues(int pos,
                             PropertyNode* property,
                             int charsAdded,
                             int charsRemoved,
                             const QString& newValue)
{
  bool updated = false;
  auto values = property->values();
  auto offsets = property->offsets();
  auto checks = property->checks();
  auto attributes = property->attributeTypes();

  for (int i = 0; i < offsets.size(); i++) {
    auto offset = offsets[i];
    auto value = values[i];
    auto start = property->start() + offset;
    auto end = start + value.length();

    if (updated) {
      offsets[i] += charsAdded;
      offsets[i] -= charsRemoved;
      continue;
    }

    if (pos >= start && pos < end) {
      values.replace(i, newValue);

      if (m_datastore->isValidPropertyValueForProperty(property->name(),
                                                       newValue)) {
        checks.replace(i, PropertyNode::GoodValue);

      } else {
        DataStore::AttributeType attribute = DataStore::NoAttributeValue;

        if ((attribute = m_datastore->propertyValueAttribute(newValue)) !=
            DataStore::NoAttributeValue) {
          attributes[i] = attribute;
        }
      }

      updated = true;
    }
  }

  property->setValues(values);
  property->setChecks(checks);
  property->setOffsets(offsets);
  property->setAttributeTypes(attributes);
}

QMenu*
Parser::createContextMenu()
{
  QMenu* menu = m_editor->createStandardContextMenu();
  menu->addSeparator();
  m_suggestionsMenu = menu->addMenu(m_editor->tr("&Suggestions"));
  menu->addSeparator();
  menu->addAction(m_formatAct);

  m_suggestionsMenu->setEnabled(false);

  return menu;
}

void
Parser::updateContextMenu(QMap<int, QString> matches,
                          NamedNode* nNode,
                          const QPoint& pos)
{
  QString typeName;
  m_suggestionsMenu->clear();

  switch (nNode->type()) {
    case Node::WidgetType:
      typeName = "widget";
      break;

    case Node::PseudoStateType:
      typeName = "pseudo state";
      break;

    case Node::SubControlType:
      typeName = "sub control";
      break;
  }

  QAction* act = new QAction(
    m_editor->tr("%1 is not a valid %2 name").arg(nNode->name()).arg(typeName));
  act->setData(pos);
  m_suggestionsMenu->addAction(act);
  m_suggestionsMenu->addSeparator();

  updateMenu(matches, nNode, pos);
}

void
Parser::updatePropertyContextMenu(QMap<int, QString> matches,
                                  PropertyNode* property,
                                  const QPoint& pos)
{
  QString typeName;
  QAction* act;
  m_suggestionsMenu->clear();

  if (property->hasPropertyMarker()) {
    act = new QAction(m_editor->tr("%1 is not a valid property name")
                        .arg(property->name())
                        .arg(typeName));
    act->setData(pos);
    m_suggestionsMenu->addAction(act);
    m_suggestionsMenu->addSeparator();
    updateMenu(matches, property, pos);

  } else {
    act = new QAction(m_editor->tr("%1 is missing a property marker")
                        .arg(property->name())
                        .arg(typeName));
    act->setData(pos);
    m_suggestionsMenu->addAction(act);
    m_suggestionsMenu->addSeparator();
    m_addPropertyMarkerAct =
      new QAction(m_editor->tr("Add property marker (:)"));
    m_suggestionsMenu->addAction(m_addPropertyMarkerAct);
    QVariant v;
    v.setValue(qMakePair<NamedNode*, QPoint>(property, pos));
    m_addPropertyMarkerAct->setData(v);
    m_editor->connect(m_addPropertyMarkerAct,
                      &QAction::triggered,
                      m_editor,
                      &StylesheetEdit::suggestion);
    m_suggestionsMenu->setEnabled(true);
  }
}

void
Parser::updatePropertyValueContextMenu(QMultiMap<int, QString> matches,
                                       PropertyNode* nNode,
                                       const QString& valueName,
                                       const QPoint& pos)
{
  m_suggestionsMenu->clear();

  QAction* act =
    new QAction(m_editor->tr("%1 is not a valid property value for %2")
                  .arg(valueName)
                  .arg(nNode->name()));
  m_suggestionsMenu->addAction(act);
  m_suggestionsMenu->addSeparator();

  updateMenu(matches, nNode, pos);
}

void
Parser::updatePropertyValueContextMenu(
  QMultiMap<int, QPair<QString, QString>> matches,
  PropertyNode* nNode,
  const QString& valueName,
  const QPoint& pos)
{
  m_suggestionsMenu->clear();

  QAction* act =
    new QAction(m_editor->tr("%1 is not a valid property value for %2")
                  .arg(valueName)
                  .arg(nNode->name()));
  m_suggestionsMenu->addAction(act);
  m_suggestionsMenu->addSeparator();

  if (matches.size() == 0) {
    act = new QAction(m_editor->tr("No suggestions are available!"));
    m_suggestionsMenu->addAction(act);
    return;
  }

  auto reversed = sortLastNValues(matches);

  QString s("%1 : %2");
  for (auto pair : reversed) {
    act = new QAction(s.arg(pair.first).arg(pair.second));
    m_suggestionsMenu->addAction(act);
    QVariant v;
    v.setValue(qMakePair<NamedNode*, QPoint>(nNode, pos));
    act->setData(v);
    m_editor->connect(
      act, &QAction::triggered, m_editor, &StylesheetEdit::suggestion);
  }

  if (reversed.size() > 0) {
    m_suggestionsMenu->setEnabled(true);
  }
}

void
Parser::updateMenu(QMap<int, QString> matches,
                   NamedNode* nNode,
                   const QPoint& pos)
{
  QAction* act;
  if (matches.size() == 0) {
    act = new QAction(m_editor->tr("No suggestions are available!"));
    m_suggestionsMenu->addAction(act);
    return;
  }

  auto reversed = reverseLastNValues(matches);

  for (auto key : reversed) {
    act = new QAction(matches.value(key));
    m_suggestionsMenu->addAction(act);
    QVariant v;
    v.setValue(qMakePair<NamedNode*, QPoint>(nNode, pos));
    act->setData(v);
    m_editor->connect(
      act, &QAction::triggered, m_editor, &StylesheetEdit::suggestion);
  }

  if (reversed.size() > 0) {
    m_suggestionsMenu->setEnabled(true);
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
    if (reversed.size() > m_maxSuggestionCount) {
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
        if (i == m_maxSuggestionCount)
          return sorted;
        auto spair = sorted.at(i);
        if (spair.first.length() > lf) {
          sorted.insert(i, v);
          success = true;
        }
      }
      if (!success) {
        sorted.append(v);
      }
      continue;
    }
  }
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

          m_datastore->getIfValidStylesheetValue(property, value, data);
        }

      } else {
        // TODO show stylesheet error - no property seperator.
      }
    }
  }

  return data;
}

void
Parser::handleDocumentChanged(int pos, int charsRemoved, int charsAdded)
{
  if (m_nodes->isEmpty()) {
    // initial text has not yet been parsed.
    return;
  }

  QString text = m_editor->toPlainText();

  if (text.isEmpty()) {
    return;
  }

  CursorData data = getNodeAtCursor(pos);
  Node* node = data.node;

  if (node) {
    QString value, newValue;

    switch (node->type()) {
        //    case Node::WidgetType: {
        //      WidgetNode* widget = qobject_cast<WidgetNode*>(node);
        //      value = widget->value();
        //      break;
        //    }

      case Node::PropertyType: {
        PropertyNode* property = qobject_cast<PropertyNode*>(node);
        QTextCursor cursor = m_editor->textCursor();
        cursor.select(QTextCursor::WordUnderCursor);
        newValue = cursor.selectedText();

        if (newValue == ";") {
          Node* next;

          if (property->hasPropertyMarker()) {
            next = nextNode(property->cursor());
            next = nextNode(next->cursor());

          } else {
            next = nextNode(property->cursor());
          }

          if (next->type() == Node::PropertyEndType) {
            PropertyEndMarkerNode* marker =
              new PropertyEndMarkerNode(next->cursor(), m_editor);
            m_nodes->insert(next->cursor(), marker);
            next->deleteLater();

            if (property->count() > 0) {
              auto checks = property->checks();
              auto check = checks.last();

              if (check == PropertyNode::MissingPropertyEnd) {
                checks.replace(checks.size() - 1, PropertyNode::GoodValue);
                property->setChecks(checks);
              }
            }
          }

        } else if (newValue == ":") {
          int start = node->start();
          int end = node->end();
          int anchor = cursor.anchor();

          if (anchor > start && anchor < end) {
            // is in property.
            int nameEnd = start + property->name().length();

            if (!property->hasPropertyMarker() && anchor >= nameEnd) {
              //            QTextCursor propCursor(m_editor->document());
              //            propCursor.setPosition(pos);
              //            PropertyMarkerNode* marker =
              //              new PropertyMarkerNode(propCursor, m_editor);
              //            m_nodes->insert(propCursor, marker);
              property->setPropertyMarkerExists(true);
              property->setPropertyMarkerOffset(pos);
            }
          }

        } else {
          updatePropertyValues(
            cursor.anchor(), property, charsAdded, charsRemoved, newValue);
        }

        break;
      }

      case Node::BadNodeType: {
        BadBlockNode* badNode = qobject_cast<BadBlockNode*>(node);
        value = badNode->name();
        ParserState::Errors errors = badNode->errors();

        if (errors.testFlag(ParserState::AnomalousType)) {
          QTextCursor cursor(badNode->cursor());
          cursor.select(QTextCursor::WordUnderCursor);
          newValue = cursor.selectedText();

          if (m_datastore->containsWidget(newValue)) {
            Node* widget =
              new WidgetNode(newValue, badNode->cursor(), m_editor);
            m_nodes->insert(badNode->cursor(), widget);
            Node* next = nextNode(badNode->cursor());

            if (next) {
              if (next->type() == Node::BadSubControlMarkerType) {
                Node* subcontrolmarker =
                  new SubControlMarkerNode(next->cursor(), m_editor);
                m_nodes->insert(next->cursor(), subcontrolmarker);
                next->deleteLater();

              } else if (next->type() == Node::BadPseudoStateMarkerType) {
                Node* pseudostatemarker =
                  new PseudoStateMarkerNode(next->cursor(), m_editor);
                m_nodes->insert(next->cursor(), pseudostatemarker);
                next->deleteLater();
              }
            }

            badNode->deleteLater();

          } else if (m_datastore->containsSubControl(newValue)) {
            Node* subcontrol =
              new SubControlNode(newValue, badNode->cursor(), m_editor);
            m_nodes->insert(badNode->cursor(), subcontrol);
            Node* prev = previousNode(badNode->cursor());

            if (prev) {
              if (prev->type() == Node::BadSubControlMarkerType) {
                Node* subcontrolmarker =
                  new SubControlMarkerNode(prev->cursor(), m_editor);
                m_nodes->insert(prev->cursor(), subcontrolmarker);
                prev->deleteLater();
              }
            }

            badNode->deleteLater();

          } else if (m_datastore->containsPseudoState(newValue)) {
            Node* pseudostate =
              new PseudoStateNode(newValue, badNode->cursor(), m_editor);
            m_nodes->insert(badNode->cursor(), pseudostate);
            Node* prev = previousNode(badNode->cursor());

            if (prev) {
              if (prev->type() == Node::BadPseudoStateMarkerType) {
                Node* pseudostatemarker =
                  new PseudoStateMarkerNode(prev->cursor(), m_editor);
                m_nodes->insert(prev->cursor(), pseudostatemarker);
                prev->deleteLater();
              }
            }

            badNode->deleteLater();

          } else {
            // still bad, just update text.
            badNode->setName(newValue);
          }
        }

        if (errors.testFlag(ParserState::InvalidWidget)) {
          QTextCursor cursor(data.cursor);
          cursor.select(QTextCursor::WordUnderCursor);
          newValue = cursor.selectedText();

          if (m_datastore->containsWidget(newValue)) {
            badNode->deleteLater();
            Node* subcontrol = new WidgetNode(newValue, data.cursor, m_editor);
            m_nodes->insert(data.cursor, subcontrol);

          } else {
            badNode->setName(newValue);
          }

        } else if (errors.testFlag(ParserState::InvalidSubControl)) {
          QTextCursor cursor(data.cursor);
          cursor.select(QTextCursor::WordUnderCursor);
          newValue = cursor.selectedText();

          if (m_datastore->containsSubControl(newValue)) {
            Node* subcontrol =
              new SubControlNode(newValue, data.cursor, m_editor);
            m_nodes->insert(data.cursor, subcontrol);
            badNode->deleteLater();

          } else {
            badNode->setName(newValue);
          }

        } else if (errors.testFlag(ParserState::InvalidPseudoState)) {
          QTextCursor cursor(data.cursor);
          cursor.select(QTextCursor::WordUnderCursor);
          newValue = cursor.selectedText();

          if (m_datastore->containsPseudoState(newValue)) {
            badNode->deleteLater();
            Node* subcontrol =
              new PseudoStateNode(newValue, data.cursor, m_editor);
            m_nodes->insert(data.cursor, subcontrol);

          } else {
            badNode->setName(newValue);
          }
        }

        break;
      }
    }

    m_editor->document()->markContentsDirty(
      0, m_editor->document()->toPlainText().length());
    emit rehighlight();
  }
}

void
Parser::handleCursorPositionChanged(QTextCursor textCursor)
{
  if (m_manualMove) {
    return;
  }

  setCurrentCursor(textCursor);

  Node* node;
  CursorData data;
  data.cursor = textCursor;

  nodeAtCursorPosition(&data, data.cursor.anchor());
  node = data.node;

  if (!node) {
    return;
  }

  auto keys = m_nodes->keys();
  auto index = keys.indexOf(data.cursor);
  auto modified = false;

  for (auto startbrace : m_startbraces) {
    if (startbrace->isBraceAtCursor()) {
      startbrace->setBraceAtCursor(false);
      modified = true;
    }
  }

  for (auto endbrace : m_endbraces) {
    if (endbrace->isBraceAtCursor()) {
      endbrace->setBraceAtCursor(false);
      modified = true;
    }
  }

  if (node) {
    auto type = node->type();

    if (type == Node::StartBraceType) {
      StartBraceNode* startbrace = qobject_cast<StartBraceNode*>(node);
      startbrace->setBraceAtCursor(true);

      for (auto i = index + 1; i < keys.size(); i++) {
        auto n = m_nodes->value(keys.at(i));

        if (n->type() == Node::EndBraceType) {
          EndBraceNode* endbrace = qobject_cast<EndBraceNode*>(n);
          endbrace->setBraceAtCursor(true);
          modified = true;
          break;
        }
      } // end for

    } else if (type == Node::EndBraceType) {
      EndBraceNode* endbrace = qobject_cast<EndBraceNode*>(node);
      endbrace->setBraceAtCursor(true);

      for (auto i = index - 1; i >= 0; i--) {
        auto n = m_nodes->value(keys.at(i));

        if (n->type() == Node::StartBraceType) {
          StartBraceNode* startbrace = qobject_cast<StartBraceNode*>(n);
          startbrace->setBraceAtCursor(true);
          modified = true;
          break;
        }
      } // end for

    } // end end brace type
  }   // end if node

  if (modified) {
    emit rehighlight();
  }
}

void
Parser::handleMouseClicked(const QPoint& pos)
{
  NamedNode* nNode = nullptr;
  nodeForPoint(pos, &nNode);

  if (nNode) {
    switch (nNode->type()) {

      case Node::WidgetType: {
        auto widget = qobject_cast<WidgetNode*>(nNode);

        if (!widget->isWidgetValid()) {
          // not a valid node
          if (widget != m_currentWidget) {
            auto matches = m_datastore->fuzzySearchWidgets(nNode->name());
            updateContextMenu(matches, nNode, pos);
          }
        }

        break;
      } // end WidgetType

      case Node::PseudoStateType: {
        auto pseudostate = qobject_cast<PseudoStateNode*>(nNode);

        if (!pseudostate->isStateValid()) {
          // not a valid node
          if (pseudostate != m_currentWidget) {
            auto matches = m_datastore->fuzzySearchPseudoStates(nNode->name());
            updateContextMenu(matches, nNode, pos);
          }
        }

        break;
      } // end PseudoStateType

      case Node::SubControlType: {
        auto subcontrol = qobject_cast<SubControlNode*>(nNode);

        if (!subcontrol->isStateValid()) {
          // not a valid node
          if (subcontrol != m_currentWidget) {
            auto matches = m_datastore->fuzzySearchSubControl(nNode->name());
            updateContextMenu(matches, nNode, pos);
          }
        }

        break;
      } // end SubControlType

      case Node::PropertyType: {
        auto property = qobject_cast<PropertyNode*>(nNode);

        if (property != m_currentWidget) {
          auto offset = m_editor->cursorForPosition(pos).anchor() -
                        property->cursor().anchor();
          auto status = property->isProperty(offset);

          if (status.status()) {
            auto matches = m_datastore->fuzzySearchProperty(status.name());
            updatePropertyContextMenu(matches, property, pos);

          } else {
            if (!status.name().isEmpty()) {
              if (property->isValidProperty()) {
                // must have a valid property to check value types.
                auto matches = m_datastore->fuzzySearchPropertyValue(
                  property->name(), status.name());
                updatePropertyValueContextMenu(
                  matches, property, status.name(), pos);
              } else {
                auto matches =
                  m_datastore->fuzzySearchProperty(property->name());
                QMultiMap<int, QPair<QString, QString>> propValMatches;
                for (auto name : matches) {
                  auto vMatches =
                    m_datastore->fuzzySearchPropertyValue(name, status.name());
                  for (auto key : vMatches.keys()) {
                    propValMatches.insert(
                      key,
                      qMakePair<QString, QString>(name, vMatches.value(key)));
                  }
                }
                updatePropertyValueContextMenu(
                  propValMatches, property, status.name(), pos);
              }
            }
          }
        }

        break;
      } // end

      case Node::BadNodeType: {
        //          auto badNode = qobject_cast<BadBlockNode*>(nNode);

        //          //      if (badNode) {
        //          //        ParserState::Errors errors =
        //          badNode->errors();

        //          //        if
        //          (errors.testFlag(ParserState::InvalidWidget)) {
        //          //          m_hoverWidget->show(pos, m_editor->tr("This
        //          is not a valid
        //          //          Widget"));

        //          //        } else if
        //          (errors.testFlag(ParserState::InvalidSubControl)) {
        //            //          m_hoverWidget->show(pos,
        //            m_editor->tr("This is not a valid
        //            //          Sub-Control"));

        //            //        } else if
        //            (errors.testFlag(ParserState::InvalidPseudoState)) {
        //              //          m_hoverWidget->show(pos,
        //              m_editor->tr("This is not a valid
        //              //          Pseudo-State"));

        //              //        } else if
        //              (errors.testFlag(ParserState::AnomalousMarkerType))
        //              //        {
        //              //          m_hoverWidget->show(
        //              //            pos,
        //              //            m_editor->tr(
        //              //              "This could be either a Pseudo-State
        //              marker or a
        //              //              Sub-Control marker."));

        //              //        } else if
        //              (errors.testFlag(ParserState::AnomalousType)) {
        //              //          m_hoverWidget->show(pos,
        //              m_editor->tr("The type of this is
        //              //          anomalous."));

        //              //        } else {
        //              //          hideHover();
        //              //        }

        //              //      }

        //              break;
      } // end case Node::BadNodeType

      default:
        //            m_hoverWidget->hideHover();
        break;
    }
  }
}

QTextCursor
Parser::currentCursor() const
{
  return m_currentCursor;
}

void
Parser::setCurrentCursor(const QTextCursor& currentCursor)
{
  m_currentCursor = currentCursor;
}

void
Parser::setMaxSuggestionCount(int maxSuggestionCount)
{
  m_maxSuggestionCount = maxSuggestionCount;
}

bool
Parser::manualMove() const
{
  return m_manualMove;
}

void
Parser::setManualMove(bool manualMove)
{
  m_manualMove = manualMove;
}

QMenu*
Parser::contextMenu() const
{
  return m_contextMenu;
}

void
Parser::actionPropertyNameChange(PropertyNode* property,
                                 const QString& name,
                                 int originalCursor)
{
  auto newLen = name.length();
  auto cursor(property->cursor());
  cursor.movePosition(
    QTextCursor::Right, QTextCursor::KeepAnchor, property->length());
  cursor.removeSelectedText();
  cursor.insertText(name);
  property->setValidProperty(true);
  if (property->hasPropertyMarker())
    property->setPropertyMarkerOffset(newLen);
  property->incrementOffsets(0, newLen - property->name().length());
  property->setName(name);
  property->setStart(originalCursor);
}

void
Parser::actionPropertyValueChange(PropertyNode* property,
                                  const PropertyStatus& status,
                                  const QString& name,
                                  int originalCursor)
{
  auto oldName = status.name();
  auto index = property->values().indexOf(oldName);
  auto offset = property->offsets().at(index);
  auto cursor(property->cursor());
  cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, offset);
  cursor.movePosition(
    QTextCursor::Right, QTextCursor::KeepAnchor, oldName.length());
  cursor.removeSelectedText();
  cursor.insertText(name);
  property->correctValue(index, name);
  property->incrementOffsets(index + 1, name.length() - oldName.length());
  property->setStart(originalCursor);
}

void
Parser::handleSuggestion(QAction* act)
{
  QVariant v = act->data();
  auto pair = v.value<QPair<NamedNode*, QPoint>>();
  auto nNode = pair.first;
  auto pos = pair.second;

  if (act == m_addPropertyMarkerAct) {
    PropertyNode* property = qobject_cast<PropertyNode*>(nNode);

    if (property) {
      auto originalCursor = property->cursor().anchor();
      property->setPropertyMarkerExists(true);
      property->setPropertyMarkerOffset(property->length());
      property->incrementOffsets(
        0); // by default increments by one, starting at 0.
      auto cursor(property->cursor());
      cursor.movePosition(
        QTextCursor::Right, QTextCursor::MoveAnchor, property->length());
      cursor.insertText(":");
      property->setStart(originalCursor);
    }

  } else {

    if (nNode) {
      auto originalCursor = nNode->cursor().anchor();
      auto name = act->text();

      switch (nNode->type()) {
        case Node::WidgetType: {
          auto widget = qobject_cast<WidgetNode*>(nNode);

          if (widget) {
            auto cursor(widget->cursor());
            widget->setWidgetValid(true);
            cursor.movePosition(
              QTextCursor::Right, QTextCursor::KeepAnchor, widget->length());
            cursor.removeSelectedText();
            cursor.insertText(name);
            widget->setName(name);
            widget->setStart(originalCursor);
          }

          break;
        }

        case Node::PseudoStateType: {
          auto pseudostate = qobject_cast<PseudoStateNode*>(nNode);

          if (pseudostate) {
            auto cursor(pseudostate->cursor());
            pseudostate->setStateValid(true);
            cursor.movePosition(QTextCursor::Right,
                                QTextCursor::KeepAnchor,
                                pseudostate->length());
            cursor.removeSelectedText();
            cursor.insertText(name);
            pseudostate->setName(name);
            pseudostate->setStart(originalCursor);
          }

          break;
        }

        case Node::SubControlType: {
          auto subcontrol = qobject_cast<SubControlNode*>(nNode);

          if (subcontrol) {
            auto cursor(subcontrol->cursor());
            subcontrol->setStateValid(true);
            cursor.movePosition(QTextCursor::Right,
                                QTextCursor::KeepAnchor,
                                subcontrol->length());
            cursor.removeSelectedText();
            cursor.insertText(name);
            subcontrol->setName(name);
            subcontrol->setStart(originalCursor);
          }

          break;
        }

        case Node::PropertyType: {
          auto property = qobject_cast<PropertyNode*>(nNode);

          if (property) {
            auto offset = m_editor->cursorForPosition(pos).anchor() -
                          property->cursor().anchor();
            auto status = property->isProperty(offset);

            if (status.status()) {
              actionPropertyNameChange(property, name, originalCursor);
            } else {
              if (name.contains(':')) {
                auto splits = name.split(':');
                auto pName = splits[0].trimmed();
                auto vName = splits[1].trimmed();

                actionPropertyNameChange(property, pName, originalCursor);
                actionPropertyValueChange(property, status, vName, originalCursor);

              } else {
                actionPropertyValueChange(property, status, name, originalCursor);
              }
            }
          }

          break;
        }
      }

      emit rehighlight();
    }
  }
}

int
Parser::maxSuggestionCount() const
{
  return m_maxSuggestionCount;
}

Node*
Parser::nextNode(QTextCursor cursor)
{
  QList<QTextCursor> keys = m_nodes->keys();
  int index = keys.indexOf(cursor) + 1; // next index

  if (index < keys.size()) {
    QTextCursor cursor = keys.value(index);
    return m_nodes->value(cursor);
  }

  return nullptr;
}

Node*
Parser::previousNode(QTextCursor cursor)
{
  QList<QTextCursor> keys = m_nodes->keys();
  int index = keys.indexOf(cursor) - 1; // previous index

  if (index >= 0) {
    QTextCursor cursor = keys.value(index);
    return m_nodes->value(cursor);
  }

  return nullptr;
}
