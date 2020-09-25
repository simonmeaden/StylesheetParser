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
#ifndef NODE_H
#define NODE_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QTextCursor>
#include <cstdarg>

#include "parserstate.h"

namespace StylesheetEditor {

class Node : public QObject
{

public:
  enum Type
  {
    NodeType,
    BaseNodeType,
    CharNodeType,
    ColonNodeType,
    NameType,
    //    ValueType,
    WidgetType,
    SubControlType,
    SubControlMarkerType,
    PseudoStateType,
    PseudoStateMarkerType,
    SemiColonType,
    StartBraceType,
    EndBraceType,
    NewlineType,
    PropertyType,
    PropertyMarkerType,
    PropertyEndType,
    BadNodeType,
  };
  Node* previous;
  Node* next;

  explicit Node(QTextCursor start, QObject* parent, Type type = NodeType);

  virtual int start() const;
  void setStart(int position);
  void moveStart(int count);
  virtual int end() const;
  virtual int length() const;

  Type type() const;
  QString toString() {
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

    case PropertyMarkerType:
      return "Property End Marker";

    case PropertyEndType:
      return "Property End";

    case BadNodeType:
      return "Bad Node";
    }

  }

protected:
  QTextCursor m_start;
  Type m_type;

};

class BaseNode : public Node
{
  Q_OBJECT
public:
  explicit BaseNode(const QString& value, QTextCursor start, QObject* parent, Type type = BaseNodeType);

  QString value() const;
  int end() const override;
  int length() const override;

protected:
  QString m_value;
};
using NodeList = QList<Node*>;

class NameNode : public Node
{
  Q_OBJECT
public:
  explicit NameNode(const QString& name, QTextCursor start, QObject* parent, Type type = NameType);

  QString value() const;
  int end() const override;
  int length() const override;

private:
  QString m_name;

};

class BadBlockNode: public Node
{
  Q_OBJECT
public:
  explicit BadBlockNode(const QString& name, QTextCursor start, ParserState::Errors errors, QObject* parent,
                        Type type = BadNodeType);

  QString value() const;
  int end() const override;
  int length() const override;

  ParserState::Errors errors() const;
  void setError(const ParserState::Errors& errors);

private:
  QString m_name;
  ParserState::Errors m_errors;
};

//class ValueNode : public Node
//{
//  Q_OBJECT
//public:
//  explicit ValueNode(const QStringList& values,
//                     QList<bool> checks,
//                     QList<int> offsets,
//                     QTextCursor start,
//                     QObject* parent,
//                     Type type = ValueType);

//  QStringList values() const;
//  QList<bool> checks() const;
//  QList<int> offsets() const;
//  void setValues(const QStringList& values);
//  void setChecks(const QList<bool>& checks);
//  void setOffsets(const QList<int>& offsets);

//  int count();
//  bool isValid(int index);


//private:
//  QStringList m_values;
//  QList<bool> m_checks;
//  QList<int> m_offsets;
//};

class WidgetNode : public BaseNode
{
  Q_OBJECT
public:
  explicit WidgetNode(const QString& name, QTextCursor start, QObject* parent, Type type = WidgetType);
};

class PropertyNode : public BaseNode
{
  Q_OBJECT
public:
  enum Check
  {
    GoodValue,
    BadValue,
//    MissingPropertyMarker,
    MissingPropertyEnd,
  };
  explicit PropertyNode(const QString& name, QTextCursor start, QObject* parent, Type type = PropertyType);

  //! Returns the values as a list.
  QStringList values() const;
  //! Returns the checks as a list.
  QList<Check> checks() const;
  //! Returns the offsets as a list.
  QList<int> offsets() const;
  //! Sets the values to the supplied string values.
  void setValues(const QStringList& values);
  //! Sets the checks to the supplied bool values.
  void setChecks(const QList<Check>& checks);
  //! Sets the offsets to the supplied int offsets.
  void setOffsets(const QList<int>& offsets);
  //! Adds a complete value/check/offset to the values.
  void addValue(const QString& value, Check check, int offset);
  //! Sets the check at index as bad.
  //!
  //! If index is not set then the default is to set the last value as bad.
  //!
  //! This will mean that the property at index will show up as a bad node, even
  //! if it is actually a correct value.
  bool setBadCheck(Check check, int index = -1);

  //! Returns the number of values in the property.
  int count();
  // indicates wheter the value at index is a valid value.
  Check isValid(int index);
  int end();

  bool propertyMarkerExists() const;
  void setPropertyMarkerExists(bool propertyMarker);

private:
  QStringList m_values;
  QList<Check> m_checks;
  QList<int> m_offsets;
  bool m_propertyMarkerExists;
};

class SubControlNode : public NameNode
{
  Q_OBJECT
public:
  explicit SubControlNode(const QString& name, QTextCursor start, QObject* parent, Type type = SubControlType);
};

class PseudoStateNode : public NameNode
{
  Q_OBJECT
public:
  explicit PseudoStateNode(const QString& name, QTextCursor start, QObject* parent, Type type = PseudoStateType);
};

class CharNode : public Node
{
  Q_OBJECT
public:
  explicit CharNode(QTextCursor start, QObject* parent, Type type = CharNodeType);

  int end() const override;
  int length() const override;
};

class ColonNode : public CharNode
{
  Q_OBJECT
public:
  explicit ColonNode(QTextCursor start, QObject* parent, Type type = ColonNodeType);
};

class PseudoStateMarkerNode : public ColonNode
{
  Q_OBJECT
public:
  explicit PseudoStateMarkerNode(QTextCursor start, QObject* parent, Type type = PseudoStateMarkerType);
};

class PropertyMarkerNode : public ColonNode
{
  Q_OBJECT
public:
  explicit PropertyMarkerNode(QTextCursor start, QObject* parent, Type type = PropertyMarkerType);
};

class SemiColonNode : public CharNode
{
  Q_OBJECT
public:
  explicit SemiColonNode(QTextCursor start, QObject* parent, Type type = SemiColonType);
};

class PropertyEndMarkerNode : public SemiColonNode
{
  Q_OBJECT
public:
  explicit PropertyEndMarkerNode(QTextCursor start, QObject* parent, Type type = PropertyEndType);
};

class PropertyEndNode : public Node
{
  Q_OBJECT
public:
  explicit PropertyEndNode(QTextCursor start, QObject* parent, Type type = PropertyEndType);
};

class SubControlMarkerNode : public CharNode
{
  Q_OBJECT
public:
  explicit SubControlMarkerNode(QTextCursor start, QObject* parent, Type type = SubControlMarkerType);

  int end() const override;
  int length() const override;
};


class NewlineNode : public CharNode
{
  Q_OBJECT
public:
  explicit NewlineNode(QTextCursor start, QObject* parent, Type type = NewlineType);
};

class StartBraceNode : public CharNode
{
  Q_OBJECT
public:
  explicit StartBraceNode(QTextCursor start, QObject* parent, Type type = StartBraceType);
};

class EndBraceNode : public CharNode
{
  Q_OBJECT
public:
  explicit EndBraceNode(QTextCursor start, QObject* parent, Type type = EndBraceType);
};


} // end of StylesheetParser

#endif // NODE_H
