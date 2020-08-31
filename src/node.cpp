/*
   Copyright 2020 Simon Meaden

   Permission is hereby granted, free of charge, to any person obtaining a copy of this
   software and associated documentation files (the "Software"), to deal in the Software
   without restriction, including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software, and to permit
                                                                         persons to whom the Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all copies or
   substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
   INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
    PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
   OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
      SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "stylesheetparser/node.h"

namespace StylesheetParser {

Node::Node(int pos, QObject* parent)
   : QObject(parent)
   , m_pos(pos)
{

}

int Node::pos() const
{
   return m_pos;
}

StringNode::StringNode(const QString& value, int pos, QObject* parent)
   : Node(pos, parent)
   , m_value(value)
{

}

QString StringNode::value() const
{
   return m_value;
}



NameNode::NameNode(const QString& name, int pos, QObject* parent)
   : StringNode(name, pos, parent)
{

}

ValueNode::ValueNode(const QString& value, int pos, QObject* parent)
   : StringNode(value, pos, parent)
{

}


ColonNode::ColonNode(int pos, QObject* parent)
   : Node(pos, parent)
{

}

SemiColonNode::SemiColonNode(int pos, QObject* parent)
   : Node(pos, parent)
{

}

StartBraceNode::StartBraceNode(int pos, QObject* parent)
   : Node(pos, parent)
{

}

EndBraceNode::EndBraceNode(int pos, QObject* parent)
   : Node(pos, parent)
{

}


} // end of StylesheetParser
