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
#include "stylesheetparser/node.h"

namespace StylesheetParser {

Node::Node(int start, QObject* parent, Type type)
  : QObject(parent)
  , previous(nullptr)
  , next(nullptr)
  , m_start(start)
  , m_type(type)
{}

int Node::start() const
{
  return m_start;
}

int Node::end() const
{
  return m_start;
}

int Node::length() const
{
  return 0;
}

Node::Type Node::type() const
{
  return m_type;
}

//Node *Node::previous() {
//  return m_previous;
//}

//Node *Node::next() {
//  return m_next;
//}

//void Node::setNext(Node *next)
//{
//  m_next = next;
//}

//void Node::setPrevious(Node *previous)
//{
//  m_previous = previous;
//}


BaseNode::BaseNode(const QString& value, int start, QObject* parent, Type type)
  : Node(start, parent, type)
  , m_value(value)
{}

QString BaseNode::value() const
{
  return m_value;
}

int BaseNode::end() const
{
  return m_start + m_value.length();
}

int BaseNode::length() const
{
  return m_value.length();
}


NameNode::NameNode(const QString& name, int start, QObject* parent, Type type)
  : Node(start, parent, type)
  , m_name(name)
{}

QString NameNode::value() const
{
  return m_name;
}

int NameNode::end() const
{
  return m_start + m_name.length();
}

int NameNode::length() const
{
  return m_name.length();
}

BadBlockNode::BadBlockNode(const QString& name, int start, QObject* parent, Type type)
  : Node(start, parent, type)
  , m_name(name)
{}

QString BadBlockNode::value() const
{
  return m_name;
}

int BadBlockNode::end() const
{
  return m_start + m_name.length();
}

int BadBlockNode::length() const
{
  return m_name.length();
}

ValueNode::ValueNode(const QStringList& values, QList<bool> checks, QList<int> offsets, int start, QObject* parent,
                     Type type)
  : Node(start, parent, type)
  , m_values(values)
  , m_checks(checks)
  , m_offsets(offsets)
{}

QStringList ValueNode::values() const
{
  return m_values;
}

int ValueNode::count()
{
  return m_values.size();
}

bool ValueNode::isValid(int index)
{
  return m_checks.at(index);
}

QList<int> ValueNode::offsets() const
{
  return m_offsets;
}

QList<bool> ValueNode::checks() const
{
  return m_checks;
}

WidgetNode::WidgetNode(const QString& name, int start, QObject* parent, Type type)
  : BaseNode(name, start, parent, type)
{}


PropertyNode::PropertyNode(const QString& name, int start, QObject* parent, Node::Type type)
  : BaseNode(name, start, parent, type)
{}

SubControlNode::SubControlNode(const QString& name, int start, QObject* parent, Type type)
  : NameNode(name, start, parent, type)
{
}

PseudoStateNode::PseudoStateNode(const QString& name, int start, QObject* parent, Type type)
  : NameNode(name, start, parent, type)
{}

CharNode::CharNode(int start, QObject* parent, Type type)
  : Node(start, parent, type)
{
}

int CharNode::end() const
{
  return m_start + 1;
}

int CharNode::length() const
{
  return 1;
}

ColonNode::ColonNode(int start, QObject* parent, Type type)
  : CharNode(start, parent, type)
{}

SubControlMarkerNode::SubControlMarkerNode(int start, QObject* parent, Type type)
  : CharNode(start, parent, type)
{}

int SubControlMarkerNode::end() const
{
  return m_start + 2;
}

int SubControlMarkerNode::length() const
{
  return 2;
}

SemiColonNode::SemiColonNode(int start, QObject* parent, Type type)
  : CharNode(start, parent, type)
{}

NewlineNode::NewlineNode(int start, QObject* parent, Type type)
  : CharNode(start, parent, type)
{}

StartBraceNode::StartBraceNode(int start, QObject* parent, Type type)
  : CharNode(start, parent, type)
{}

EndBraceNode::EndBraceNode(int start, QObject* parent, Type type)
  : CharNode(start, parent, type)
{}

PseudoStateMarkerNode::PseudoStateMarkerNode(int start, QObject* parent, Type type)
  : ColonNode(start, parent, type)
{}

PropertyMarkerNode::PropertyMarkerNode(int start, QObject* parent, Node::Type type)
  : ColonNode(start, parent, type)
{}





} // end of StylesheetParser
