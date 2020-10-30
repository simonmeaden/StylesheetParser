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
#include "node.h"

#include <QtDebug>

#include "stylesheetparser/stylesheetedit.h"

Node::Node(QTextCursor cursor, StylesheetEdit* parent, enum NodeType type)
  : QObject(parent)
  , previous(nullptr)
  , next(nullptr)
{
  d_ptr = new NodeData;
  d_ptr->cursor = cursor;
  d_ptr->type = type;
  d_ptr->parent = parent;
}

Node::Node(const Node& other)
{
  d_ptr = new NodeData(*other.d_ptr);
}

Node::~Node()
{
  delete d_ptr;
}

int
Node::start() const
{
  return d_ptr->cursor.anchor();
}

void
Node::setStart(int position)
{
  d_ptr->cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
  d_ptr->cursor.movePosition(
    QTextCursor::Right, QTextCursor::MoveAnchor, position);
}

void
Node::moveStart(int count)
{
  QTextCursor::MoveOperation moveOp =
    (count < 0 ? QTextCursor::Left : QTextCursor::Right);
  d_ptr->cursor.movePosition(moveOp, QTextCursor::MoveAnchor, qAbs(count));
}

int
Node::end() const
{
  return d_ptr->cursor.anchor();
}

QTextCursor
Node::cursor() const
{
  return d_ptr->cursor;
}

enum NodeType
Node::type() const
{
  return d_ptr->type;
}

QString
Node::toString() const
{
  switch (d_ptr->type) {
    case NodeType:
      return "Node";

    case BaseNodeType:
      return "Base";

    case CharNodeType:
      return "Char";

    case ColonNodeType:
      return "Colon";

    case NameType:
      return "Name";

    case WidgetType:
      return "Widget";

    case SubControlType:
      return "Sub Control";

    case SubControlMarkerType:
      return "Sub Control Marker";

    case PseudoStateType:
      return "PseudoState";

    case PseudoStateMarkerType:
      return "PseudoState Marker";

    case SemiColonType:
      return "SemiColon";

    case StartBraceType:
      return "Start Brace";

    case EndBraceType:
      return "End Brace";

    case NewlineType:
      return "Newline";

    case PropertyType:
      return "Property";

    case PropertyEndType:
      return "Property End";

    case BadNodeType:
      return "Bad";
  }

  return QString();
}

NameNode::NameNode(const QString& name)
{
  n_ptr = new NameNodeData;
  n_ptr->name = name;
}

NameNode::NameNode(const NameNode& other)
{
  n_ptr = new NameNodeData(*other.n_ptr);
}

NameNode::~NameNode()
{
  delete n_ptr;
}

QString
NameNode::name() const
{
  return n_ptr->name;
}

void
NameNode::setName(const QString& value)
{
  n_ptr->name = value;
}

int
NameNode::length() const
{
  return n_ptr->name.length();
}

NamedNode::NamedNode(const QString& name,
                     QTextCursor start,
                     StylesheetEdit* parent,
                     enum NodeType type)
  : Node(start, parent, type)
  , NameNode(name)
{}

QPair<NodeSectionType, int>
NamedNode::isIn(QPoint pos)
{
  auto x = pos.x();
  auto y = pos.y();
  auto rect = d_ptr->parent->cursorRect(QTextCursor(d_ptr->cursor));
  auto left = rect.x();
  auto fm = d_ptr->parent->fontMetrics();
  auto width = fm.horizontalAdvance(n_ptr->name);
  auto height = fm.height();
  auto right = left + width;
  auto top = rect.y();
  auto bottom = top + height;

  if (x >= left && x <= right && y >= top && y <= bottom) {
    return qMakePair<NodeSectionType, int>(NodeSectionType::Name, 0);
  }

  return qMakePair<NodeSectionType, int>(NodeSectionType::None, -1);
}

int
NamedNode::end() const
{
  return start() + length();
}

BadBlockNode::BadBlockNode(const QString& name,
                           QTextCursor start,
                           ParserState::Errors errors,
                           StylesheetEdit* parent,
                           enum NodeType type)
  : NamedNode(name, start, parent, type)
  , BadNode(errors)
{}

BadNode::BadNode(ParserState::Errors errors)
  : m_errors(errors)
{}

int
BadBlockNode::end() const
{
  return d_ptr->cursor.anchor() + name().length();
}

ParserState::Errors
BadNode::errors() const
{
  return m_errors;
}

void
BadNode::setError(const ParserState::Errors& errors)
{
  m_errors = errors;
}

BadSubControlMarkerNode::BadSubControlMarkerNode(QTextCursor start,
                                                 ParserState::Errors errors,
                                                 StylesheetEdit* parent,
                                                 enum NodeType type)
  : SubControlMarkerNode(start, parent, type)
  , BadNode(errors)
{}

BadPseudoStateMarkerNode::BadPseudoStateMarkerNode(QTextCursor start,
                                                   ParserState::Errors errors,
                                                   StylesheetEdit* parent,
                                                   enum NodeType type)
  : PseudoStateMarkerNode(start, parent, type)
  , BadNode(errors)
{}

WidgetNode::WidgetNode(const QString& name,
                       QTextCursor start,
                       StylesheetEdit* parent,
                       enum NodeType type)
  : NamedNode(name, start, parent, type)
{
  w_ptr = new WidgetNodeData;
}

WidgetNode::WidgetNode(const WidgetNode& other)
  : NamedNode(other.name(), other.cursor(), other.d_ptr->parent, other.type())
{
  w_ptr = new WidgetNodeData(*other.w_ptr);
}

WidgetNode::~WidgetNode()
{
  delete w_ptr;
}

bool
WidgetNode::isWidgetValid() const
{
  return w_ptr->widgetValid;
}

void
WidgetNode::setWidgetValid(bool widgetValid)
{
  w_ptr->widgetValid = widgetValid;
}

PropertyNode::PropertyNode(const QString& name,
                           QTextCursor start,
                           StylesheetEdit* parent,
                           enum NodeType type)
  : NamedNode(name, start, parent, type)
{
  p_ptr = new PropertyNodeData;
}

PropertyNode::PropertyNode(const PropertyNode& other)
  : NamedNode(other.name(), other.cursor(), other.d_ptr->parent, other.type())
{
  p_ptr = new PropertyNodeData(*other.p_ptr);
}

PropertyNode::~PropertyNode()
{
  delete p_ptr;
}

QStringList
PropertyNode::values() const
{
  return p_ptr->values;
}

QList<PropertyCheck>
PropertyNode::checks() const
{
  return p_ptr->checks;
}

QList<int>
PropertyNode::offsets() const
{
  return p_ptr->offsets;
}

QList<DataStore::AttributeType>
PropertyNode::attributeTypes() const
{
  return p_ptr->attributeTypes;
}

void
PropertyNode::setValues(const QStringList& values)
{
  p_ptr->values = values;
}

void
PropertyNode::setChecks(const QList<PropertyCheck>& checks)
{
  p_ptr->checks = checks;
}

void
PropertyNode::setOffsets(const QList<int>& offsets)
{
  p_ptr->offsets = offsets;
}

void
PropertyNode::addValue(
  const QString& value,
  PropertyCheck check,
  int offset,
  DataStore::AttributeType attType = DataStore::NoAttributeValue)
{
  p_ptr->values.append(value);
  p_ptr->checks.append(check);
  p_ptr->offsets.append(offset);
  p_ptr->attributeTypes.append(attType);
}

void
PropertyNode::correctValue(int index, const QString& value)
{
  p_ptr->values.replace(index, value);
  p_ptr->checks.replace(index, GoodValue);
}

bool
PropertyNode::setBadCheck(PropertyCheck check, int index)
{
  if (index == -1 && !p_ptr->checks.isEmpty()) {
    p_ptr->checks[p_ptr->checks.length() - 1] = check;
    return true;

  } else if (index >= 0 && index < p_ptr->checks.length()) {
    p_ptr->checks[index] = check;
    return true;
  }

  return false;
}

int
PropertyNode::count()
{
  return p_ptr->offsets.size();
}

PropertyCheck
PropertyNode::isValueValid(int index)
{
  if (index > 00 && index < count()) {
    return p_ptr->checks.at(index);
  }

  // default to bad.
  return PropertyCheck::BadValue;
}

int
PropertyNode::end() const
{
  if (p_ptr->offsets.isEmpty() || p_ptr->values.isEmpty()) {
    return start();
  }

  return start() + length();
}

int
PropertyNode::length() const
{
  auto value = n_ptr->name.length();

  if (p_ptr->offsets.isEmpty()) {
    if (hasPropertyMarker()) {
      value += propertyMarkerOffset() + 1;
    }
  } else {
    value += p_ptr->values.last().length();
  }

  return value;
}

bool
PropertyNode::hasPropertyMarker() const
{
  return p_ptr->propertyMarkerExists;
}

void
PropertyNode::setPropertyMarkerExists(bool propertyMarker)
{
  p_ptr->propertyMarkerExists = propertyMarker;
}

bool
PropertyNode::isValidProperty() const
{
  return p_ptr->validProperty;
}

void
PropertyNode::setValidProperty(bool validProperty)
{
  p_ptr->validProperty = validProperty;
}

int
PropertyNode::propertyMarkerOffset() const
{
  return p_ptr->propertyMarkerOffset;
}

void
PropertyNode::setPropertyMarkerOffset(int propertymarkerOffset)
{
  p_ptr->propertyMarkerOffset = propertymarkerOffset;
}

void
PropertyNode::incrementOffsets(int increment, int startIndex)
{
  for (int i = startIndex; i < p_ptr->offsets.length(); i++) {
    p_ptr->offsets[i] = p_ptr->offsets.at(i) + increment;
  }
}

QPair<NodeSectionType, int>
PropertyNode::isIn(QPoint pos)
{
  auto x = pos.x();
  auto y = pos.y();
  auto cursor = QTextCursor(d_ptr->cursor);
  auto rect = d_ptr->parent->cursorRect(cursor);
  auto left = rect.x();
  auto fm = d_ptr->parent->fontMetrics();
  auto width =
    fm.horizontalAdvance(n_ptr->name); // initially just property name
  auto height = fm.height();
  auto right = left + width;
  auto top = rect.y();
  auto bottom = top + height;
  auto propLeft = left;
  QTextCursor propCursor(cursor);
  propCursor.movePosition(
    QTextCursor::Right, QTextCursor::MoveAnchor, offsets().last());
  rect = d_ptr->parent->cursorRect(propCursor);
  auto propRight = rect.x() + fm.horizontalAdvance(values().last());

  // is it inside the entire property boundaries.
  if (y >= top && y <= bottom && x >= propLeft && x <= propRight) {
    // Property name.
    if (x >= left && x <= right) {
      return qMakePair<NodeSectionType, int>(NodeSectionType::Name, 0);
    }

    // Property values.
    for (int i = 0; i < offsets().count(); i++) {
      auto value = values().at(i);
      auto offset = offsets().at(i);
      auto valCursor(cursor);
      auto oldRight = right;
      valCursor.movePosition(
        QTextCursor::Right, QTextCursor::MoveAnchor, offset);
      rect = d_ptr->parent->cursorRect(valCursor);
      width = fm.horizontalAdvance(value);
      left = rect.x();
      right = left + width + 1;

      if (x >= oldRight && x <= left) {
        // between values or name and first value.
        return qMakePair<NodeSectionType, int>(NodeSectionType::None, -1);
      }

      if (x >= left && x <= right) {
        // inside a value.
        return qMakePair<NodeSectionType, int>(NodeSectionType::Value, i);
      }
    }
  } // else return not inside.

  return qMakePair<NodeSectionType, int>(NodeSectionType::None, -1);
}

PropertyStatus
PropertyNode::isProperty(int pos) const
{
  if (pos < n_ptr->name.length()) {
    return PropertyStatus(true, n_ptr->name);

  } else {
    for (int i = 0; i < p_ptr->values.size(); i++) {
      auto value = p_ptr->values.at(i);
      auto offset = p_ptr->offsets.at(i);

      if (pos >= offset && pos < offset + value.length()) {
        return PropertyStatus(false, value, offset);
      }
    }
  }

  return PropertyStatus();
}

void
PropertyNode::setAttributeTypes(
  const QList<DataStore::AttributeType>& attributeTypes)
{
  p_ptr->attributeTypes = attributeTypes;
}

SubControlNode::SubControlNode(const QString& name,
                               QTextCursor start,
                               StylesheetEdit* parent,
                               enum NodeType type)
  : NamedNode(name, start, parent, type)
{
  s_ptr = new NodeStateData;
}

SubControlNode::SubControlNode(const SubControlNode& other)
  : NamedNode(other.name(), other.cursor(), other.d_ptr->parent, other.type())
{
  s_ptr = new NodeStateData(*other.s_ptr);
}

SubControlNode::~SubControlNode()
{
  delete s_ptr;
}

int
SubControlNode::end() const
{
  return d_ptr->cursor.anchor() + n_ptr->name.length();
}

bool
SubControlNode::isStateValid() const
{
  return s_ptr->stateValid;
}

void
SubControlNode::setStateValid(bool stateValid)
{
  s_ptr->stateValid = stateValid;
}

PseudoStateNode::PseudoStateNode(const QString& name,
                                 QTextCursor start,
                                 StylesheetEdit* parent,
                                 enum NodeType type)
  : NamedNode(name, start, parent, type)
{
  s_ptr = new NodeStateData;
}

PseudoStateNode::PseudoStateNode(const PseudoStateNode& other)
  : NamedNode(other.name(), other.cursor(), other.d_ptr->parent, other.type())
{
  s_ptr = new NodeStateData(*other.s_ptr);
}

PseudoStateNode::~PseudoStateNode()
{
  delete s_ptr;
}

int
PseudoStateNode::end() const
{
  return d_ptr->cursor.anchor() + n_ptr->name.length();
}

bool
PseudoStateNode::isStateValid() const
{
  return s_ptr->stateValid;
}

void
PseudoStateNode::setStateValid(bool stateValid)
{
  s_ptr->stateValid = stateValid;
}

CommentNode::CommentNode(QTextCursor start,
                         StylesheetEdit* parent,
                         enum NodeType type)
  : NamedNode(QString(), start, parent, type)
{}

void
CommentNode::append(QChar c)
{
  n_ptr->name.append(c);
}

void
CommentNode::append(QString text)
{
  n_ptr->name.append(text);
}

int
CommentNode::end() const
{
  return d_ptr->cursor.anchor() + n_ptr->name.length();
}

QPair<NodeSectionType, int> CommentNode::isIn(QPoint /*pos*/)
{
  // TODO isIn for comments.
  return qMakePair<NodeSectionType, int>(NodeSectionType::None, -1);
}

ColonNode::ColonNode(QTextCursor start,
                     StylesheetEdit* parent,
                     enum NodeType type)
  : NamedNode(":", start, parent, type)
{}

SubControlMarkerNode::SubControlMarkerNode(QTextCursor start,
                                           StylesheetEdit* parent,
                                           enum NodeType type)
  : NamedNode("::", start, parent, type)
{}

int
SubControlMarkerNode::end() const
{
  return d_ptr->cursor.anchor() + 2;
}

SemiColonNode::SemiColonNode(QTextCursor start,
                             StylesheetEdit* parent,
                             enum NodeType type)
  : NamedNode(";", start, parent, type)
{}

NewlineNode::NewlineNode(QTextCursor start,
                         StylesheetEdit* parent,
                         enum NodeType type)
  : NamedNode("\n", start, parent, type)
{}

StartBraceNode::StartBraceNode(QTextCursor start,
                               StylesheetEdit* parent,
                               enum NodeType type)
  : NamedNode("{", start, parent, type)
  , m_isBraceAtCursor(false)
  , m_endBrace(nullptr)
{}

bool
StartBraceNode::isBraceAtCursor() const
{
  return m_isBraceAtCursor;
}

void
StartBraceNode::setBraceAtCursor(bool isFlagBrace)
{
  m_isBraceAtCursor = isFlagBrace;
}

void
StartBraceNode::setEndBrace(EndBraceNode* endBrace)
{
  m_endBrace = endBrace;
}

bool
StartBraceNode::hasEndBrace()
{
  return (m_endBrace != nullptr);
}

EndBraceNode*
StartBraceNode::endBrace() const
{
  return m_endBrace;
}

EndBraceNode::EndBraceNode(QTextCursor start,
                           StylesheetEdit* parent,
                           enum NodeType type)
  : NamedNode("}", start, parent, type)
  , m_isBraceAtCursor(false)
  , m_startBrace(nullptr)
{}

bool
EndBraceNode::isBraceAtCursor() const
{
  return m_isBraceAtCursor;
}

void
EndBraceNode::setBraceAtCursor(bool isFlagBrace)
{
  m_isBraceAtCursor = isFlagBrace;
}

void
EndBraceNode::setStartNode(StartBraceNode* startNode)
{
  m_startBrace = startNode;
}

bool
EndBraceNode::hasStartBrace()
{
  return (m_startBrace != nullptr);
}

StartBraceNode*
EndBraceNode::startBrace() const
{
  return m_startBrace;
}

StartCommentNode::StartCommentNode(QTextCursor start,
                                   StylesheetEdit* parent,
                                   enum NodeType type)
  : NamedNode("/*", start, parent, type)
{}

int
StartCommentNode::end() const
{
  return d_ptr->cursor.anchor() + 2;
}

EndCommentNode::EndCommentNode(QTextCursor start,
                               StylesheetEdit* parent,
                               enum NodeType type)
  : NamedNode("*/", start, parent, type)
{}

int
EndCommentNode::end() const
{
  return d_ptr->cursor.anchor() + 2;
}

PseudoStateMarkerNode::PseudoStateMarkerNode(QTextCursor start,
                                             StylesheetEdit* parent,
                                             enum NodeType type)
  : ColonNode(start, parent, type)
{}

PropertyEndMarkerNode::PropertyEndMarkerNode(QTextCursor start,
                                             StylesheetEdit* parent,
                                             enum NodeType type)
  : SemiColonNode(start, parent, type)
{}

PropertyEndNode::PropertyEndNode(QTextCursor start,
                                 StylesheetEdit* parent,
                                 enum NodeType type)
  : Node(start, parent, type)
{}
