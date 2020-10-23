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
#include <QWidget>
//#include <cstdarg>

#include "parserstate.h"
#include "datastore.h"

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
    //    PropertyMarkerType,
    PropertyEndType,
    PropertyEndMarkerType,
    BadNodeType,
    BadSubControlMarkerType,
    BadPseudoStateMarkerType,
    CommentType,
    CommentStartMarkerType,
    CommentEndMarkerType,
  };
  Node* previous;
  Node* next;

  explicit Node(QTextCursor start, StylesheetEdit* parent, Type type = NodeType);

  virtual int start() const;
  void setStart(int position);
  void moveStart(int count);
  virtual int end() const;
  QTextCursor cursor();

  Type type() const;
  QString toString();

protected:
  QTextCursor m_cursor;
  Type m_type;
  StylesheetEdit* m_parent;

};

class NameNode
{
public:
  enum SectionType
  {
    None,
    Name,
    Value, // only valid for propety nodes.
  };
  explicit NameNode(const QString& name);

  QString name() const;
  void setName(const QString& value);
  virtual int length() const;

  // can't be implemented here as it doesn't have enough info.
  virtual QPair<SectionType, int> isIn(QPoint pos) = 0;

protected:
  QString m_name;
};

class NamedNode : public Node, public NameNode
{
  Q_OBJECT
public:
  NamedNode(const QString& name, QTextCursor start, StylesheetEdit* parent, Type type = NodeType);

  QPair<SectionType, int> isIn(QPoint pos) override;
  int end() const;
};

class BadNode
{
public:
  explicit BadNode(ParserState::Errors errors);

  ParserState::Errors errors() const;
  void setError(const ParserState::Errors& errors);

private:
  ParserState::Errors m_errors;

};

class BadBlockNode: public NamedNode, public BadNode
{
  Q_OBJECT
public:
  explicit BadBlockNode(const QString& name, QTextCursor start, ParserState::Errors errors, StylesheetEdit* parent,
                        Node::Type type = Node::BadNodeType);

  int end() const override;
};

class SubControlMarkerNode : public NamedNode
{
  Q_OBJECT
public:
  explicit SubControlMarkerNode(QTextCursor start, StylesheetEdit* parent, Type type = SubControlMarkerType);

  int end() const override;
};

class ColonNode : public NamedNode
{
  Q_OBJECT
public:
  explicit ColonNode(QTextCursor start, StylesheetEdit* parent, Type type = ColonNodeType);
};

class BadSubControlMarkerNode : public SubControlMarkerNode, public BadNode
{
  Q_OBJECT
public:
  BadSubControlMarkerNode(QTextCursor start, ParserState::Errors errors, StylesheetEdit* parent,
                          Type type = BadSubControlMarkerType);
};

class PseudoStateMarkerNode : public ColonNode
{
  Q_OBJECT
public:
  explicit PseudoStateMarkerNode(QTextCursor start, StylesheetEdit* parent, Type type = PseudoStateMarkerType);
};

class BadPseudoStateMarkerNode : public PseudoStateMarkerNode, public BadNode
{
  Q_OBJECT
public:
  BadPseudoStateMarkerNode(QTextCursor start, ParserState::Errors errors, StylesheetEdit* parent,
                           Type type = BadPseudoStateMarkerType);
};

class WidgetNode : public NamedNode
{
  Q_OBJECT
public:
  explicit WidgetNode(const QString& name, QTextCursor start, StylesheetEdit* parent, Type type = WidgetType);

  bool isWidgetValid() const;
  void setWidgetValid(bool widgetValid);

private:
  bool m_widgetValid;

};

class PropertyNode : public NamedNode
{
  Q_OBJECT
public:
  enum Check
  {
    GoodValue,
    ValidPropertyType,
    BadValue,
    MissingPropertyEnd,
  };
  explicit PropertyNode(const QString& name, QTextCursor start, StylesheetEdit* parent, Type type = PropertyType);

  //! Returns the values as a list.
  QStringList values() const;
  //! Returns the checks as a list.
  QList<Check> checks() const;
  //! Returns the offsets as a list.
  QList<int> offsets() const;
  //! Returns the attribute types as a list.
  QList<DataStore::AttributeType> attributeTypes() const;
  //! Sets the values to the supplied string values.
  void setValues(const QStringList& values);
  //! Sets the checks to the supplied bool values.
  void setChecks(const QList<Check>& checks);
  //! Sets the offsets to the supplied int offsets.
  void setOffsets(const QList<int>& offsets);
  //! Sets the attribute types as a list.
  void setAttributeTypes(const QList<DataStore::AttributeType>& attributeTypes);
  //! Adds a complete value/check/offset to the values.
  void addValue(const QString& value, Check check, int offset, DataStore::AttributeType attType);
  //! Sets the check at index as bad.
  //!
  //! If index is not set then the default is to set the last value as bad.
  //!
  //! This will mean that the property at index will show up as a bad node, even
  //! if it is actually a correct value.
  bool setBadCheck(Check check, int index = -1);

  //! Returns the number of values in the property.
  int count();
  // indicates whether the value at index is a valid value.
  Check isValueValid(int index);
  int end() const override;

  bool hasPropertyMarker() const;
  void setPropertyMarkerExists(bool propertyMarker);

  bool isValidProperty() const;
  void setValidProperty(bool validProperty);

  int propertyMarkerOffset() const;
  void setPropertyMarkerOffset(int propertymarkerOffset);

  QPair<SectionType, int> isIn(QPoint pos) override;
  QPair<bool, QString> isProperty(int offset);

private:
  QStringList m_values;
  QList<Check> m_checks;
  QList<int> m_offsets;
  QList<DataStore::AttributeType> m_attributeTypes;
  bool m_propertyMarkerExists;
  int m_propertymarkerOffset;
  bool m_validProperty;
};

class BadPropertyNode : public PropertyNode, public BadNode
{
  Q_OBJECT
public:

};

class SubControlNode : public NamedNode
{
  Q_OBJECT
public:
  explicit SubControlNode(const QString& name, QTextCursor start, StylesheetEdit* parent, Type type = SubControlType);

  int end() const override;
  bool isStateValid() const;
  void setStateValid(bool stateValid);

private:
  bool m_stateValid;

};

class PseudoStateNode : public NamedNode
{
  Q_OBJECT
public:
  explicit PseudoStateNode(const QString& name, QTextCursor start, StylesheetEdit* parent, Type type = PseudoStateType);

  int end() const override;
  bool isStateValid() const;
  void setStateValid(bool stateValid);

private:
  bool m_stateValid;

};

class CommentNode : public NamedNode
{
  Q_OBJECT
public:
  explicit CommentNode(QTextCursor start, StylesheetEdit* parent, Type type = CommentType);

  void append(QChar c);
  void append(QString text);
  int end() const override;

  QPair<SectionType, int> isIn(QPoint pos) override;
};

class SemiColonNode : public NamedNode
{
  Q_OBJECT
public:
  explicit SemiColonNode(QTextCursor start, StylesheetEdit* parent, Type type = SemiColonType);
};

class PropertyEndMarkerNode : public SemiColonNode
{
  Q_OBJECT
public:
  explicit PropertyEndMarkerNode(QTextCursor start, StylesheetEdit* parent, Type type = PropertyEndMarkerType);
};

class PropertyEndNode : public Node
{
  Q_OBJECT
public:
  explicit PropertyEndNode(QTextCursor start, StylesheetEdit* parent, Type type = PropertyEndType);
};

class NewlineNode : public NamedNode
{
  Q_OBJECT
public:
  explicit NewlineNode(QTextCursor start, StylesheetEdit* parent, Type type = NewlineType);
};

class StartBraceNode : public NamedNode
{
  Q_OBJECT
public:
  explicit StartBraceNode(QTextCursor start, StylesheetEdit* parent, Type type = StartBraceType);

  bool isBraceAtCursor() const;
  void setBraceAtCursor(bool isFlagBrace);

private:
  bool m_isBraceAtCursor;
};

class EndBraceNode : public NamedNode
{
  Q_OBJECT
public:
  explicit EndBraceNode(QTextCursor start, StylesheetEdit* parent, Type type = EndBraceType);

  bool isBraceAtCursor() const;
  void setBraceAtCursor(bool isFlagBrace);

private:
  bool m_isBraceAtCursor;
};

class StartCommentNode : public NamedNode
{
  Q_OBJECT
public:
  explicit StartCommentNode(QTextCursor start, StylesheetEdit* parent, Type type = CommentStartMarkerType);

  int end() const override;
};

class EndCommentNode : public NamedNode
{
  Q_OBJECT
public:
  explicit EndCommentNode(QTextCursor start, StylesheetEdit* parent, Type type = CommentEndMarkerType);

  int end() const override;
};

#endif // NODE_H
