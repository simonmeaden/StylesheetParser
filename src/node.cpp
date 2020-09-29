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

namespace StylesheetEditor {

Node::Node(QTextCursor start, QObject* parent, Type type)
  : QObject(parent)
  , previous(nullptr)
  , next(nullptr)
  , m_start(start)
  , m_type(type)
{}

int Node::start() const
{
  return m_start.anchor();
}

void Node::setStart(int position)
{
  m_start.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
  m_start.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, position);
}

void Node::moveStart(int count)
{
  QTextCursor::MoveOperation moveOp =
    (count < 0 ? QTextCursor::Left : QTextCursor::Right);
  m_start.movePosition(moveOp, QTextCursor::MoveAnchor, qAbs(count));
}

int Node::end() const
{
  return m_start.anchor();
}

int Node::length() const
{
  return 0;
}

Node::Type Node::type() const
{
  return m_type;
}

// Node *Node::previous() {
//  return m_previous;
//}

// Node *Node::next() {
//  return m_next;
//}

// void Node::setNext(Node *next)
//{
//  m_next = next;
//}

// void Node::setPrevious(Node *previous)
//{
//  m_previous = previous;
//}

BaseNode::BaseNode(const QString& value,
                   QTextCursor start,
                   QObject* parent,
                   Type type)
  : Node(start, parent, type)
  , m_value(value)
{}

QString BaseNode::value() const
{
  return m_value;
}

int BaseNode::end() const
{
  return m_start.anchor() + m_value.length();
}

int BaseNode::length() const
{
  return m_value.length();
}

NameNode::NameNode(const QString& name,
                   QTextCursor start,
                   QObject* parent,
                   Type type)
  : Node(start, parent, type)
  , m_name(name)
{}

QString NameNode::value() const
{
  return m_name;
}

int NameNode::end() const
{
  return m_start.anchor() + m_name.length();
}

int NameNode::length() const
{
  return m_name.length();
}

BadBlockNode::BadBlockNode(const QString& name,
                           QTextCursor start,
                           ParserState::Errors errors,
                           QObject* parent,
                           Type type)
  : Node(start, parent, type)
  , m_name(name)
  , m_errors(errors)
{}

QString BadBlockNode::value() const
{
  return m_name;
}

int BadBlockNode::end() const
{
  return m_start.anchor() + m_name.length();
}

int BadBlockNode::length() const
{
  return m_name.length();
}

ParserState::Errors BadBlockNode::errors() const
{
  return m_errors;
}

void BadBlockNode::setError(const ParserState::Errors& errors)
{
  m_errors = errors;
}

// ValueNode::ValueNode(const QStringList& values, QList<bool> checks,
// QList<int> offsets, QTextCursor start,
//                     QObject* parent,
//                     Type type)
//  : Node(start, parent, type)
//  , m_values(values)
//  , m_checks(checks)
//  , m_offsets(offsets)
//{}

// QStringList ValueNode::values() const
//{
//  return m_values;
//}

// int ValueNode::count()
//{
//  return m_values.size();
//}

// bool ValueNode::isValid(int index)
//{
//  return m_checks.at(index);
//}

// void ValueNode::setValues(const QStringList& values)
//{
//  m_values = values;
//}

// void ValueNode::setChecks(const QList<bool>& checks)
//{
//  m_checks = checks;
//}

// void ValueNode::setOffsets(const QList<int>& offsets)
//{
//  m_offsets = offsets;
//}

// QList<int> ValueNode::offsets() const
//{
//  return m_offsets;
//}

// QList<bool> ValueNode::checks() const
//{
//  return m_checks;
//}

WidgetNode::WidgetNode(const QString& name,
                       QTextCursor start,
                       QObject* parent,
                       Type type)
  : BaseNode(name, start, parent, type)
{}

PropertyNode::PropertyNode(const QString& name,
                           QTextCursor start,
                           QObject* parent,
                           Node::Type type)
  : BaseNode(name, start, parent, type)
  , m_propertyMarkerExists(false)
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

void PropertyNode::addValue(const QString& value, PropertyNode::Check check, int offset)
{
  m_values.append(value);
  m_checks.append(check);
  m_offsets.append(offset);
}

bool PropertyNode::setBadCheck(Check check, int index)
{
  if (index == -1) {
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

PropertyNode::Check PropertyNode::isValid(int index)
{
  if (index > 00 && index < count()) {
    return m_checks.at(index);
  }

  // default to bad.
  return Check::BadValue;
}

int PropertyNode::end() const
{
  return start() + m_offsets.back() + m_values.last().length();
}

bool PropertyNode::propertyMarkerExists() const
{
  return m_propertyMarkerExists;
}

void PropertyNode::setPropertyMarkerExists(bool propertyMarker)
{
  m_propertyMarkerExists = propertyMarker;
}

SubControlNode::SubControlNode(const QString& name,
                               QTextCursor start,
                               QObject* parent,
                               Type type)
  : NameNode(name, start, parent, type)
{}

PseudoStateNode::PseudoStateNode(const QString& name,
                                 QTextCursor start,
                                 QObject* parent,
                                 Type type)
  : NameNode(name, start, parent, type)
{}

CommentNode::CommentNode(QTextCursor start, QObject* parent, Node::Type type)
  : BaseNode(QString(), start, parent, type)
{}

void CommentNode::append(QChar c)
{
  m_value.append(c);
}

void CommentNode::append(QString text)
{
  m_value.append(text);
}

int CommentNode::end() const
{
  return m_start.anchor() + m_value.length();
}

int CommentNode::length() const
{
  return m_value.length();
}

CharNode::CharNode(QTextCursor start, QObject* parent, Type type)
  : Node(start, parent, type)
{}

int CharNode::end() const
{
  return m_start.anchor() + 1;
}

int CharNode::length() const
{
  return 1;
}

ColonNode::ColonNode(QTextCursor start, QObject* parent, Type type)
  : CharNode(start, parent, type)
{}

SubControlMarkerNode::SubControlMarkerNode(QTextCursor start,
    QObject* parent,
    Type type)
  : CharNode(start, parent, type)
{}

int SubControlMarkerNode::end() const
{
  return m_start.anchor() + 2;
}

int SubControlMarkerNode::length() const
{
  return 2;
}

SemiColonNode::SemiColonNode(QTextCursor start, QObject* parent, Type type)
  : CharNode(start, parent, type)
{}

NewlineNode::NewlineNode(QTextCursor start, QObject* parent, Type type)
  : CharNode(start, parent, type)
{}

StartBraceNode::StartBraceNode(QTextCursor start, QObject* parent, Type type)
  : CharNode(start, parent, type)
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

EndBraceNode::EndBraceNode(QTextCursor start, QObject* parent, Type type)
  : CharNode(start, parent, type)
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

StartCommentNode::StartCommentNode(QTextCursor start, QObject *parent, Node::Type type)
  : CharNode(start, parent, type)
{}

int StartCommentNode::end() const
{
  return m_start.anchor() + 2;
}

int StartCommentNode::length() const
{
  return 2;
}

EndCommentNode::EndCommentNode(QTextCursor start, QObject* parent, Node::Type type)
  : CharNode(start, parent, type)
{}

int EndCommentNode::end() const
{
  return m_start.anchor() + 2;
}

int EndCommentNode::length() const
{
  return 2;
}

PseudoStateMarkerNode::PseudoStateMarkerNode(QTextCursor start,
    QObject* parent,
    Type type)
  : ColonNode(start, parent, type)
{}

PropertyMarkerNode::PropertyMarkerNode(QTextCursor start,
                                       QObject* parent,
                                       Node::Type type)
  : ColonNode(start, parent, type)
{}

PropertyEndMarkerNode::PropertyEndMarkerNode(QTextCursor start,
    QObject* parent,
    Node::Type type)
  : SemiColonNode(start, parent, type)
{}

PropertyEndNode::PropertyEndNode(QTextCursor start,
                                 QObject* parent,
                                 Node::Type type)
  : Node(start, parent, type)
{}




} // end of StylesheetParser
