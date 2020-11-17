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

Node::Node(const QString& name,
           QTextCursor cursor,
           StylesheetEdit* editor,
           QObject* parent,
           enum NodeType type)
  : QObject(parent)
{
  d_ptr = new NodeData;
  d_ptr->cursor = cursor;
  d_ptr->type = type;
  d_ptr->editor = editor;
  d_ptr->name = name;
}

Node::Node(const Node& other)
{
  d_ptr = new NodeData(*other.d_ptr);
}

Node::~Node()
{
  delete d_ptr;
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
  return d_ptr->cursor.anchor() + d_ptr->name.length();
}

QTextCursor
Node::cursor() const
{
  return d_ptr->cursor;
}

int
Node::start() const
{
  return d_ptr->cursor.anchor();
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
      return d_ptr->name;
    case WidgetType:
      return d_ptr->name;
    case StartBraceType:
      return "{";
    case EndBraceType:
      return "}";
    case NewlineType:
      return "\n";
    case PropertyType:
      return d_ptr->name;
    case CommentType:
      return d_ptr->name;
    case BadNodeType:
      return "";
  }

  return QString();
}

QString
Node::name() const
{
  return d_ptr->name;
}

void
Node::setName(const QString& value)
{
  d_ptr->name = value;
}

int
Node::length() const
{
  return d_ptr->name.length();
}

QPair<NodeSectionType, int>
Node::isIn(QPoint pos)
{
  auto x = pos.x();
  auto y = pos.y();
  auto rect = d_ptr->editor->cursorRect(QTextCursor(d_ptr->cursor));
  auto left = rect.x();
  auto fm = d_ptr->editor->fontMetrics();
  auto width = fm.horizontalAdvance(d_ptr->name);
  auto height = fm.height();
  auto right = left + width;
  auto top = rect.y();
  auto bottom = top + height;

  if (x >= left && x <= right && y >= top && y <= bottom) {
    return qMakePair<NodeSectionType, int>(NodeSectionType::Name, 0);
  }

  return qMakePair<NodeSectionType, int>(NodeSectionType::None, -1);
}

BadBlockNode::BadBlockNode(const QString& name,
                           QTextCursor start,
                           ParserState::Errors errors,
                           StylesheetEdit* editor,
                           QObject* parent,
                           enum NodeType type)
  : Node(name, start, editor, parent, type)
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

WidgetNode::WidgetNode(const QString& name,
                       QTextCursor start,
                       StylesheetEdit* editor,
                       QObject* parent,
                       enum NodeType type)
  : Node(name, start, editor, parent, type)
{
  w_ptr = new WidgetNodeData;
}

WidgetNode::WidgetNode(const WidgetNode& other)
  : Node(other.name(),
         other.cursor(),
         other.d_ptr->editor,
         other.parent(),
         other.type())
{
  w_ptr = new WidgetNodeData(*other.w_ptr);
}

WidgetNode::~WidgetNode()
{
  delete w_ptr;
}

int
WidgetNode::length() const
{
  if (hasEndBrace()) {
    return endBraceCursor().anchor() - cursor().anchor();
  } else if (w_ptr->properties.size() > 0) {
    return w_ptr->properties.last()->length();
  } else if (hasStartBrace()) {
    return startBraceCursor().anchor() - cursor().anchor();
  } else if (hasExtension()) {
    return extensionCursor().anchor() - cursor().anchor() + extensionLength();
  } else {
    return name().length();
  }
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

QPair<NodeSectionType, int>
WidgetNode::isIn(QPoint pos)
{

  // first check if it is the widget properties.
  for (int i = 0; i < propertyCount(); i++) {
    auto isin = w_ptr->properties.at(i)->isIn(pos);
    if (isin.first == NodeSectionType::Name) {
      isin.first = NodeSectionType::WidgetPropertyName;
      return isin;
    } else if (isin.first == NodeSectionType::Value) {
      isin.first = NodeSectionType::WidgetPropertyValue;
      return isin;
    }
  }

  // then elsewhere in the widget.
  auto x = pos.x();
  auto y = pos.y();
  auto cursor = QTextCursor(d_ptr->cursor);
  auto oldPos = cursor.position();
  auto rect = d_ptr->editor->cursorRect(cursor);
  auto left = rect.x();
  auto top = rect.top();
  auto fm = d_ptr->editor->fontMetrics();
  int right = left, bottom = top;

  int end;
  end = length();
  while (true) {
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 1);
    rect = d_ptr->editor->cursorRect(cursor);
    right = (rect.right() > right ? rect.right() : right);
    bottom = (rect.bottom() > bottom ? rect.bottom() : bottom);
    if (cursor.position() == end) {
      break;
    }
  }

  if (x > left && x <= right && y > top && y <= bottom)
    return qMakePair<NodeSectionType, int>(NodeSectionType::Widget, -1);

  cursor.setPosition(oldPos);

  return qMakePair<NodeSectionType, int>(NodeSectionType::None, -1);
}

void
WidgetNode::setSubControlMarkerCursor(QTextCursor cursor)
{
  w_ptr->extension = SubControlExtension;
  w_ptr->markerPosition = cursor;
}

void
WidgetNode::setPseudoStateMarkerCursor(QTextCursor cursor)
{
  w_ptr->extension = PseudoStateExtension;
  w_ptr->markerPosition = cursor;
}

QTextCursor
WidgetNode::extensionCursor() const
{
  return w_ptr->extensionPosition;
}

int
WidgetNode::extensionPosition() const
{
  return w_ptr->extensionPosition.anchor();
}

void
WidgetNode::setExtensionCursor(QTextCursor cursor)
{
  w_ptr->extensionPosition = cursor;
}

QTextCursor
WidgetNode::markerCursor() const
{
  return w_ptr->markerPosition;
}

int
WidgetNode::markerPosition() const
{
  return w_ptr->markerPosition.anchor();
}

void
WidgetNode::setMarkerCursor(QTextCursor cursor)
{
  w_ptr->markerPosition = cursor;
}

QString
WidgetNode::extensionName() const
{
  return w_ptr->extensionName;
}

void
WidgetNode::setExtensionName(const QString& name)
{
  w_ptr->extensionName = name;
}

bool
WidgetNode::isExtensionValid() const
{
  return w_ptr->extensionValid;
}

bool
WidgetNode::isSubControl() const
{
  return (w_ptr->extension == SubControlExtension);
}

bool
WidgetNode::isPseudoState() const
{
  return (w_ptr->extension == PseudoStateExtension);
}

void
WidgetNode::setExtensionValid(bool valid)
{
  w_ptr->extensionValid = valid;
}

bool
WidgetNode::hasExtension() const
{
  return (w_ptr->extension == SubControlExtension ||
          w_ptr->extension == PseudoStateExtension);
}

int
WidgetNode::extensionLength() const
{
  return w_ptr->extensionName.length();
}

bool
WidgetNode::hasStartBrace() const
{
  return (w_ptr->startBrace);
}

void
WidgetNode::setStartBraceCursor(QTextCursor cursor)
{
  w_ptr->startBrace = true;
  w_ptr->startBracePosition = cursor;
}

QTextCursor
WidgetNode::startBraceCursor() const
{
  return w_ptr->startBracePosition;
}

int
WidgetNode::startBracePosition() const
{
  return w_ptr->startBracePosition.anchor();
}

void
WidgetNode::removeStartBrace()
{
  w_ptr->startBrace = false;
}

bool
WidgetNode::hasEndBrace() const
{
  return (w_ptr->endBrace);
}

void
WidgetNode::setEndBraceCursor(QTextCursor cursor)
{
  w_ptr->endBrace = true;
  w_ptr->endBracePosition = cursor;
}

QTextCursor
WidgetNode::endBraceCursor() const
{
  return w_ptr->endBracePosition;
}

int
WidgetNode::endBracePosition() const
{
  return w_ptr->endBracePosition.anchor();
}

void
WidgetNode::removeEndBrace()
{
  w_ptr->endBrace = false;
  //  w_ptr->endBraceOffset = 0;
}

bool
WidgetNode::offsetsMatch() const
{
  return (w_ptr->startBrace && w_ptr->endBrace);
}

void
WidgetNode::addProperty(PropertyNode* property)
{
  w_ptr->properties.append(property);
}

PropertyNode*
WidgetNode::property(int index) const
{
  return w_ptr->properties.at(index);
}

int
WidgetNode::propertyCount() const
{
  return w_ptr->properties.size();
}

PropertyNode::PropertyNode(const QString& name,
                           QTextCursor start,
                           StylesheetEdit* editor,
                           QObject* parent,
                           enum NodeType type)
  : WidgetNode(name, start, editor, parent, type)
{
  p_ptr = new PropertyNodeData;
}

PropertyNode::PropertyNode(const PropertyNode& other)
  : WidgetNode(other.name(),
         other.cursor(),
         other.d_ptr->editor,
         other.parent(),
         other.type())
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

QList<PropertyValueCheck>
PropertyNode::checks() const
{
  return p_ptr->checks;
}

QList<QTextCursor>
PropertyNode::positionCursors() const
{
  return p_ptr->cursors;
}

void
PropertyNode::setCursor(int index, QTextCursor position)
{
  if (index >= 0 && index < p_ptr->cursors.length()) {
    p_ptr->cursors.replace(index, position);
  }
}

QList<AttributeType>
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
PropertyNode::setValue(int index, const QString& value)
{
  if (index >= 0 && index < p_ptr->values.length()) {
    p_ptr->values.replace(index, value);
  }
}

void
PropertyNode::setChecks(const QList<PropertyValueCheck>& checks)
{
  p_ptr->checks = checks;
}

void
PropertyNode::setCheck(int index, PropertyValueCheck check)
{
  if (index >= 0 && index < p_ptr->checks.length()) {
    p_ptr->checks.replace(index, check);
  }
}

PropertyValueCheck
PropertyNode::check(int index)
{
  if (index >= 0 && index < p_ptr->values.size())
    return p_ptr->checks.at(index);
  return PropertyValueCheck::NoCheck;
}

void
PropertyNode::setCursors(const QList<QTextCursor>& positions)
{
  p_ptr->cursors = positions;
}

int
PropertyNode::position(int index)
{
  if (index >= 0 && index < p_ptr->cursors.size())
    return p_ptr->cursors.at(index).anchor();
  return -1;
}

void
PropertyNode::addValue(
  const QString& value,
  PropertyValueCheck check,
  QTextCursor position,
  AttributeType attType = NoAttributeValue)
{
  p_ptr->values.append(value);
  p_ptr->checks.append(check);
  p_ptr->cursors.append(position);
  p_ptr->attributeTypes.append(attType);
}

QString
PropertyNode::value(int index)
{
  if (index >= 0 && index < p_ptr->values.size())
    return p_ptr->values.at(index);
  return QString();
}

bool
PropertyNode::setBadCheck(PropertyValueCheck check, int index)
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
  return p_ptr->cursors.size();
}

PropertyValueCheck
PropertyNode::isValueValid(int index)
{
  if (index > 00 && index < count()) {
    return p_ptr->checks.at(index);
  }

  // default to bad.
  return PropertyValueCheck::BadValue;
}

int
PropertyNode::end() const
{
  if (p_ptr->cursors.isEmpty() || p_ptr->values.isEmpty()) {
    return start();
  }

  return start() + length();
}

int
PropertyNode::length() const
{
  if (p_ptr->cursors.isEmpty()) {
    if (!hasPropertyMarker()) {
      return d_ptr->name.length();
    } else {
      return propertyMarkerCursor().anchor() + 1;
    }
  } else {
    return p_ptr->cursors.last().anchor() + p_ptr->values.last().length();
  }

  return 0;
}

bool
PropertyNode::hasPropertyMarker() const
{
  return p_ptr->propertyState.testFlag(PropertyCheck::PropertyMarker);
}

void
PropertyNode::setPropertyMarker(bool exists)
{
  p_ptr->propertyState.setFlag(PropertyCheck::PropertyMarker, exists);
}

bool
PropertyNode::isValidPropertyName() const
{
  return p_ptr->propertyState.testFlag(PropertyCheck::ValidName);
}

void
PropertyNode::setValidPropertyName(bool valid)
{
  p_ptr->propertyState.setFlag(PropertyCheck::ValidName, valid);
}

bool
PropertyNode::isValidProperty()
{
  return p_ptr->propertyState.testFlag(PropertyCheck::GoodProperty);
}

QTextCursor
PropertyNode::propertyMarkerCursor() const
{
  return p_ptr->propertyMarkerCursor;
}

int
PropertyNode::propertyMarkerPosition() const
{
  return p_ptr->propertyMarkerCursor.anchor();
}

void
PropertyNode::setPropertyMarkerCursor(QTextCursor position)
{
  p_ptr->propertyMarkerCursor = position;
}

bool
PropertyNode::hasPropertyEndMarker()
{
  return p_ptr->propertyState.testFlag(PropertyCheck::PropertyEndMarker);
  ;
}

void
PropertyNode::setPropertyEndMarker(bool exists)
{
  p_ptr->propertyState.setFlag(PropertyCheck::PropertyEndMarker, exists);
}

QTextCursor
PropertyNode::propertyEndMarkerCursor()
{
  return p_ptr->endMarkerCursor;
}

int
PropertyNode::propertyEndMarkerPosition()
{
  return p_ptr->endMarkerCursor.anchor();
}

void
PropertyNode::setPropertyEndMarkerCursor(QTextCursor position)
{
  p_ptr->endMarkerCursor = position;
}

QPair<NodeSectionType, int>
PropertyNode::isIn(QPoint pos)
{
  auto x = pos.x();
  auto y = pos.y();
  auto cursor = QTextCursor(d_ptr->cursor);
  auto rect = d_ptr->editor->cursorRect(cursor);
  auto left = rect.x();
  auto fm = d_ptr->editor->fontMetrics();
  auto width =
    fm.horizontalAdvance(d_ptr->name); // initially just property name
  auto height = fm.height();
  auto right = left + width;
  auto top = rect.y();
  auto bottom = top + height;
  auto propLeft = left;
  QTextCursor propCursor(cursor);
  propCursor.movePosition(
    QTextCursor::Right, QTextCursor::MoveAnchor, length());
  rect = d_ptr->editor->cursorRect(propCursor);
  //  auto propRight = rect.x()/* + fm.horizontalAdvance(values().last())*/;

  // is it inside the entire property boundaries.
  if (y >= top && y <= bottom && x >= propLeft && x <= rect.right()) {
    // Property name.
    if (x >= left && x <= right) {
      return qMakePair<NodeSectionType, int>(NodeSectionType::Name, 0);
    }

    // Property values.
    for (int i = 0; i < positionCursors().count(); i++) {
      auto value = values().at(i);
      auto offset = positionCursors().at(i).anchor();
      auto valCursor(cursor);
      auto oldRight = right;
      valCursor.movePosition(
        QTextCursor::Right, QTextCursor::MoveAnchor, offset);
      rect = d_ptr->editor->cursorRect(valCursor);
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
  if (pos < d_ptr->name.length()) {
    return PropertyStatus(true, d_ptr->name);

  } else {
    for (int i = 0; i < p_ptr->values.size(); i++) {
      auto value = p_ptr->values.at(i);
      auto offset = p_ptr->cursors.at(i).anchor();

      if (pos >= offset && pos < offset + value.length()) {
        return PropertyStatus(false, value, offset);
      }
    }
  }

  return PropertyStatus();
}

void
PropertyNode::setAttributeTypes(
  const QList<AttributeType>& attributeTypes)
{
  p_ptr->attributeTypes = attributeTypes;
}

CommentNode::CommentNode(QTextCursor start,
                         StylesheetEdit* editor,
                         QObject* parent,
                         enum NodeType type)
  : WidgetNode(QString(), start, editor, parent, type)
{
  c_ptr = new CommentNodeData;
}

CommentNode::CommentNode(const CommentNode& other)
  : WidgetNode(other.name(),
         other.cursor(),
         other.d_ptr->editor,
         other.parent(),
         other.type())
{
  c_ptr = new CommentNodeData(*other.c_ptr);
}

CommentNode::~CommentNode()
{
  delete c_ptr;
}

void
CommentNode::append(QChar c)
{
  d_ptr->name.append(c);
}

void
CommentNode::append(QString text)
{
  d_ptr->name.append(text);
}

int
CommentNode::end() const
{
  return d_ptr->cursor.anchor() + d_ptr->name.length();
}

int
CommentNode::length() const
{
  if (hasEndComment()) {
    return endCommentCursor().anchor() - start() + 2;
  } else if (d_ptr->name.length() > 0) {
    return c_ptr->cursor.anchor() + d_ptr->name.length() - start();
  } else {
    return 2; // length of start comment
  }
}

QTextCursor
CommentNode::textCursor() const
{
  return c_ptr->cursor;
}

void
CommentNode::setTextCursor(QTextCursor cursor)
{
  c_ptr->cursor = cursor;
}

int
CommentNode::textPosition() const
{
  return c_ptr->cursor.anchor();
}

bool
CommentNode::hasEndComment() const
{
  return c_ptr->endCommentExists;
}

void
CommentNode::setEndCommentExists(bool exists)
{
  c_ptr->endCommentExists = exists;
}

QTextCursor
CommentNode::endCommentCursor() const
{
  return c_ptr->endCursor;
}

void
CommentNode::setEndCommentCursor(QTextCursor cursor)
{
  c_ptr->endCursor = cursor;
}

QPair<NodeSectionType, int>
CommentNode::isIn(QPoint pos)
{
  auto x = pos.x();
  auto y = pos.y();
  auto cursor = QTextCursor(d_ptr->cursor);
  auto oldPos = cursor.position();
  auto rect = d_ptr->editor->cursorRect(cursor);
  auto left = rect.x();
  auto top = rect.top();
  auto fm = d_ptr->editor->fontMetrics();
  int right = left, bottom = top;

  int end = length();
  while (true) {
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 1);
    rect = d_ptr->editor->cursorRect(cursor);
    right = (rect.right() > right ? rect.right() : right);
    bottom = (rect.bottom() > bottom ? rect.bottom() : bottom);
    if (cursor.position() == end) {
      break;
    }
  }

  if (x > left && x <= right && y > top && y <= bottom)
    return qMakePair<NodeSectionType, int>(NodeSectionType::Comment, -1);

  cursor.setPosition(oldPos);

  return qMakePair<NodeSectionType, int>(NodeSectionType::None, -1);
}

NewlineNode::NewlineNode(QTextCursor start,
                         StylesheetEdit* editor,
                         QObject* parent,
                         enum NodeType type)
  : WidgetNode("\n", start, editor, parent, type)
{}

StartBraceNode::StartBraceNode(QTextCursor start,
                               StylesheetEdit* editor,
                               QObject* parent,
                               enum NodeType type)
  : WidgetNode("{", start, editor, parent, type)
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
                           StylesheetEdit* editor,
                           QObject* parent,
                           enum NodeType type)
  : WidgetNode("}", start, editor, parent, type)
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

BadStartCommentNode::BadStartCommentNode(QTextCursor start,
                                         ParserState::Errors errors,
                                         StylesheetEdit* editor,
                                         QObject* parent,
                                         enum NodeType type)
  : WidgetNode("/*", start, editor, parent, type)
  , BadNode(errors)
{}

int
BadStartCommentNode::end() const
{
  return d_ptr->cursor.anchor() + 2;
}

BadEndCommentNode::BadEndCommentNode(QTextCursor start,
                                     ParserState::Errors errors,
                                     StylesheetEdit* editor,
                                     QObject* parent,
                                     enum NodeType type)
  : WidgetNode("*/", start, editor, parent, type)
  , BadNode(errors)
{}

int
BadEndCommentNode::end() const
{
  return d_ptr->cursor.anchor() + 2;
}

// PseudoStateMarkerNode::PseudoStateMarkerNode(QTextCursor start,
//                                             StylesheetEdit* parent,
//                                             enum NodeType type)
//  : ColonNode(start, parent, type)
//{}

// PropertyEndMarkerNode::PropertyEndMarkerNode(QTextCursor start,
//                                             StylesheetEdit* parent,
//                                             enum NodeType type)
//  : SemiColonNode(start, parent, type)
//{}

// PropertyEndNode::PropertyEndNode(QTextCursor start,
//                                 StylesheetEdit* parent,
//                                 enum NodeType type)
//  : Node(start, parent, type)
//{}
