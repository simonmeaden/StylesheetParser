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
#ifndef NODE_H
#define NODE_H

#include <QObject>
#include <QList>

namespace StylesheetParser {

class Node : public QObject
{
public:
   explicit Node(int pos, QObject *parent);

   int pos() const;

private:
   int m_pos;
};

class StringNode : public Node {
public:
  explicit StringNode(const QString& value, int pos, QObject *parent);

  QString value() const;

private:
  QString m_value;
};

class NameNode : public StringNode
{
public:
  explicit NameNode(const QString& name, int pos, QObject *parent);
};

class ValueNode : public StringNode
{
public:
  explicit ValueNode(const QString& name, int pos, QObject *parent);
};

class ColonNode : public Node
{
public:
  explicit ColonNode(int pos, QObject *parent);
};

class SemiColonNode : public Node
{
public:
  explicit SemiColonNode(int pos, QObject *parent);
};

class StartBraceNode : public Node
{
public:
  explicit StartBraceNode(int pos, QObject *parent);
};

class EndBraceNode : public Node
{
public:
  explicit EndBraceNode(int pos, QObject *parent);
};

} // end of StylesheetParser

#endif // NODE_H
