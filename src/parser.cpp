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

// void
// Parser::stepBack(int& pos, const QString& block)
//{
//  pos -= block.length();
//}

QPair<enum NodeType, enum NodeState>
Parser::checkType(int start,
                  const QString& block,
                  enum NodeState lastState,
                  PropertyNode* property) const
{
  if (lastState == IdSelectorMarkerState) {
    return qMakePair<NodeType, NodeState>(NodeType::IdSelectorType,
                                          NodeState::IdSelectorState);
  } else if (lastState == SubControlState) {
    if (m_datastore->containsProperty(block) &&
        m_datastore->containsSubControl(block)) {
      // this is a special case, the only case I know is icon which can be
      // both a property value AND a SubControl.
      return qMakePair<NodeType, NodeState>(NodeType::SubControlType,
                                            NodeState::SubControlState);
    }
  }
  if (property) {
    auto propertyState = m_datastore->isValidPropertyValueForProperty(
      property->name(), start, block);
    if (property && propertyState->isGoodValue()) {
      return qMakePair<NodeType, NodeState>(NodeType::PropertyValueType,
                                            NodeState::GoodPropertyState);
    } else {
      auto data =
        m_datastore->fuzzySearchPropertyValue(property->name(), block);
      if (data.isEmpty()) {
        return qMakePair<NodeType, NodeState>(NodeType::PropertyValueType,
                                              NodeState::BadPropertyValueState);
      } else {
        return qMakePair<NodeType, NodeState>(
          NodeType::PropertyValueType, NodeState::FuzzyPropertyValueState);
      }
    }
  }

  if (m_datastore->containsWidget(block)) {
    return qMakePair<NodeType, NodeState>(NodeType::WidgetType,
                                          NodeState::WidgetState);
  } else if (m_datastore->containsProperty(block)) {
    return qMakePair<NodeType, NodeState>(NodeType::PropertyType,
                                          NodeState::ValidNameState);
  } else if (m_datastore->containsPseudoState(block)) {
    return qMakePair<NodeType, NodeState>(NodeType::PseudoStateType,
                                          NodeState::PseudostateState);
  } else if (m_datastore->containsSubControl(block)) {
    return qMakePair<NodeType, NodeState>(NodeType::SubControlType,
                                          NodeState::SubControlState);
  } else if (block == ",") {
    return qMakePair<NodeType, NodeState>(NodeType::SubControlType,
                                          NodeState::SubControlSeperatorState);
  } else if (block == ":") {
    return qMakePair<NodeType, NodeState>(NodeType::ColonType,
                                          NodeState::PseudostateState);
  } else if (block == "::") {
    return qMakePair<NodeType, NodeState>(NodeType::SubControlMarkerType,
                                          NodeState::SubControlState);
  } else if (block == "#") {
    return qMakePair<NodeType, NodeState>(NodeType::IdSelectorMarkerType,
                                          NodeState::IdSelectorMarkerState);
  } else if (block == "/*") {
    return qMakePair<NodeType, NodeState>(NodeType::CommentType,
                                          NodeState::CommentState);
  } else if (block == "{") {
    return qMakePair<NodeType, NodeState>(NodeType::StartBraceType,
                                          NodeState::StartBraceState);
  } else if (block == "}") {
    return qMakePair<NodeType, NodeState>(NodeType::EndBraceType,
                                          NodeState::EndBraceState);
  } else if (block == "\n") {
    return qMakePair<NodeType, NodeState>(NodeType::NewlineType,
                                          NodeState::NewLineState);
  } else {
    QMultiMap<int, QString> possibilities;
    if (property) {
      possibilities =
        m_datastore->fuzzySearchPropertyValue(property->name(), block);
      if (possibilities.size() > 0)
        return qMakePair<NodeType, NodeState>(NodeType::PropertyValueType,
                                              NodeState::FuzzyPropertyState);
    }
    possibilities = m_datastore->fuzzySearchProperty(block);
    if (possibilities.size() > 0)
      return qMakePair<NodeType, NodeState>(NodeType::PropertyType,
                                            NodeState::FuzzyPropertyState);
    possibilities = m_datastore->fuzzySearchWidgets(block);
    if (possibilities.size() > 0)
      return qMakePair<NodeType, NodeState>(NodeType::WidgetType,
                                            NodeState::FuzzyWidgetState);
    possibilities = m_datastore->fuzzySearchPseudoStates(block);
    if (possibilities.size() > 0)
      return qMakePair<NodeType, NodeState>(NodeType::PseudoStateType,
                                            NodeState::FuzzyPseudostateState);
    possibilities = m_datastore->fuzzySearchSubControl(block);
    if (possibilities.size() > 0)
      return qMakePair<NodeType, NodeState>(NodeType::SubControlType,
                                            NodeState::FuzzySubControlState);
  }
  return qMakePair<NodeType, NodeState>(NodeType::NoType,
                                        NodeState::BadNodeState);
}

QMap<QTextCursor, Node*>
Parser::parseText(const QString& text)
{
  QString block;
  int start;
  int pos = 0;
  QMap<QTextCursor, Node*> nodes;

  while (true) {
    if ((block = m_datastore->findNext(text, pos, m_showLineMarkers))
          .isEmpty()) {
      break;
    }

    //    qDebug() << block;

    start = pos - block.length();
    QTextCursor cursor;

    NodeState lastState = BadNodeState;
    auto [type, state] = checkType(start, block, lastState);
    WidgetNodes* widgetnodes = nullptr;

    switch (type) {
      case NodeType::WidgetType: {
        cursor = m_datastore->getCursorForPosition(start);
        if (!widgetnodes) {
          widgetnodes = new WidgetNodes(cursor, m_editor, this);
          nodes.insert(cursor, widgetnodes);
        }

        auto widget = new WidgetNode(block, cursor, m_editor, state, this);
        widgetnodes->addWidget(widget);
        SubControl* subcontrol = nullptr;
        PseudoState* pseudostate = nullptr;
        IDSelector* idselector = nullptr;

        while (!(block = m_datastore->findNext(text, pos, m_showLineMarkers))
                  .isEmpty()) {
          start = pos - block.length();
          cursor = m_datastore->getCursorForPosition(start);

          auto [type, state] = checkType(start, block, lastState);
          if (type == NodeType::ColonType) {
            // a single colon could be either a pseudo state marker or a
            // property marker. if a property is detected then it should be
            // eaten within the property parsing so this should only appear
            // within a widget.
            type = NodeType::PseudoStateMarkerType;
          }
          switch (type) {
            case NodeType::WidgetType: {
              // if we find another widget than accept that the previous one is
              // incomplete.
              widget = new WidgetNode(block, cursor, m_editor, state, this);
              widgetnodes->addWidget(widget);
              subcontrol = nullptr;
              pseudostate = nullptr;
              idselector = nullptr;
              lastState = state;
              continue;
            }
            case NodeType::PropertyType: {
              PropertyNode* property =
                new PropertyNode(block, cursor, m_editor, state, this);
              property->setWidgetNodes(widgetnodes);
              widgetnodes->addProperty(property);
              parsePropertyWithValues(
                &nodes, property, text, start, pos /*, block*/);
              lastState = state;
              continue;
            }
            case NodeType::SubControlMarkerType:
              if (!widget->hasSubControl()) {
                subcontrol = new SubControl(
                  cursor, QTextCursor(), QString(), m_editor, this);
              } else {
                subcontrol = widget->subControl(cursor);
                subcontrol->setCursor(cursor);
              }
              widget->addSubControl(subcontrol);
              lastState = state;
              break;
            case NodeType::SubControlType:
              if (state == SubControlSeperatorState) {
                // TODO this should seperate sets of widget controls.
                widgetnodes->addSeperatorCursor(cursor);
                subcontrol = nullptr;
                pseudostate = nullptr;
                idselector = nullptr;
                widget = nullptr;
                lastState = state;
                continue;
              } else {
                if (subcontrol) {
                  if (subcontrol->hasMarker()) {
                    // check that this block follows the marker.
                    int markerPos = subcontrol->position() + 2;
                    auto s = text.mid(markerPos, start - markerPos);
                    if (s.trimmed().isEmpty()) {
                      subcontrol->setNameCursor(cursor);
                      subcontrol->setName(block);
                      subcontrol->setStateFlag(state);
                      if (m_datastore->isValidSubControlForWidget(
                            widget->name(), block)) {
                        subcontrol->clearStateFlag(BadSubControlForWidgetState);
                      } else {
                        subcontrol->setStateFlag(BadSubControlForWidgetState);
                      }
                    }
                  } else {
                    qWarning();
                    subcontrol = new SubControl(
                      QTextCursor(), cursor, QString(), m_editor, this);
                  }
                } else {
                  // TODO
                  qWarning();
                }
              }
              lastState = state;
              continue;
            case NodeType::PseudoStateMarkerType:
              if (lastState == IdSelectorState) {
                idselector = widget->idSelector();
                if (!idselector) {
                  idselector = new IDSelector(
                    cursor, QTextCursor(), QString(), m_editor, this);
                }
                pseudostate = idselector->pseudoState(cursor);
                if (!pseudostate) {
                  pseudostate = new PseudoState(
                    cursor, QTextCursor(), QString(), m_editor, this);
                }
                idselector->addPseudoState(pseudostate);
              } else if (lastState == SubControlState) {
                if (!subcontrol) {
                  if (widget->hasSubControl()) {
                    subcontrol = widget->subControl(cursor);
                  }
                }
                pseudostate = subcontrol->pseudoState(cursor);
                if (!pseudostate) {
                  pseudostate = new PseudoState(
                    cursor, QTextCursor(), QString(), m_editor, this);
                }
                subcontrol->addPseudoState(pseudostate);
              }
              break;
            case NodeType::PseudoStateType:
              if (!pseudostate) {
                pseudostate =
                  new PseudoState(cursor, QTextCursor(), block, m_editor, this);
              }
              pseudostate->setNameCursor(cursor);
              pseudostate->setName(block);
              pseudostate->setStateFlag(state);
              if (lastState == SubControlState) {
                subcontrol->addPseudoState(pseudostate);
              } else if (lastState == IdSelectorState) {
                idselector->addPseudoState(pseudostate);
              } else {
                // TODO pseudostate not on subcontrol.
                qWarning();
              }
              pseudostate = nullptr; // finished with pseudostate.
              lastState = state;
              continue;
            case NodeType::IdSelectorMarkerType:
              idselector = widget->idSelector();
              if (!idselector) {
                idselector = new IDSelector(
                  cursor, QTextCursor(), QString(), m_editor, this);
              }
              widget->setIdSelector(idselector);
              lastState = state;
              continue;
            case NodeType::IdSelectorType:
              if (!idselector) {
                idselector = new IDSelector(
                  cursor, QTextCursor(), QString(), m_editor, this);
              }
              idselector->setNameCursor(cursor);
              idselector->setName(block);
              idselector->setStateFlag(state);
              if (widget)
                widget->setIdSelector(idselector);
              else
                // TODO idselector not on widget.
                qWarning();
              idselector = nullptr; // finished with pseudostate.
              lastState = state;
              continue;
            case NodeType::CommentType:
              parseComment(&nodes, text, start, pos);
              lastState = state;
              continue;
            case NodeType::StartBraceType:
              widgetnodes->setStartBraceCursor(cursor);
              lastState = state;
              continue;
            case NodeType::EndBraceType:
              widgetnodes->setEndBraceCursor(cursor);
              widgetnodes = nullptr;
              //              leaveWidget = true;
              lastState = state;
              break;
            case NodeType::NewlineType:
              stashNewline(&nodes, pos++);
              lastState = state;
              break;
            default:
              // TODO error
              lastState = state;
              break;
          }
          if (/*leaveWidget*/ !widgetnodes) {
            lastState = state;
            break;
          }
        }
        continue;
      }
      case NodeType::PropertyType: {
        cursor = m_datastore->getCursorForPosition(start);
        PropertyNode* property =
          new PropertyNode(block, cursor, m_editor, state, this);
        auto counts = m_datastore->attributeCounts(block);
        if (counts.first != -1) {
          property->setMinCount(counts.first);
          property->setMaxCount(counts.second);
        }
        nodes.insert(cursor, property);
        parsePropertyWithValues(&nodes, property, text, start, pos /*, block*/);
        continue;
      }
      case NodeType::CommentType:
        parseComment(&nodes, text, start, pos);
        continue;
      case NodeType::NewlineType:
        stashNewline(&nodes, pos++);
        break;
      default:
        QString nextBlock;

        if (nodes.isEmpty()) {
          auto oldPos = pos;
          nextBlock = m_datastore->findNext(text, pos, m_showLineMarkers);

          if (nextBlock == ":") {
            auto colonPos = pos;
            nextBlock = m_datastore->findNext(text, pos, m_showLineMarkers);

            if (m_datastore->containsPseudoState(nextBlock)) {
              cursor = m_datastore->getCursorForPosition(start);
              auto widget =
                new WidgetNode(block, cursor, m_editor, state, this);
              widgetnodes->addWidget(widget);
              continue;

            } else if (m_datastore->propertyValueAttribute(nextBlock) !=
                       NoAttributeValue) {
              cursor = m_datastore->getCursorForPosition(start);
              PropertyNode* property =
                new PropertyNode(block, cursor, m_editor, state, this);
              property->setPropertyMarker(true);
              property->setPropertyMarkerCursor(
                m_datastore->getCursorForPosition(colonPos));
              nodes.insert(cursor, property);
              pos -= block.length(); // step back
              parsePropertyWithValues(
                &nodes, property, text, start, pos /*, block*/);
              continue;
            }

          } else if (nextBlock == "::") {
            nextBlock = m_datastore->findNext(text, pos, m_showLineMarkers);

            if (m_datastore->containsSubControl(nextBlock)) {
              cursor = m_datastore->getCursorForPosition(start);
              auto widget =
                new WidgetNode(block, cursor, m_editor, state, this);
              widgetnodes->addWidget(widget);
              if (!widget->isSubControlFuzzy(cursor)) {
                if (!m_datastore->isValidSubControlForWidget(nextBlock,
                                                             widget->name())) {
                  widget->setWidgetCheck(
                    NodeState::BadSubControlForWidgetState);
                }
              }
              continue;
            }
          }
          // step back
          pos = oldPos;
          cursor = m_datastore->getCursorForPosition(start);
          auto widget = new WidgetNode(block, cursor, m_editor, state, this);
          widgetnodes->addWidget(widget);
        } else { // anomalous type - see what comes next.
          int oldPos = pos;
          nextBlock = m_datastore->findNext(text, pos, m_showLineMarkers);

          if (nextBlock == ":") {
            nextBlock = m_datastore->findNext(text, pos, m_showLineMarkers);

            if (m_datastore->containsPseudoState(nextBlock)) {
              cursor = m_datastore->getCursorForPosition(start);
              auto widget =
                new WidgetNode(block, cursor, m_editor, state, this);
              widgetnodes->addWidget(widget);
              continue;

            } else if (m_datastore->propertyValueAttribute(nextBlock) !=
                       NoAttributeValue) {
              cursor = m_datastore->getCursorForPosition(start);
              PropertyNode* property =
                new PropertyNode(block, cursor, m_editor, state, this);
              property->setPropertyMarker(true);
              property->setPropertyMarkerCursor(
                m_datastore->getCursorForPosition(oldPos));
              nodes.insert(cursor, property);
              pos -= block.length(); // step back
              parsePropertyWithValues(
                &nodes, property, text, start, pos /*, block*/);
              continue;
            }

          } else if (nextBlock == "::") {
            nextBlock = m_datastore->findNext(text, pos, m_showLineMarkers);

            if (m_datastore->containsSubControl(nextBlock)) {
              cursor = m_datastore->getCursorForPosition(start);
              auto widget =
                new WidgetNode(block, cursor, m_editor, state, this);
              widgetnodes->addWidget(widget);
              continue;
            }
          }
        }
        break;
    }
  }
  return nodes;
}

void
Parser::parseInitialText(const QString& text)
{
  m_datastore->setBraceCount(0);
  m_datastore->clearNodes();

  QMap<QTextCursor, Node*> nodes = parseText(text);
  m_datastore->setNodes(nodes);

  emit parseComplete();
}

void
Parser::parsePropertyWithValues(QMap<QTextCursor, Node*>* nodes,
                                PropertyNode* property,
                                const QString& text,
                                int start,
                                int& pos)
{
  QString block;
  bool hash = false;
  if (property->isValidPropertyName()) {
    QString propertyName = property->name();
    while (!(block = m_datastore->findNext(text, pos, m_showLineMarkers))
              .isEmpty()) {
      if (hash) {
        block.prepend("#");
        hash = false;
      }
      PropertyStatus* valueStatus = nullptr;
      if (block == ":") {
        if (!property->hasPropertyMarker()) {
          property->setPropertyMarker(true);
          property->setPropertyMarkerCursor(
            m_datastore->getCursorForPosition(pos - 1));
        } else {
          /*TODO error too many :*/
        }
      } else if (block == "#") {
        // if in valid property this is probably a color #
        hash = true;
      } else if (block == "/*") {
        parseComment(nodes, text, start, pos);
      } else if (block == "\n") {
        stashNewline(nodes, pos++);
      } else if (block == ";") {
        property->setPropertyEndMarker(true);
        property->setPropertyEndMarkerCursor(
          m_datastore->getCursorForPosition(pos - 1));
        break;
      } else if (block == "}") {
        m_datastore->stepBack(pos, block);
        break;
      } else if ((valueStatus = m_datastore->isValidPropertyValueForProperty(
                    propertyName, pos, block))) {
        // some properties (ie bottom) can be BOTH a property and a value
        // so check if it is a possible value FIRST before assuming
        // that it is a following property.
        property->addValue(ValidPropertyNameState, valueStatus);
      } else if (m_datastore->containsProperty(block)) {
        // This is a different property, we are probably
        // missing a property end marker.
        m_datastore->stepBack(pos, block);
        break;
      } /* else {
         auto valueStatus = m_datastore->isValidPropertyValueForProperty(
           propertyName, pos, block, text);
         property->addValue(ValidPropertyNameState, valueStatus);
       }*/
    }
  }
}

void
Parser::parseComment(QMap<QTextCursor, Node*>* nodes,
                     const QString& text,
                     int start,
                     int& pos)
{
  QTextCursor cursor = m_datastore->getCursorForPosition(start);

  QChar c;
  CommentNode* comment = new CommentNode(cursor, m_editor);
  //  m_datastore->insertNode(cursor, comment);
  nodes->insert(cursor, comment);

  while (true) {
    c = text.at(pos);
    if (c.isSpace()) {
      pos++;
    } else {
      comment->setTextCursor(m_datastore->getCursorForPosition(pos));
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
          comment->setEndCommentCursor(
            m_datastore->getCursorForPosition(pos - 2));
          break;

        } else {
          comment->append(c);
        }
      }
    }
    c = text.at(pos++);
  }
}

void
Parser::nodeForPoint(const QPoint& pos, NodeSection** nodeSection)
{
  auto values = m_datastore->nodes().values();

  for (auto& n : values) {
    auto section = n->sectionIfIn(pos);

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
  Node* previous = nullptr;
  QMap<QTextCursor, Node*> nodes = m_datastore->nodes();

  for (auto [key, node] : asKeyValueRange(nodes)) {
    if (!node) {
      return;
    }

    if (position > node->end()) {
      previous = node;
      continue;

    } else if (position >= node->position() && position <= node->end()) {
      data->node = node;
      data->cursor = node->cursor();
      data->prevNode = previous;
      break;
    }
  }
}

// WidgetNode*
// Parser::stashWidget(QMap<QTextCursor, Node*>* nodes,
//                    QTextCursor cursor,
//                    const QString& block,
//                    enum NodeCheck check)
//{
//  auto widgetnode = new WidgetNode(block, cursor, m_editor, check, this);
//  (*nodes).insert(cursor, widgetnode);
//  return widgetnode;
//}

// void
// Parser::stashBadNode(QMap<QTextCursor, Node*>* nodes,
//                     int position,
//                     const QString& block,
//                     ParserState::Error error)
//{
//  auto cursor = getCursorForPosition(position);
//  auto badblock = new BadBlockNode(block, cursor, error, m_editor, this);
//  //  m_datastore->insertNode(cursor, badblock);
//  (*nodes).insert(cursor, badblock);
//}

void
Parser::stashNewline(QMap<QTextCursor, Node*>* nodes, int position)
{
  auto cursor = m_datastore->getCursorForPosition(position);
  auto newline = new NewlineNode(cursor, m_editor);
  //  m_datastore->insertNode(cursor, newline);
  (*nodes).insert(cursor, newline);
}

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

  QAction* act =
    (*suggestionsMenu)
      ->addSection(m_datastore->invalidIcon(),
                   tr("%1 is not a valid %2 name").arg(node->name(), typeName));
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
  QAction* act;
  (*suggestionsMenu)->clear();

  if (!property->isValidPropertyName()) {
    act = (*suggestionsMenu)
            ->addSection(
              m_datastore->invalidIcon(),
              tr("%1 is not a valid property name").arg(property->name()));
    act->setData(pos);
    (*suggestionsMenu)->addAction(act);
    (*suggestionsMenu)->addSeparator();
    updateMenu(
      matches, property, pos, suggestionsMenu, SectionType::PropertyName);
  } else if (!property->hasPropertyMarker()) {
    act = new QAction(
      m_datastore->invalidIcon(),
      tr("%1 is missing a property marker (:)").arg(property->name()));
    (*suggestionsMenu)->addAction(act);
    (*suggestionsMenu)->addSeparator();
    act =
      new QAction(m_datastore->addColonIcon(), tr("Add property marker (:)"));
    act->setData(pos);
    (*suggestionsMenu)->addAction(act);
    setMenuData(act, property, SectionType::PropertyMarker);
    m_editor->connect(
      act, &QAction::triggered, m_editor, &StylesheetEditor::suggestionMade);
  } else if (!property->hasPropertyEndMarker()) {
    act =
      new QAction(m_datastore->badSemiColonIcon(),
                  tr("%1 is missing an end marker (;)").arg(property->name()));
    (*suggestionsMenu)->addAction(act);
    (*suggestionsMenu)->addSeparator();
    act = new QAction(m_datastore->addSemiColonIcon(),
                      tr("Add property end marker (;)"));
    act->setData(pos);
    (*suggestionsMenu)->addAction(act);
    setMenuData(act, property, SectionType::PropertyEndMarker);
    m_editor->connect(
      act, &QAction::triggered, m_editor, &StylesheetEditor::suggestionMade);
  } else {
    act =
      new QAction(m_datastore->validIcon(),
                  tr("Property %1 appears to be valid!").arg(property->name()));
    act->setData(pos);
    (*suggestionsMenu)->addAction(act);
    (*suggestionsMenu)->addSeparator();
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
Parser::updateSubControlMenu(WidgetNode* widget,
                             const QString& name,
                             QPoint pos,
                             QMenu** suggestionsMenu)
{
  if (!widget->isSubControlValid(pos)) {
    if (widget->isSubControlFuzzy(pos)) {
      auto matches = m_datastore->fuzzySearchSubControl(name);
      (*suggestionsMenu)->clear();
      auto widgetact = getWidgetAction(
        m_datastore->fuzzyIcon(),
        tr("Fuzzy sub control %1.<br>Possible values showing below.").arg(name),
        *suggestionsMenu);
      (*suggestionsMenu)->addAction(widgetact);
      (*suggestionsMenu)->addSeparator();
      updateMenu(matches,
                 widget,
                 pos,
                 suggestionsMenu,
                 SectionType::FuzzyWidgetSubControl);
    } else if (widget->isSubControlBad(pos)) {
      if (widget->isSubControl()) {
        auto matches = m_datastore->fuzzySearchSubControl(name);
        (*suggestionsMenu)->clear();
        auto widgetact = getWidgetAction(m_datastore->fuzzyIcon(),
                                         tr("Sub control %1 does not match <br>"
                                            "supplied widget %2.<br>"
                                            "Possible sub controls are.")
                                           .arg(name, widget->name()),
                                         *suggestionsMenu);
        (*suggestionsMenu)->addAction(widgetact);
        (*suggestionsMenu)->addSeparator();
        auto act = new QAction(m_datastore->addDColonIcon(),
                               tr("Change to sub control marker (::)"));
        (*suggestionsMenu)->addAction(act);
        setMenuData(act, widget, SectionType::WidgetSubControlMarker);
        m_editor->connect(act,
                          &QAction::triggered,
                          m_editor,
                          &StylesheetEditor::suggestionMade);
        (*suggestionsMenu)->addSeparator();
        updateMenu(matches,
                   widget,
                   pos,
                   suggestionsMenu,
                   SectionType::FuzzyWidgetSubControl);
      } else if (widget->isPseudoState()) {
        auto matches = m_datastore->fuzzySearchSubControl(name);
        (*suggestionsMenu)->clear();
        auto widgetact =
          getWidgetAction(m_datastore->fuzzyIcon(),
                          tr("Pseudo state %1 does not match <br>"
                             "supplied widget %2.<br>"
                             "Possible states are.")
                            .arg(name, widget->name()),
                          *suggestionsMenu);
        (*suggestionsMenu)->addAction(widgetact);
        (*suggestionsMenu)->addSeparator();
        updateMenu(matches,
                   widget,
                   pos,
                   suggestionsMenu,
                   SectionType::FuzzyWidgetSubControl);
      }
    } else if (!widget->doesMarkerMatch(SubControlState)) {
      (*suggestionsMenu)->clear();
      auto widgetact = getWidgetAction(
        m_datastore->badColonIcon(),
        tr("Sub control %1 does not match<br>pseudo state marker (::)")
          .arg(name),
        *suggestionsMenu);
      (*suggestionsMenu)->addAction(widgetact);
      (*suggestionsMenu)->addSeparator();
      auto act = new QAction(m_datastore->addDColonIcon(),
                             tr("Change to sub control marker (::)"));
      (*suggestionsMenu)->addAction(act);
      setMenuData(act, widget, SectionType::WidgetSubControlMarker);
      m_editor->connect(
        act, &QAction::triggered, m_editor, &StylesheetEditor::suggestionMade);
    }
  } else if (widget->isSubControlBad(pos)) {
    auto subcontrols =
      m_datastore->possibleSubControlsForWidget(widget->name());
    (*suggestionsMenu)->clear();
    auto widgetact =
      getWidgetAction(m_datastore->invalidIcon(),
                      tr("Sub control %1 does not match widget %2<br>"
                         "possible sub controls are:")
                        .arg(name)
                        .arg(widget->name()),
                      *suggestionsMenu);
    (*suggestionsMenu)->addAction(widgetact);
    (*suggestionsMenu)->addSeparator();
    updateMenu(subcontrols,
               widget,
               pos,
               suggestionsMenu,
               SectionType::BadWidgetSubControl);
  }
}

void
Parser::updatePseudoStateMenu(WidgetNode* widget,
                              QPoint pos,
                              QMenu** suggestionsMenu)
{
  auto pseudoState = widget->pseudoState(pos);
  if (pseudoState) {
    auto name = pseudoState->name();
    if (widget->isSubControlFuzzy(pos)) {
      auto matches = m_datastore->fuzzySearchPseudoStates(name);
      (*suggestionsMenu)->clear();
      auto widgetact = getWidgetAction(
        m_datastore->fuzzyIcon(),
        tr("Fuzzy pseudo state name.<br>Possible values showing below.")
          .arg(name),
        *suggestionsMenu);
      (*suggestionsMenu)->addAction(widgetact);
      (*suggestionsMenu)->addSeparator();
      updateMenu(matches,
                 widget,
                 pos,
                 suggestionsMenu,
                 SectionType::FuzzyWidgetPseudoState);
    } else if (!widget->doesMarkerMatch(PseudostateState)) {
      (*suggestionsMenu)->clear();
      auto widgetact = getWidgetAction(
        m_datastore->badDColonIcon(),
        tr("Pseudo state %1 does not match<br>sub control marker (:)")
          .arg(name),
        *suggestionsMenu);
      (*suggestionsMenu)->addAction(widgetact);
      (*suggestionsMenu)->addSeparator();
      auto act = new QAction(m_datastore->addColonIcon(),
                             tr("Change to pseudo state marker (:)"));
      (*suggestionsMenu)->addAction(act);
      setMenuData(act, widget, SectionType::WidgetPseudoStateMarker);
      m_editor->connect(
        act, &QAction::triggered, m_editor, &StylesheetEditor::suggestionMade);
    }
  }
}

void
Parser::updateValidPropertyValueContextMenu(QMultiMap<int, QString> matches,
                                            QPoint pos,
                                            PropertyNode* property,
                                            const QString& valueName,
                                            QMenu** suggestionsMenu)
{
  (*suggestionsMenu)->clear();

  auto keys = matches.keys(valueName);
  for (auto& key : keys) {
    matches.remove(key, valueName);
  }

  auto act = getWidgetAction(
    m_datastore->validIcon(),
    tr("%1 is a valid value for %2.<br>Other suggestions are : ")
      .arg(valueName, property->name()),
    *suggestionsMenu);
  (*suggestionsMenu)->addAction(act);
  (*suggestionsMenu)->addSeparator();

  updateMenu(matches, property, pos, suggestionsMenu, SectionType::None);
}

void
Parser::setMenuData(QAction* act,
                    Node* node,
                    SectionType type,
                    const QString& oldName,
                    int offset,
                    int index)
{
  MenuData data;
  data.node = node;
  data.type = type;
  data.oldName = oldName;
  data.offset = offset;
  data.index = index;
  QVariant v;
  v.setValue<MenuData>(data);
  act->setData(v);
}

void
Parser::updateInvalidPropertyValueContextMenu(QMultiMap<int, QString> matches,
                                              QPoint pos,
                                              PropertyNode* property,
                                              const QString& valueName,
                                              QMenu** suggestionsMenu)
{
  (*suggestionsMenu)->clear();

  auto keys = matches.keys(valueName);
  for (auto& key : keys) {
    matches.remove(key, valueName);
  }

  auto act = getWidgetAction(
    m_datastore->invalidIcon(),
    tr("%1 is an invalid value for %2.<br>Possible suggestions are : ")
      .arg(valueName, property->name()),
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
                 tr("%1 is not a valid property value for %2")
                   .arg(valueName, nNode->name()));
  if (matches.isEmpty()) {
    (*suggestionsMenu)
      ->addSection(m_datastore->noIcon(), tr("No suggestions are available!"));
    return;
  }

  auto reversed = sortLastNValues(matches);

  QString s("%1 : %2");
  for (auto& pair : reversed) {
    auto act = new QAction(s.arg(pair.first, pair.second));
    (*suggestionsMenu)->addAction(act);
    QVariant v;
    v.setValue(qMakePair<NamedNode*, QPoint>(nNode, pos));
    act->setData(v);
    m_editor->connect(
      act, &QAction::triggered, m_editor, &StylesheetEditor::suggestionMade);
  }

  if (reversed.size() > 0) {
    (*suggestionsMenu)->setEnabled(true);
  }
}

void
Parser::updateMenu(QStringList matches,
                   Node* nNode,
                   QPoint pos,
                   QMenu** suggestionsMenu,
                   SectionType type,
                   const QString& oldName)
{
  QAction* act;

  if (matches.isEmpty()) {
    (*suggestionsMenu)
      ->addSection(m_datastore->noIcon(), tr("No suggestions are available!"));
    return;
  }

  auto reversed = reverseList(matches, m_datastore->maxSuggestionCount());

  for (auto& match : reversed) {
    act = new QAction(match);
    act->setData(pos);
    (*suggestionsMenu)->addAction(act);
    setMenuData(act, nNode, type, oldName);
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
                   QPoint pos,
                   QMenu** suggestionsMenu,
                   SectionType type,
                   const QString& oldName,
                   int offset,
                   int index)
{
  QAction* act;

  if (matches.isEmpty()) {
    (*suggestionsMenu)
      ->addSection(m_datastore->noIcon(), tr("No suggestions are available!"));
    return;
  }

  auto reversed = reverseLastNValues(matches);

  for (auto& key : reversed) {
    act = new QAction(matches.value(key));
    (*suggestionsMenu)->addAction(act);
    setMenuData(act, nNode, type, oldName, offset, index);
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
  //  QMultiMap<int, QString> rMatches;
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

QStringList
Parser::reverseList(QStringList list, int count)
{
  std::reverse(list.begin(), list.end());
  return list.mid(0, count - 1);
}

QList<QPair<QString, QString>>
Parser::sortLastNValues(QMultiMap<int, QPair<QString, QString>> matches)
{
  //  QMultiMap<int, QPair<QString, QString>> rMatches;
  QList<QPair<QString, QString>> sorted;
  auto values = matches.values();
  for (auto& v : values) {
    auto lf = v.first.length();
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

// StylesheetData*
// Parser::getStylesheetProperty(const QString& sheet, int& pos)
//{
//  QString property, sep, value;
//  StylesheetData* data = nullptr;

//  property = m_datastore->findNext(sheet, pos, m_showLineMarkers);

//  if (m_datastore->containsStylesheetProperty(property)) {
//    data = new StylesheetData();
//    data->name = property;

//    if (pos < sheet.length()) {
//      sep = m_datastore->findNext(sheet, pos, m_showLineMarkers);

//      if (sep == ":") {
//        while (true) {
//          value = findNext(sheet, pos);

//          if (value == ";" || value == "}") {
//            break;
//          }

//          // TODO reimplement this seperately
//          //          m_datastore->ifValidStylesheetValue(property, value,
//          //          data);
//        }

//      } else {
//        // TODO show stylesheet error - no property seperator.
//      }
//    }
//  }

//  return data;
//}

void
Parser::handleDocumentChanged(int offset, int charsRemoved, int charsAdded)
{
  // If there is no text then nothing to be done.
  if (m_editor->toPlainText().trimmed().isEmpty()) {
    return;
  }

  /*
     Initially I handled this by rebuilding the nodes as I modified the
     text, However the code was getting lerger than I liked, and was taking
     too long to write so I decided to reparse the entire code. I suspect
     that no stylesheet would be large enough to be worth the cost of
     keeping the code clean. I might revisit this later.
  */
  //  parseInitialText(m_editor->toPlainText());
  return;

  //  // If the text is changed due to a suggestion then ignore
  //  // it as is is handled elsewhere.
  //  if (m_datastore->hasSuggestion()) {
  //    m_datastore->setHasSuggestion(false);
  //    return;
  //  }

  //  QString text;
  //  if ((text = m_editor->toPlainText()).isEmpty()) {
  //    return;
  //  }
  //  auto count = charsAdded - charsRemoved;
  //  auto textChanged = text.mid(offset, count);

  //  CursorData data = getNodeAtCursor(offset);
  //  Node* node = data.node;

  //  if (node) {
  //    switch (node->type()) {
  //      case NodeType::PropertyType: {
  //        auto property = qobject_cast<PropertyNode*>(node);

  //        if (property) {
  //          auto status = property->sectionAtOffset(offset, textChanged);
  //          switch (status.status()) {
  //            case SectionType::PropertyName: {
  //              int length = property->name().length() + count;
  //              auto newName = text.mid(property->start(), length);
  //              property->setName(newName);
  //              if (m_datastore->containsProperty(newName)) {
  //                property->setValidPropertyName(true);
  //              } else {
  //                property->setValidPropertyName(false);
  //              }
  //              emit rehighlightBlock(data.cursor.block());
  //              break;
  //            }
  //            case SectionType::PropertyValue: {
  //              int index = property->values().indexOf(status.name());
  //              int length = status.name().length() + count;
  //              auto newName = text.mid(property->valuePosition(index),
  //              length); property->setValue(index, newName); if
  //              (m_datastore->isValidPropertyValueForProperty(property->name(),
  //                                                               newName))
  //                                                               {
  //                property->setCheck(index,
  //                PropertyValueCheck::GoodValue);
  //              } else {
  //                property->setCheck(index, PropertyValueCheck::BadValue);
  //              }
  //              emit rehighlightBlock(data.cursor.block());
  //              break;
  //            }
  //            case SectionType::PropertyMarker: {
  //              if (!property->hasPropertyMarker()) {
  //                int length = property->name().length();
  //                QTextCursor cursor = property->cursor();
  //                cursor.movePosition(
  //                  QTextCursor::Right, QTextCursor::MoveAnchor, length);
  //                property->setPropertyMarkerCursor(cursor);
  //                property->setPropertyMarker(true);
  //              }
  //              emit rehighlightBlock(data.cursor.block());
  //              break;
  //            }
  //            case SectionType::PropertyEndMarker: {
  //              if (!property->hasPropertyEndMarker()) {
  //                int length = property->length();
  //                QTextCursor cursor = property->cursor();
  //                cursor.movePosition(
  //                  QTextCursor::Right, QTextCursor::MoveAnchor, length);
  //                property->setPropertyEndMarkerCursor(cursor);
  //                property->setPropertyEndMarker(true);
  //              }
  //              emit rehighlightBlock(data.cursor.block());
  //              break;
  //            }
  //          }
  //        }
  //      }
  //    }
  //  } else {
  //    // code has been added outside an existing node.
  //      // NOTE may be worth the cost of reparsing the entire text
  //      // as it shouldn't be really huge in real terms.
  //      auto nodes = parseText(textChanged);
  //  }
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

void
Parser::updateColorDialogMenu(QAction* act,
                              PropertyNode* property,
                              const QString& name,
                              int offset,
                              int index,
                              QMenu** suggestionsMenu)
{
  MenuData data;
  data.node = property;
  data.type = SectionType::ColorDialog;
  data.oldName = name;
  data.offset = offset;
  data.index = index;
  QVariant v;
  v.setValue<MenuData>(data);
  act->setData(v);
  (*suggestionsMenu)->addAction(act);
  m_editor->connect(
    act, &QAction::triggered, m_editor, &StylesheetEditor::suggestionMade);
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
        switch (section->type) {
          case WidgetName: {
            if (widget->isNameFuzzy()) {
              auto matches = m_datastore->fuzzySearchWidgets(widget->name());
              suggestionsMenu->clear();
              auto widgetact = getWidgetAction(
                m_datastore->fuzzyIcon(),
                tr("Fuzzy widget name.<br>Possible values showing below.")
                  .arg(widget->subControl(pos)->name()),
                suggestionsMenu);
              suggestionsMenu->addAction(widgetact);
              suggestionsMenu->addSeparator();
              updateMenu(matches,
                         widget,
                         pos,
                         &suggestionsMenu,
                         SectionType::FuzzyWidgetName);
            } else {
              qWarning();
            }
            break;
          }
          case WidgetSubControlName: {
            auto name = widget->subControl(pos)->name();
            updateSubControlMenu(widget, name, pos, &suggestionsMenu);
            break;
          }
          case SectionType::WidgetSubControlMarker: {
            auto name = widget->subControl(pos)->name();
            updateSubControlMenu(widget, name, pos, &suggestionsMenu);
            break;
          }
          case WidgetPseudoState: {
            updatePseudoStateMenu(widget, pos, &suggestionsMenu);
            break;
          }
          case SectionType::WidgetPseudoStateMarker: {
            updatePseudoStateMenu(widget, pos, &suggestionsMenu);
            break;
          }
          case WidgetPropertyName: {
            qWarning();
            break;
          }
          case SectionType::WidgetPropertyMarker: {
            qWarning();
            break;
          }
          case WidgetPropertyValue: {
            qWarning();
            break;
          }
          case WidgetPropertyEndMarker: {
            qWarning();
            break;
          }
        }
      } // end WidgetType

      case NodeType::PropertyType: {
        auto property = qobject_cast<PropertyNode*>(section->node);
        switch (section->type) {
          case SectionType::PropertyName: {
            auto minCount = property->minCount();
            auto maxCount = property->maxCount();
            auto count = property->valueCount();

            if (count > maxCount) {
              suggestionsMenu->clear();
              auto widgetact =
                getWidgetAction(m_datastore->badIcon(),
                                tr("Too many values %1, maximum is %2")
                                  .arg(count)
                                  .arg(maxCount),
                                suggestionsMenu);
              suggestionsMenu->addAction(widgetact);
            } else if (count < minCount) {
              suggestionsMenu->clear();
              auto widgetact = getWidgetAction(
                m_datastore->badIcon(),
                tr("Not enough values, should be %1").arg(minCount),
                suggestionsMenu);
              suggestionsMenu->addAction(widgetact);
            } else if (!property->hasPropertyEndMarker() &&
                       !property->isFinalProperty()) {
              suggestionsMenu->clear();
              auto widgetact =
                getWidgetAction(m_datastore->badDColonIcon(),
                                tr("Property has no end marker (;)"),
                                suggestionsMenu);
              suggestionsMenu->addAction(widgetact);
              suggestionsMenu->addSeparator();
              auto act =
                new QAction(m_datastore->addColonIcon(), tr("Add end marker"));
              suggestionsMenu->addAction(act);
              setMenuData(act, property, SectionType::PropertyEndMarker);
              m_editor->connect(act,
                                &QAction::triggered,
                                m_editor,
                                &StylesheetEditor::suggestionMade);
            } else {
              if (property->isValidPropertyName()) {
                updatePropertyContextMenu(property, pos, &suggestionsMenu);
              } else {
                auto matches =
                  m_datastore->fuzzySearchProperty(property->name());
                updatePropertyContextMenu(
                  property, pos, &suggestionsMenu, matches);
              }
            }
            break;
          }
          case SectionType::PropertyValue: {
            auto valName = property->value(section->position);
            auto status = property->valueStatus(section->position);
            QString message;
            while (status) {
              switch (status->state()) {
                case BadValue: {
                  if (status->isInRect(pos)) {
                    for (auto i = 0; i < status->sectionCount(); i++) {
                      if (status->isInSectionRect(i, pos)) {
                        auto state = status->sectionState(i);
                        switch (state) {
                          case BadValueName:
                            message = tr("Bad value name %1")
                                        .arg(status->sectionName(i));
                            break;
                          case FuzzyValueName:
                            message = tr("Value name is fuzzy %1")
                                        .arg(status->sectionName(i));
                            break;
                          case FuzzyColorValue:
                            message = tr("Color name is fuzzy %1")
                                        .arg(status->sectionName(i));
                            break;
                          case BadColorValue:
                            message = tr("Color name is bad %1")
                                        .arg(status->sectionName(i));
                            break;
                          case BadUrlValue:
                            message = tr("URL value is bad %1")
                                        .arg(status->sectionName(i));
                            break;
                          case BadValueCount:
                            message = tr("Wrong number of parameters %1")
                                        .arg(status->sectionName(i));
                            break;
                          case RepeatValueName:
                            message = tr("Value name was repeated %1")
                                        .arg(status->sectionName(i));
                            break;
                          case BadNumericalValue:
                            message = tr("Numerical parameter is bad %1")
                                        .arg(status->sectionName(i));
                            break;
                          case BadNumericalValue_31:
                            message =
                              tr("Numerical parameter should be 0-31 : %1")
                                .arg(status->sectionName(i));
                            break;
                          case BadNumericalValue_100:
                            message =
                              tr("Numerical parameter should be 0-100 : %1")
                                .arg(status->sectionName(i));
                            break;
                          case BadNumericalValue_255:
                            message =
                              tr("Numerical parameter should be 0-255 : %1")
                                .arg(status->sectionName(i));
                            break;
                          case BadNumericalValue_359:
                            message =
                              tr("Numerical parameter should be 0-359 : %1")
                                .arg(status->sectionName(i));
                            break;
                          case BadLengthUnit:
                            message =
                              tr("Should have a unit (px, pt, em, ex) : %1")
                                .arg(status->sectionName(i));
                            break;
                          case BadFontUnit:
                            message = tr("Should have a unit (px, pt) : %1")
                                        .arg(status->sectionName(i));
                            break;
                        }
                        suggestionsMenu->clear();
                        auto widgetact = getWidgetAction(
                          m_datastore->invalidIcon(), message, suggestionsMenu);
                        suggestionsMenu->addAction(widgetact);
                      }
                    }
                  }
                  break;
                }
                case BadHashColorValue: {
                  message = tr("Color value is bad: %1").arg(status->name());
                  suggestionsMenu->clear();
                  auto widgetact = getWidgetAction(
                    m_datastore->invalidIcon(), message, suggestionsMenu);
                  suggestionsMenu->addAction(widgetact);
                  suggestionsMenu->addSeparator();
                  auto act = new QAction(m_datastore->addColonIcon(),
                                         tr("Call ColorDialog."));
                  updateColorDialogMenu(act,
                                        property,
                                        status->name(),
                                        status->offset(),
                                        section->position,
                                        &suggestionsMenu);
                  break;
                }
                case BadColorValue: {
                  message = tr("Color value is bad: %1").arg(status->name());
                  suggestionsMenu->clear();
                  auto widgetact = getWidgetAction(
                    m_datastore->invalidIcon(), message, suggestionsMenu);
                  suggestionsMenu->addAction(widgetact);
                  suggestionsMenu->addSeparator();
                  auto act = new QAction(m_datastore->addColonIcon(),
                                         tr("Call ColorDialog."));
                  updateColorDialogMenu(act,
                                        property,
                                        status->name(),
                                        status->offset(),
                                        section->position,
                                        &suggestionsMenu);
                  break;
                }
                case FuzzyColorValue: {
                  auto matches =
                    m_datastore->fuzzySearchColorNames(status->name());
                  message = tr("Color name is fuzzy: %1").arg(status->name());
                  suggestionsMenu->clear();
                  auto widgetact = getWidgetAction(
                    m_datastore->invalidIcon(), message, suggestionsMenu);
                  suggestionsMenu->addAction(widgetact);
                  suggestionsMenu->addSeparator();
                  auto act = new QAction(
                    m_datastore->addColonIcon(),
                    tr("Fuzzy color %1.<br>Possible values showing below."));
                  act->setData(pos);
                  updateMenu(matches,
                             property,
                             pos,
                             &suggestionsMenu,
                             SectionType::FuzzyPropertyValue,
                             status->name(),
                             status->offset());
                  break;
                }
              }
              status = status->next();
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
        }
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

  if (m_datastore->containsProperty(newName))
    property->setPropertyNameCheck(NodeState::ValidNameState);
  else if (!m_datastore->fuzzySearchProperty(newName).isEmpty()) {
    property->setPropertyNameCheck(NodeState::FuzzyPropertyState);
  } else {
    property->setPropertyNameCheck(NodeState::BadNodeState);
  }
  property->setName(newName);
  property->setCursor(cursor);
  emit rehighlightBlock(cursor.block());
}

void
Parser::actionPropertyValueChange(PropertyNode* property,
                                  const QString& oldName,
                                  const QString& newName)
{
  auto index = property->indexOf(oldName);
  auto position = property->valuePosition(index);
  auto cursor = m_datastore->getCursorForPosition(position);

  updateTextChange(cursor, oldName, newName);

  property->setValue(index, newName);
  property->setStateFlag(index, ValidPropertyValueState);
  property->setValueOffset(index, m_datastore->getCursorForPosition(position));
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
  auto node = menuData.node;
  auto type = menuData.type;
  auto oldName = menuData.oldName;
  auto offset = menuData.offset;
  auto index = menuData.index;
  QPoint pos = act->data().toPoint();

  if (node) {
    auto newName = act->text();

    switch (type) {
      case ColorDialog: {
        auto property = qobject_cast<PropertyNode*>(node);
        //        const QColorDialog::ColorDialogOptions options =
        //          QColorDialog::ShowAlphaChannel |
        //          QColorDialog::DontUseNativeDialog;
        //        auto color =
        //          QColorDialog::getColor(Qt::black, m_editor, "Choose Color",
        //          options);
        auto colorDlg = new ExtendedColorDialog(m_editor);
        if (colorDlg->exec() == QDialog::Accepted) {
          auto color = colorDlg->primaryColor();
          if (color.isValid()) {
            QString value;
            if (color.alpha() == 255)
              value = color.name(QColor::HexRgb);
            else
              value = color.name(QColor::HexArgb);
            // copy start offset
            auto cursor = m_datastore->getCursorForPosition(offset);
            // this moves offset to end on new text.
            updateTextChange(cursor, oldName, value);
            cursor.setPosition(offset, QTextCursor::MoveAnchor);
            property->setValueName(index, value);
            property->setValueState(index, PropertyValueState::GoodValue);
            // reset offset.
            property->setValueOffset(index, cursor);
          }
        }
        break;
      }
      case FuzzyPropertyValue: {
        auto property = qobject_cast<PropertyNode*>(node);
        auto cursor = m_datastore->getCursorForPosition(offset);
        updateTextChange(cursor, oldName, newName);
        property->setValueName(index, newName);
        property->setValueState(index, PropertyValueState::GoodValue);
        property->setValueOffset(index, cursor);
        break;
      }
      case FuzzyWidgetName: {
        auto widget = qobject_cast<WidgetNode*>(node);
        auto cursor = widget->cursor();
        updateTextChange(cursor, widget->name(), newName);
        widget->setName(newName);
        widget->setWidgetCheck(NodeState::WidgetState);
        if (widget->hasSubControl()) {
          auto subcontrol = widget->subControl(pos);
          auto [type, state] =
            checkType(subcontrol->position(), subcontrol->name(), BadNodeState);
          switch (state) {
            case NodeState::SubControlState:
            case NodeState::FuzzySubControlState:
              widget->setSubControlMarkerCursor(subcontrol->cursor());
              widget->setWidgetCheck(state);
              break;
            case NodeState::PseudostateState:
            case NodeState::FuzzyPseudostateState:
              //              widget->setPseudoStateMarkerCursor(widget->extensionCursor());
              //              widget->setWidgetCheck(check);
              break;
            default:
              qWarning();
              break;
          }
        }
        break;
      }
      case BadWidgetSubControl:
      case FuzzyWidgetPseudoState:
      case FuzzyWidgetSubControl: {
        auto widget = qobject_cast<WidgetNode*>(node);
        auto subcontrol = widget->subControl(pos);
        if (subcontrol) {
          oldName = subcontrol->name();
          auto cursor = subcontrol->cursor();
          updateTextChange(cursor, oldName, newName);
          subcontrol->setName(newName);
        }
        break;
      }
      case WidgetPseudoStateMarker: {
        auto widget = qobject_cast<WidgetNode*>(node);
        if (widget->hasPseudoStates()) {
          auto pseudostate = widget->pseudoState(pos);
          if (pseudostate) {
            auto cursor = pseudostate->cursor();
            updateTextChange(cursor, "::", ":");
          }
        }
        break;
      }
      case WidgetSubControlPseudoStateMarker: {
        auto widget = qobject_cast<WidgetNode*>(node);
        if (widget->hasSubControls()) {
          auto subcontrol = widget->subControl(pos);
          if (subcontrol) {
            if (subcontrol->hasPseudoStates()) {
              auto pseudostate = widget->pseudoState(pos);
              if (pseudostate) {
                auto cursor = pseudostate->cursor();
                updateTextChange(cursor, "::", ":");
              }
            }
          }
        }
        break;
      }
      case WidgetSubControlMarker: {
        auto widget = qobject_cast<WidgetNode*>(node);
        if (widget->hasSubControls()) {
          auto subcontrol = widget->subControl(pos);
          if (subcontrol) {
            auto cursor = subcontrol->cursor();
            updateTextChange(cursor, ":", "::");
          }
        }
        break;
      }
      case WidgetName:
      case WidgetPropertyName:
      case WidgetPropertyValue:
      case WidgetPropertyMarker:
      case WidgetPropertyEndMarker:
      case WidgetStartBrace:
      case WidgetEndBrace:
        qWarning();
        break;

      case Comment:
        break;

      case SectionType::PropertyName: {
        m_datastore->setHasSuggestion(true);
        actionPropertyNameChange(qobject_cast<PropertyNode*>(node), newName);
        break;
      }
      case SectionType::PropertyValue: {
        m_datastore->setHasSuggestion(true);
        actionPropertyValueChange(
          qobject_cast<PropertyNode*>(node), oldName, newName);
        break;
      }
      case SectionType::PropertyMarker: {
        m_datastore->setHasSuggestion(true);
        actionPropertyMarker(qobject_cast<PropertyNode*>(node));
        break;
      }
      case SectionType::PropertyEndMarker: {
        m_datastore->setHasSuggestion(true);
        actionPropertyEndMarker(qobject_cast<PropertyNode*>(node));
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
