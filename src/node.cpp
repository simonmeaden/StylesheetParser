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


Node::Node(QTextCursor start, StylesheetEdit* parent, Type type)
  : QObject(parent)
  , previous(nullptr)
  , next(nullptr)
  , m_cursor(start)
  , m_type(type)
  , m_parent(parent)
{}

int Node::start() const
{
  return m_cursor.anchor();
}

void Node::setStart(int position)
{
  m_cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
  m_cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, position);
}

void Node::moveStart(int count)
{
  QTextCursor::MoveOperation moveOp =
    (count < 0 ? QTextCursor::Left : QTextCursor::Right);
  m_cursor.movePosition(moveOp, QTextCursor::MoveAnchor, qAbs(count));
}

int Node::end() const
{
  return m_cursor.anchor();
}

QTextCursor Node::cursor()
{
  return m_cursor;
}

Node::Type Node::type() const
{
  return m_type;
}

QString Node::toString()
{
  switch (m_type) {
  case NodeType:
    return "Node";

  case BaseNodeType:
    return "Base Node";

  case CharNodeType:
    return "Char Node";

  case ColonNodeType:
    return "Colon Node";

  case NameType:
    return "Name Node";

  case WidgetType:
    return "Widget Node";

  case SubControlType:
    return "Sub Control Node";

  case SubControlMarkerType:
    return "Sub Control Marker";

  case PseudoStateType:
    return "PseudoState Node";

  case PseudoStateMarkerType:
    return "PseudoState Marker";

  case SemiColonType:
    return "SemiColon Node";

  case StartBraceType:
    return "Start Brace Node";

  case EndBraceType:
    return "End Brace Node";

  case NewlineType:
    return "Newline Node";

  case PropertyType:
    return "Property Node";

  //  case PropertyMarkerType:
  //    return "Property End Marker";

  case PropertyEndType:
    return "Property End";

  case BadNodeType:
    return "Bad Node";
  }

  return QString();
}

NameNode::NameNode(const QString& name)
  : m_name(name)
{}

QString NameNode::name() const
{
  return m_name;
}

void NameNode::setName(const QString& value)
{
  m_name = value;
}

int NameNode::length() const
{
  return m_name.length();
}

NamedNode::NamedNode(const QString& name, QTextCursor start, StylesheetEdit* parent, Node::Type type)
  : Node(start, parent, type)
  , NameNode(name)
{}

QPair<NameNode::SectionType, int> NamedNode::isIn(QPoint pos)
{
  auto x = pos.x();
  auto y = pos.y();
  auto rect = m_parent->cursorRect(QTextCursor(m_cursor));
  auto left = rect.x();
  auto fm = m_parent->fontMetrics();
  auto width = fm.horizontalAdvance(m_name);
  auto height = fm.height();
  auto right = left + width;
  auto top = rect.y();
  auto bottom = top + height;

  if (x >= left && x <= right && y >= top && y <= bottom) {
    return qMakePair<SectionType, int>(NameNode::Name, 0);
  }

  return qMakePair<SectionType, int>(NameNode::None, -1);
}


BadBlockNode::BadBlockNode(const QString& name,
                           QTextCursor start,
                           ParserState::Errors errors,
                           StylesheetEdit* parent,
                           Type type)
  : NamedNode(name, start, parent, type)
  , BadNode(errors)
{}

BadNode::BadNode(ParserState::Errors errors)
  : m_errors(errors)
{}

int BadBlockNode::end() const
{
  return m_cursor.anchor() + name().length();
}

ParserState::Errors BadNode::errors() const
{
  return m_errors;
}

void BadNode::setError(const ParserState::Errors& errors)
{
  m_errors = errors;
}

BadSubControlMarkerNode::BadSubControlMarkerNode(QTextCursor start,
    ParserState::Errors errors,
    StylesheetEdit* parent,
    Node::Type type)
  : SubControlMarkerNode(start, parent, type)
  , BadNode(errors)
{

}

BadPseudoStateMarkerNode::BadPseudoStateMarkerNode(QTextCursor start,
    ParserState::Errors errors, StylesheetEdit* parent,
    Node::Type type)
  : PseudoStateMarkerNode(start, parent, type)
  , BadNode(errors)
{

}

WidgetNode::WidgetNode(const QString& name,
                       QTextCursor start,
                       StylesheetEdit* parent,
                       Type type)
  : NamedNode(name, start, parent, type)
  , m_widgetValid(true)
{}

bool WidgetNode::isWidgetValid() const
{
  return m_widgetValid;
}

void WidgetNode::setWidgetValid(bool widgetValid)
{
  m_widgetValid = widgetValid;
}

PropertyNode::PropertyNode(const QString& name,
                           QTextCursor start,
                           StylesheetEdit* parent,
                           Node::Type type)
  : NamedNode(name, start, parent, type)
  , m_propertyMarkerExists(false)
  , m_validProperty(true)
{}

QStringList PropertyNode::values() const
{
  return m_values;
}

QList<PropertyNode::Check> PropertyNode::checks() const
{
  return m_checks;
}

QList<int> PropertyNode::offsets() const
{
  return m_offsets;
}

QList<DataStore::AttributeType> PropertyNode::attributeTypes() const
{
  return m_attributeTypes;
}

void PropertyNode::setValues(const QStringList& values)
{
  m_values = values;
}

void PropertyNode::setChecks(const QList<PropertyNode::Check>& checks)
{
  m_checks = checks;
}

void PropertyNode::setOffsets(const QList<int>& offsets)
{
  m_offsets = offsets;
}

void PropertyNode::addValue(const QString& value, PropertyNode::Check check, int offset,
                            DataStore::AttributeType attType = DataStore::NoAttributeValue)
{
  m_values.append(value);
  m_checks.append(check);
  m_offsets.append(offset);
  m_attributeTypes.append(attType);
}

bool PropertyNode::setBadCheck(Check check, int index)
{
  if (index == -1 && !m_checks.isEmpty()) {
    m_checks[m_checks.length() - 1] = check;
    return true;

  } else if (index >= 0 && index < m_checks.length()) {
    m_checks[index] = check;
    return true;
  }

  return false;
}

int PropertyNode::count()
{
  return m_offsets.size();
}

PropertyNode::Check PropertyNode::isValueValid(int index)
{
  if (index > 00 && index < count()) {
    return m_checks.at(index);
  }

  // default to bad.
  return Check::BadValue;
}

int PropertyNode::end() const
{
  if (m_offsets.isEmpty() || m_values.isEmpty()) {
    return start();
  }

  return start() + m_offsets.back() + m_values.last().length();
}

bool PropertyNode::hasPropertyMarker() const
{
  return m_propertyMarkerExists;
}

void PropertyNode::setPropertyMarkerExists(bool propertyMarker)
{
  m_propertyMarkerExists = propertyMarker;
}

bool PropertyNode::isValidProperty() const
{
  return m_validProperty;
}

void PropertyNode::setValidProperty(bool validProperty)
{
  m_validProperty = validProperty;
}

int PropertyNode::propertyMarkerOffset() const
{
  return m_propertymarkerOffset;
}

void PropertyNode::setPropertyMarkerOffset(int propertymarkerOffset)
{
  m_propertymarkerOffset = propertymarkerOffset;
}

QPair<NameNode::SectionType, int> PropertyNode::isIn(QPoint pos)
{
  auto x = pos.x();
  auto y = pos.y();
  auto cursor = QTextCursor(m_cursor);
  auto rect = m_parent->cursorRect(cursor);
  auto left = rect.x();
  auto fm = m_parent->fontMetrics();
  auto width = fm.horizontalAdvance(m_name); // initially just property name
  auto height = fm.height();
  auto right = left + width;
  auto top = rect.y();
  auto bottom = top + height;
  auto propLeft = left;
  QTextCursor propCursor(cursor);
  propCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, offsets().last());
  rect = m_parent->cursorRect(propCursor);
  auto propRight = rect.x() + fm.horizontalAdvance(values().last());

  // is it inside the entire property boundaries.
  if (y >= top && y <= bottom && x >= propLeft && x <= propRight) {
    // Property name.
    if (x >= left && x <= right) {
      return qMakePair<SectionType, int>(NameNode::Name, 0);
    }

    // Property values.
    for (int i = 0; i < offsets().count(); i++) {
      auto value = values().at(i);
      auto offset = offsets().at(i);
      auto valCursor(cursor);
      auto oldRight = right;
      valCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, offset);
      rect = m_parent->cursorRect(valCursor);
      width = fm.horizontalAdvance(value);
      left = rect.x();
      right = left + width + 1;

      if (x >= oldRight && x <= left) {
        // between values or name and first value.
        return qMakePair<SectionType, int>(NamedNode::None, -1);
      }

      if (x >= left && x <= right) {
        // inside a value.
        return qMakePair<SectionType, int>(NameNode::Value, i);
      }
    }
  } // else return not inside.

  return qMakePair<SectionType, int>(NamedNode::None, -1);
}

void PropertyNode::setAttributeTypes(const QList<DataStore::AttributeType>& attributeTypes)
{
  m_attributeTypes = attributeTypes;
}

SubControlNode::SubControlNode(const QString& name,
                               QTextCursor start,
                               StylesheetEdit* parent,
                               Type type)
  : NamedNode(name, start, parent, type)
{}

int SubControlNode::end() const
{
  return m_cursor.anchor() + m_name.length();
}

PseudoStateNode::PseudoStateNode(const QString& name,
                                 QTextCursor start,
                                 StylesheetEdit* parent,
                                 Type type)
  : NamedNode(name, start, parent, type)
{}

int PseudoStateNode::end() const
{
  return m_cursor.anchor() + m_name.length();
}

CommentNode::CommentNode(QTextCursor start, StylesheetEdit* parent, Node::Type type)
  : NamedNode(QString(), start, parent, type)
{}

void CommentNode::append(QChar c)
{
  m_name.append(c);
}

void CommentNode::append(QString text)
{
  m_name.append(text);
}

int CommentNode::end() const
{
  return m_cursor.anchor() + m_name.length();
}

QPair<NameNode::SectionType, int> CommentNode::isIn(QPoint pos)
{
  // TODO isIn for comments.
  return qMakePair<SectionType, int>(NamedNode::None, -1);
}

ColonNode::ColonNode(QTextCursor start, StylesheetEdit* parent, Type type)
  : NamedNode(":", start, parent, type)
{}

SubControlMarkerNode::SubControlMarkerNode(QTextCursor start,
    StylesheetEdit* parent,
    Type type)
  : NamedNode("::", start, parent, type)
{}

int SubControlMarkerNode::end() const
{
  return m_cursor.anchor() + 2;
}

SemiColonNode::SemiColonNode(QTextCursor start, StylesheetEdit* parent, Type type)
  : NamedNode(";", start, parent, type)
{}

NewlineNode::NewlineNode(QTextCursor start, StylesheetEdit* parent, Type type)
  : NamedNode("\n", start, parent, type)
{}

StartBraceNode::StartBraceNode(QTextCursor start, StylesheetEdit* parent, Type type)
  : NamedNode("{", start, parent, type)
  , m_isBraceAtCursor(false)
{}

bool StartBraceNode::isBraceAtCursor() const
{
  return m_isBraceAtCursor;
}

void StartBraceNode::setBraceAtCursor(bool isFlagBrace)
{
  m_isBraceAtCursor = isFlagBrace;
}

EndBraceNode::EndBraceNode(QTextCursor start, StylesheetEdit* parent, Type type)
  : NamedNode("}", start, parent, type)
  , m_isBraceAtCursor(false)
{}

bool EndBraceNode::isBraceAtCursor() const
{
  return m_isBraceAtCursor;
}

void EndBraceNode::setBraceAtCursor(bool isFlagBrace)
{
  m_isBraceAtCursor = isFlagBrace;
}

StartCommentNode::StartCommentNode(QTextCursor start, StylesheetEdit* parent, Node::Type type)
  : NamedNode("/*", start, parent, type)
{}

int StartCommentNode::end() const
{
  return m_cursor.anchor() + 2;
}

EndCommentNode::EndCommentNode(QTextCursor start, StylesheetEdit* parent, Node::Type type)
  : NamedNode("*/", start, parent, type)
{}

int EndCommentNode::end() const
{
  return m_cursor.anchor() + 2;
}

PseudoStateMarkerNode::PseudoStateMarkerNode(QTextCursor start,
    StylesheetEdit* parent,
    Type type)
  : ColonNode(start, parent, type)
{}

PropertyEndMarkerNode::PropertyEndMarkerNode(QTextCursor start,
    StylesheetEdit* parent,
    Node::Type type)
  : SemiColonNode(start, parent, type)
{}

PropertyEndNode::PropertyEndNode(QTextCursor start,
                                 StylesheetEdit* parent,
                                 Node::Type type)
  : Node(start, parent, type)
{}


