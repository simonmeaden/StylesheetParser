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

#include "stylesheetedit_p.h"
#include "stylesheetparser/stylesheetedit.h"

Node::Node(const QString& name,
           QTextCursor cursor,
           StylesheetEditor* editor,
           QObject* parent,
           enum NodeType type)
  : QObject(parent)
{
  node_ptr = new NodeData;
  node_ptr->cursor = cursor;
  node_ptr->type = type;
  node_ptr->editor = editor;
  node_ptr->name = name;
}

Node::Node(const Node& other)
{
  node_ptr = new NodeData(*other.node_ptr);
}

Node::~Node()
{
  delete node_ptr;
}

void
Node::setStart(int position)
{
  node_ptr->cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
  node_ptr->cursor.movePosition(
    QTextCursor::Right, QTextCursor::MoveAnchor, position);
}

void
Node::moveStart(int count)
{
  QTextCursor::MoveOperation moveOp =
    (count < 0 ? QTextCursor::Left : QTextCursor::Right);
  node_ptr->cursor.movePosition(moveOp, QTextCursor::MoveAnchor, qAbs(count));
}

int
Node::end() const
{
  return node_ptr->cursor.anchor() + node_ptr->name.length();
}

int
Node::pointWidth() const
{
  auto fm = node_ptr->editor->fontMetrics();
  return fm.horizontalAdvance(node_ptr->name);
}

int
Node::pointHeight() const
{
  auto fm = node_ptr->editor->fontMetrics();
  return fm.height();
}

QTextCursor
Node::cursor() const
{
  return node_ptr->cursor;
}

void
Node::setCursor(QTextCursor cursor)
{
  node_ptr->cursor = cursor;
}

int
Node::start() const
{
  return node_ptr->cursor.anchor();
}

enum NodeType
Node::type() const
{
  return node_ptr->type;
}

QString
Node::toString() const
{
  switch (node_ptr->type) {
    case NodeType:
      return node_ptr->name;
    case WidgetType:
      return node_ptr->name;
    case StartBraceType:
      return "{";
    case EndBraceType:
      return "}";
    case NewlineType:
      return "\n";
    case PropertyType:
      return node_ptr->name;
    case CommentType:
      return node_ptr->name;
    case BadNodeType:
      return "";
  }

  return QString();
}

QString
Node::name() const
{
  return node_ptr->name;
}

void
Node::setName(const QString& value)
{
  node_ptr->name = value;
}

int
Node::length() const
{
  return node_ptr->name.length();
}

NodeSection*
Node::isIn(QPoint pos)
{
  auto x = pos.x();
  auto y = pos.y();
  auto rect = node_ptr->editor->cursorRect(QTextCursor(node_ptr->cursor));
  auto left = rect.x();
  auto width = pointWidth();
  auto height = pointHeight();
  auto right = left + width;
  auto top = rect.y();
  auto bottom = top + height;

  if (x >= left && x <= right && y >= top && y <= bottom) {
    return new NodeSection(SectionType::Name, -1);
  }

  return new NodeSection(SectionType::None, -1);
}

BadBlockNode::BadBlockNode(const QString& name,
                           QTextCursor start,
                           ParserState::Errors errors,
                           StylesheetEditor* editor,
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
  return node_ptr->cursor.anchor() + name().length();
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
                       StylesheetEditor* editor,
                       QObject* parent,
                       enum NodeType type)
  : Node(name, start, editor, parent, type)
{
  widget_ptr = new WidgetNodeData;
}

WidgetNode::WidgetNode(const WidgetNode& other)
  : Node(other.name(),
         other.cursor(),
         other.node_ptr->editor,
         other.parent(),
         other.type())
{
  widget_ptr = new WidgetNodeData(*other.widget_ptr);
}

WidgetNode::~WidgetNode()
{
  delete widget_ptr;
}

int
WidgetNode::length() const
{
  if (hasEndBrace()) {
    return endBraceCursor().anchor() - cursor().anchor();
  } else if (widget_ptr->properties.size() > 0) {
    return widget_ptr->properties.last()->length();
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
  return widget_ptr->widgetValid;
}

void
WidgetNode::setWidgetValid(bool widgetValid)
{
  widget_ptr->widgetValid = widgetValid;
}

NodeSection*
WidgetNode::isIn(QPoint pos)
{

  // this just checks the property name.
  auto isin = Node::isIn(pos);
  if (isin->type == SectionType::Name) {
    isin->type = SectionType::WidgetName;
    return isin;
  }

  QRect rect;
  auto x = pos.x();
  auto y = pos.y();
  int left, right;
  auto fm = node_ptr->editor->fontMetrics();
  int top, bottom;
  QTextCursor cursor;

  if (hasMarker()) {
    cursor = markerCursor();
    rect = node_ptr->editor->cursorRect(cursor);
    top = rect.y();
    bottom = top + rect.height();
    left = rect.x();
    if (isSubControl())
      right = left + fm.horizontalAdvance("::");
    else
      right = left + fm.horizontalAdvance(":");
    if (x >= left && x <= right && y >= top && y < bottom) {
      if (isSubControl())
        isin->type = SectionType::WidgetSubControlMarker;
      else
        isin->type = SectionType::WidgetPseudoStateMarker;
      return isin;
    }
  }

  if (hasExtension()) {
    cursor = extensionCursor();
    rect = node_ptr->editor->cursorRect(cursor);
    top = rect.y();
    bottom = top + rect.height();
    left = rect.x();
    right = left + fm.horizontalAdvance(extensionName());
    if (x >= left && x <= right && y >= top && y < bottom) {
      if (isSubControl())
        isin->type = SectionType::WidgetSubControl;
      else
        isin->type = SectionType::WidgetPseudoState;
      return isin;
    }
  }

  if (hasStartBrace()) {
    cursor = startBraceCursor();
    rect = node_ptr->editor->cursorRect(cursor);
    top = rect.y();
    bottom = top + rect.height();
    left = rect.x();
    right = left + fm.horizontalAdvance("{");
    if (x >= left && x <= right && y >= top && y < bottom) {
      isin->type = SectionType::WidgetStartBrace;
      return isin;
    }
  }

  // then value(s)
  QString value;
  for (int i = 0; i < propertyCount(); i++) {
    PropertyNode* p = property(i);
    isin = p->isIn(pos);
    if (isin->isPropertyType()) {
      switch (isin->type) {
        case SectionType::PropertyName:
          isin->type = SectionType::WidgetPropertyName;
          break;
        case SectionType::PropertyMarker:
          isin->type = SectionType::WidgetPropertyMarker;
          break;
        case SectionType::PropertyValue:
          isin->type = SectionType::WidgetPropertyValue;
          break;
        case SectionType::PropertyEndMarker:
          isin->type = SectionType::WidgetPropertyEndMarker;
          break;
      }
      isin->propertyIndex = i;
      return isin;
    }
  }

  if (hasEndBrace()) {
    cursor = endBraceCursor();
    rect = node_ptr->editor->cursorRect(cursor);
    top = rect.y();
    bottom = top + rect.height();
    left = rect.x();
    right = left + fm.horizontalAdvance("}");
    if (x >= left && x <= right && y >= top && y < bottom) {
      isin->type = SectionType::WidgetEndBrace;
      return isin;
    }
  }

  return isin;
}

PropertyStatus
WidgetNode::sectionAtOffset(int pos) const
{
  if (pos >= node_ptr->cursor.anchor() &&
      pos < node_ptr->cursor.anchor() + node_ptr->name.length()) {
    return PropertyStatus(SectionType::WidgetPropertyName, node_ptr->name);

  } else {
    //    for (int i = 0; i < p_ptr->values.size(); i++) {
    //      auto value = p_ptr->values.at(i);
    //      auto offset = p_ptr->cursors.at(i).anchor();

    //      if (pos >= offset && pos < offset + value.length()) {
    //        return PropertyStatus(SectionType::WidgetPropertyValue, value,
    //        offset);
    //      }
    //    }
  }

  return PropertyStatus();
}

void
WidgetNode::setSubControlMarkerCursor(QTextCursor cursor)
{
  widget_ptr->markerPosition = cursor;
  widget_ptr->markerType = NodeType::SubControlMarkerType;
}

void
WidgetNode::setPseudoStateMarkerCursor(QTextCursor cursor)
{
  widget_ptr->markerPosition = cursor;
  widget_ptr->markerType = NodeType::PseudoStateMarkerType;
}

bool
WidgetNode::doesMarkerMatch(enum NodeType type) const
{
  if ((type == NodeType::PseudoStateType &&
       widget_ptr->markerType == NodeType::PseudoStateMarkerType) ||
      (type == NodeType::SubControlType &&
       widget_ptr->markerType == NodeType::SubControlMarkerType)) {
    return true;
  }
  return false;
}

QTextCursor
WidgetNode::extensionCursor() const
{
  return widget_ptr->extensionPosition;
}

int
WidgetNode::extensionPosition() const
{
  return widget_ptr->extensionPosition.anchor();
}

void
WidgetNode::setExtensionCursor(QTextCursor cursor)
{
  widget_ptr->extensionPosition = cursor;
}

QTextCursor
WidgetNode::markerCursor() const
{
  return widget_ptr->markerPosition;
}

int
WidgetNode::markerPosition() const
{
  return widget_ptr->markerPosition.anchor();
}

void
WidgetNode::setMarkerCursor(QTextCursor cursor)
{
  widget_ptr->markerPosition = cursor;
}

bool
WidgetNode::hasMarker()
{
  return (widget_ptr->markerPosition.isNull());
}

QString
WidgetNode::extensionName() const
{
  return widget_ptr->extensionName;
}

void
WidgetNode::setExtensionName(const QString& name)
{
  widget_ptr->extensionName = name;
}

bool
WidgetNode::isExtensionValid() const
{
  if (widget_ptr->extensionType == NodeType::SubControlType &&
      doesMarkerMatch(NodeType::SubControlType)) {
    return true;
  }
  if (widget_ptr->extensionType == NodeType::PseudoStateType &&
      doesMarkerMatch(NodeType::PseudoStateType)) {
    return true;
  }
  return false;
}

bool
WidgetNode::isSubControl() const
{
  return (widget_ptr->extensionType == NodeType::SubControlType ||
          widget_ptr->extensionType == NodeType::FuzzySubControlType);
}

bool
WidgetNode::isPseudoState() const
{
  return (widget_ptr->extensionType == NodeType::PseudoStateType ||
          widget_ptr->extensionType == NodeType::FuzzyPseudoStateType);
}

void
WidgetNode::setExtensionType(enum NodeType type)
{
  switch (type) {
    case NodeType::SubControlType:
    case NodeType::PseudoStateType:
    case NodeType::FuzzySubControlType:
    case NodeType::FuzzyPseudoStateType:
      widget_ptr->extensionType = type;
  }
}

bool
WidgetNode::hasExtension() const
{
  return (widget_ptr->extensionType == NodeType::SubControlType ||
          widget_ptr->extensionType == NodeType::PseudoStateType ||
          widget_ptr->extensionType == NodeType::FuzzySubControlType ||
          widget_ptr->extensionType == NodeType::FuzzyPseudoStateType);
}

int
WidgetNode::extensionLength() const
{
  return widget_ptr->extensionName.length();
}

bool
WidgetNode::hasStartBrace() const
{
  return (widget_ptr->startBrace);
}

void
WidgetNode::setStartBraceCursor(QTextCursor cursor)
{
  widget_ptr->startBrace = true;
  widget_ptr->startBracePosition = cursor;
}

QTextCursor
WidgetNode::startBraceCursor() const
{
  return widget_ptr->startBracePosition;
}

int
WidgetNode::startBracePosition() const
{
  return widget_ptr->startBracePosition.anchor();
}

void
WidgetNode::removeStartBrace()
{
  widget_ptr->startBrace = false;
}

bool
WidgetNode::hasEndBrace() const
{
  return (widget_ptr->endBrace);
}

void
WidgetNode::setEndBraceCursor(QTextCursor cursor)
{
  widget_ptr->endBrace = true;
  widget_ptr->endBracePosition = cursor;
}

QTextCursor
WidgetNode::endBraceCursor() const
{
  return widget_ptr->endBracePosition;
}

int
WidgetNode::endBracePosition() const
{
  return widget_ptr->endBracePosition.anchor();
}

void
WidgetNode::removeEndBrace()
{
  widget_ptr->endBrace = false;
  //  w_ptr->endBraceOffset = 0;
}

bool
WidgetNode::offsetsMatch() const
{
  return (widget_ptr->startBrace && widget_ptr->endBrace);
}

bool
WidgetNode::isExtensionMarkerCorrect()
{
  return widget_ptr->extensionMarkerCorrect;
}

void
WidgetNode::setExtensionMarkerCorrect(bool correct)
{
  widget_ptr->extensionMarkerCorrect = correct;
}

void
WidgetNode::addProperty(PropertyNode* property)
{
  widget_ptr->properties.append(property);
}

PropertyNode*
WidgetNode::property(int index) const
{
  return widget_ptr->properties.at(index);
}

int
WidgetNode::propertyCount() const
{
  return widget_ptr->properties.size();
}

bool
WidgetNode::isFinalProperty(PropertyNode* property) const
{
  auto index = widget_ptr->properties.indexOf(property);
  return (index == widget_ptr->properties.size() - 1);
}

PropertyNode::PropertyNode(const QString& name,
                           QTextCursor start,
                           StylesheetEditor* editor,
                           QObject* parent,
                           enum NodeType type)
  : Node(name, start, editor, parent, type)
{
  property_ptr = new PropertyNodeData;
}

PropertyNode::PropertyNode(const PropertyNode& other)
  : Node(other.name(),
         other.cursor(),
         other.node_ptr->editor,
         other.parent(),
         other.type())
{
  property_ptr = new PropertyNodeData(*other.property_ptr);
}

PropertyNode::~PropertyNode()
{
  delete property_ptr;
}

void
PropertyNode::setWidget(WidgetNode* widget)
{
  property_ptr->widget = widget;
}

bool
PropertyNode::hasWidget()
{
  return (property_ptr->widget);
}

QStringList
PropertyNode::values() const
{
  return property_ptr->values;
}

QList<PropertyValueCheck>
PropertyNode::checks() const
{
  return property_ptr->checks;
}

QList<QTextCursor>
PropertyNode::valueCursors() const
{
  return property_ptr->cursors;
}

void
PropertyNode::setValueCursor(int index, QTextCursor cursor)
{
  if (index >= 0 && index < property_ptr->cursors.length()) {
    property_ptr->cursors.replace(index, cursor);
  }
}

QList<AttributeType>
PropertyNode::attributeTypes() const
{
  return property_ptr->attributeTypes;
}

void
PropertyNode::setValues(const QStringList& values)
{
  property_ptr->values = values;
}

void
PropertyNode::setValue(int index, const QString& value)
{
  if (index >= 0 && index < property_ptr->values.length()) {
    property_ptr->values.replace(index, value);
  }
}

void
PropertyNode::setChecks(const QList<PropertyValueCheck>& checks)
{
  property_ptr->checks = checks;
}

void
PropertyNode::setCheck(int index, PropertyValueCheck check)
{
  if (index >= 0 && index < property_ptr->checks.length()) {
    property_ptr->checks.replace(index, check);
  }
}

PropertyValueCheck
PropertyNode::check(int index)
{
  if (index >= 0 && index < property_ptr->values.size())
    return property_ptr->checks.at(index);
  return PropertyValueCheck::NoCheck;
}

void
PropertyNode::setValueCursors(const QList<QTextCursor>& positions)
{
  property_ptr->cursors = positions;
}

int
PropertyNode::valuePosition(int index)
{
  if (index >= 0 && index < property_ptr->cursors.size())
    return property_ptr->cursors.at(index).anchor();
  return -1;
}

void
PropertyNode::addValue(const QString& value,
                       PropertyValueCheck check,
                       QTextCursor position,
                       AttributeType attType = NoAttributeValue)
{
  property_ptr->values.append(value);
  property_ptr->checks.append(check);
  property_ptr->cursors.append(position);
  property_ptr->attributeTypes.append(attType);
}

QString
PropertyNode::value(int index)
{
  if (index >= 0 && index < property_ptr->values.size())
    return property_ptr->values.at(index);
  return QString();
}

bool
PropertyNode::setBadCheck(PropertyValueCheck check, int index)
{
  if (index == -1 && !property_ptr->checks.isEmpty()) {
    property_ptr->checks[property_ptr->checks.length() - 1] = check;
    return true;

  } else if (index >= 0 && index < property_ptr->checks.length()) {
    property_ptr->checks[index] = check;
    return true;
  }

  return false;
}

int
PropertyNode::count()
{
  return property_ptr->cursors.size();
}

bool
PropertyNode::isValueValid(int index)
{
  if (index >= 0 && index < count()) {
    return property_ptr->checks.at(index) != PropertyValueCheck::BadValue;
  }

  // default to bad.
  return false;
}

int
PropertyNode::end() const
{
  if (property_ptr->cursors.isEmpty() || property_ptr->values.isEmpty()) {
    return start();
  }

  return start() + length();
}

int
PropertyNode::length() const
{
  if (property_ptr->cursors.isEmpty()) {
    if (!hasPropertyMarker()) {
      return node_ptr->name.length();
    } else {
      return propertyMarkerCursor().anchor() + 1;
    }
  } else {
    return property_ptr->cursors.last().anchor() - node_ptr->cursor.anchor() +
           property_ptr->values.last().length();
  }

  return 0;
}

bool
PropertyNode::hasPropertyMarker() const
{
  return property_ptr->propertyState.testFlag(
    PropertyCheck::PropertyMarkerCheck);
}

void
PropertyNode::setPropertyMarker(bool exists)
{
  property_ptr->propertyState.setFlag(PropertyCheck::PropertyMarkerCheck,
                                      exists);
}

bool
PropertyNode::isValidPropertyName() const
{
  return property_ptr->propertyState.testFlag(PropertyCheck::ValidNameCheck);
}

void
PropertyNode::setValidPropertyName(bool valid)
{
  property_ptr->propertyState.setFlag(PropertyCheck::ValidNameCheck, valid);
}

bool
PropertyNode::isValidProperty(bool finalProperty)
{
  if (property_ptr->widget) {
    if (property_ptr->widget->isFinalProperty(this)) {
      return isValidPropertyName() && hasPropertyMarker();
    }
  }

  if (!hasPropertyEndMarker() && finalProperty) {
    return isValidPropertyName() && hasPropertyMarker();
  }

  return property_ptr->propertyState.testFlag(PropertyCheck::GoodPropertyCheck);
}

QTextCursor
PropertyNode::propertyMarkerCursor() const
{
  return property_ptr->propertyMarkerCursor;
}

int
PropertyNode::propertyMarkerPosition() const
{
  return property_ptr->propertyMarkerCursor.anchor();
}

void
PropertyNode::setPropertyMarkerCursor(QTextCursor position)
{
  property_ptr->propertyMarkerCursor = position;
}

bool
PropertyNode::hasPropertyEndMarker() const
{
  return property_ptr->propertyState.testFlag(
    PropertyCheck::PropertyEndMarkerCheck);
}

void
PropertyNode::setPropertyEndMarker(bool exists)
{
  property_ptr->propertyState.setFlag(PropertyCheck::PropertyEndMarkerCheck,
                                      exists);
}

QTextCursor
PropertyNode::propertyEndMarkerCursor() const
{
  return property_ptr->endMarkerCursor;
}

int
PropertyNode::propertyEndMarkerPosition() const
{
  return property_ptr->endMarkerCursor.anchor();
}

void
PropertyNode::setPropertyEndMarkerCursor(QTextCursor position)
{
  property_ptr->endMarkerCursor = position;
}

NodeSection*
PropertyNode::isIn(QPoint pos)
{
  // this just checks the property name.
  auto isin = Node::isIn(pos);
  if (isin->type == SectionType::Name) {
    isin->type = SectionType::PropertyName;
    return isin;
  }

  QRect rect;
  auto x = pos.x();
  auto y = pos.y();
  int left, right;
  auto fm = node_ptr->editor->fontMetrics();
  int top, bottom;
  QTextCursor cursor;

  // check marker;
  if (hasPropertyMarker()) {
    cursor = propertyMarkerCursor();
    rect = node_ptr->editor->cursorRect(cursor);
    top = rect.y();
    bottom = top + rect.height();
    left = rect.x();
    right = left + fm.horizontalAdvance(":");
    if (x >= left && x <= right && y >= top && y < bottom) {
      isin->type = SectionType::PropertyMarker;
      return isin;
    }
  }

  // then value(s)
  QString value;
  for (int i = 0; i < valueCursors().count(); i++) {
    value = values().at(i);
    cursor = valueCursors().at(i);
    rect = node_ptr->editor->cursorRect(cursor);
    top = rect.y();
    bottom = top + rect.height();
    left = rect.x();
    right = left + fm.horizontalAdvance(value);

    if (x >= left && x <= right && y >= top && y < bottom) {
      isin->type = SectionType::PropertyValue;
      isin->position = i;
      break;
    }
  }

  if (hasPropertyEndMarker()) {
    cursor = propertyEndMarkerCursor();
    rect = node_ptr->editor->cursorRect(cursor);
    top = rect.y();
    bottom = top + rect.height();
    left = rect.x();
    right = left + fm.horizontalAdvance(":");
    if (x >= left && x <= right && y >= top && y < bottom) {
      isin->type = SectionType::PropertyEndMarker;
      return isin;
    }
  }

  return isin;
}

PropertyStatus
PropertyNode::sectionAtOffset(int pos, QString textChanged) const
{
  if (pos >= node_ptr->cursor.anchor() &&
      pos < node_ptr->cursor.anchor() + node_ptr->name.length()) {
    return PropertyStatus(SectionType::PropertyName, node_ptr->name);
  }

  if (hasPropertyMarker()) {
    auto markerPos = propertyMarkerPosition();
    if (pos >= markerPos && pos < markerPos + 1) {
      return PropertyStatus(SectionType::PropertyMarker);
    }
  }

  if (!values().isEmpty()) {
    if (pos < valueCursors().constFirst().anchor()) {
      return PropertyStatus(SectionType::PropertyMarker);
    }
    for (int i = 0; i < property_ptr->values.size(); i++) {
      auto value = property_ptr->values.at(i);
      auto offset = property_ptr->cursors.at(i).anchor();

      if (pos >= offset && pos <= offset + value.length()) {
        if (textChanged.trimmed() == ";") {
          return PropertyStatus(SectionType::PropertyEndMarker);
        } else {
          return PropertyStatus(SectionType::PropertyValue, value, offset);
        }
      }
    }
  }

  if (!hasPropertyEndMarker()) {
    if (!values().isEmpty()) {
      return PropertyStatus(SectionType::PropertyEndMarker);
    }
  } else {
    auto markerPos = propertyEndMarkerPosition();
    if (pos >= markerPos && pos < markerPos + 1) {
      return PropertyStatus(SectionType::PropertyEndMarker);
    }
  }

  return PropertyStatus();
}

void
PropertyNode::setAttributeTypes(const QList<AttributeType>& attributeTypes)
{
  property_ptr->attributeTypes = attributeTypes;
}

CommentNode::CommentNode(QTextCursor start,
                         StylesheetEditor* editor,
                         QObject* parent,
                         enum NodeType type)
  : WidgetNode(QString(), start, editor, parent, type)
{
  c_ptr = new CommentNodeData;
}

CommentNode::CommentNode(const CommentNode& other)
  : WidgetNode(other.name(),
               other.cursor(),
               other.node_ptr->editor,
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
  node_ptr->name.append(c);
}

void
CommentNode::append(QString text)
{
  node_ptr->name.append(text);
}

int
CommentNode::end() const
{
  return node_ptr->cursor.anchor() + node_ptr->name.length();
}

int
CommentNode::length() const
{
  if (hasEndComment()) {
    return endCommentCursor().anchor() - start() + 2;
  } else if (node_ptr->name.length() > 0) {
    return c_ptr->cursor.anchor() + node_ptr->name.length() - start();
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

NodeSection*
CommentNode::isIn(QPoint pos)
{
  QRect rect;
  auto x = pos.x();
  auto y = pos.y();
  int left, right;
  auto fm = node_ptr->editor->fontMetrics();
  int top, bottom;
  NodeSection* isin = new NodeSection();

  // check marker;
  QTextCursor s(node_ptr->editor->document());
  s.setPosition(start());
  rect = node_ptr->editor->cursorRect(s);
  top = rect.y();
  bottom = top + rect.height();
  left = rect.x();
  s.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, length());
  rect = node_ptr->editor->cursorRect(s);
  right = rect.x();
  if (x >= left && x <= right && y >= top && y < bottom) {
    isin->type = SectionType::Comment;
    return isin;
  }

  return isin;
}

NewlineNode::NewlineNode(QTextCursor start,
                         StylesheetEditor* editor,
                         QObject* parent,
                         enum NodeType type)
  : WidgetNode("\n", start, editor, parent, type)
{}

StartBraceNode::StartBraceNode(QTextCursor start,
                               StylesheetEditor* editor,
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
                           StylesheetEditor* editor,
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
                                         StylesheetEditor* editor,
                                         QObject* parent,
                                         enum NodeType type)
  : WidgetNode("/*", start, editor, parent, type)
  , BadNode(errors)
{}

int
BadStartCommentNode::end() const
{
  return node_ptr->cursor.anchor() + 2;
}

BadEndCommentNode::BadEndCommentNode(QTextCursor start,
                                     ParserState::Errors errors,
                                     StylesheetEditor* editor,
                                     QObject* parent,
                                     enum NodeType type)
  : WidgetNode("*/", start, editor, parent, type)
  , BadNode(errors)
{}

int
BadEndCommentNode::end() const
{
  return node_ptr->cursor.anchor() + 2;
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
